#include "HyDE_TUI.hpp"

// Custom deleter implementation
void DlCloser::operator()(void* handle) const noexcept {
    if (handle) dlclose(handle);
}

// Ncurses implementations
NcursesManager::NcursesManager() {
    check_ncurses();
    init_ncurses();
}

NcursesManager::~NcursesManager() {
    endwin();
}

void NcursesManager::check_ncurses() {
    std::unique_ptr<void, DlCloser> handle(
        dlopen("libncurses++w.so", RTLD_LAZY)
    );
    
    if (!handle) {
        std::cout << "NCurses not found. Install? [y/N] ";
        char answer;
        std::cin >> answer;
        
        if (tolower(answer) == 'y') {
            install_ncurses();
        } else {
            exit(EXIT_FAILURE);
        }
    }
}

void NcursesManager::install_ncurses() {
    if (system("sudo pacman -S --noconfirm ncurses") != 0) {
        std::cerr << "Installation failed. Please run:\n";
        std::cerr << "  sudo pacman -S ncurses\n";
        exit(EXIT_FAILURE);
    }
}

void NcursesManager::init_ncurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    setup_transparent_background();
}

void NcursesManager::setup_transparent_background() {
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);    // For headers and titles
    init_pair(2, COLOR_YELLOW, -1);  // For selected items
    init_pair(3, COLOR_GREEN, -1);   // For success messages
    init_pair(4, COLOR_RED, -1);     // For error messages
    init_pair(5, COLOR_WHITE, -1);   // For normal text
    
    if (!has_colors() || !can_change_color()) {
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_RED, COLOR_BLACK);
        init_pair(5, COLOR_WHITE, COLOR_BLACK);
    }
}

// TUI implementations
void TUI::draw_box(int endy, int endx, int starty, int startx) {
    attron(COLOR_PAIR(0));
    mvaddch(starty, startx, ACS_ULCORNER);
    mvaddch(starty, endx - 1, ACS_URCORNER);
    mvaddch(endy - 1, startx, ACS_LLCORNER);
    mvaddch(endy - 1, endx - 1, ACS_LRCORNER);
        
    for (int i = startx + 1; i < endx - 1; i++) {
        mvaddch(starty, i, ACS_HLINE);
        mvaddch(endy - 1, i, ACS_HLINE);
    }
        
    for (int i = starty + 1; i < endy - 1; i++) {
        mvaddch(i, startx, ACS_VLINE);
        mvaddch(i, endx - 1, ACS_VLINE);
    }
    attroff(COLOR_PAIR(0));
}

void TUI::layout(int max_y, int max_x) {
    draw_box(9, max_x);
    draw_box(max_y - 20, max_x, 9);
    draw_box(max_y - 3, max_x / 2, max_y - 20);
    draw_box(max_y - 3, max_x, max_y - 20, max_x / 2);
    draw_box(max_y, max_x, max_y -3);
}

void TUI::draw_centered_multiline(int start_y, const std::vector<std::string>& lines) {
    size_t max_length = 0;
    for (const auto& line : lines) {
        max_length = std::max(max_length, line.length());
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        mvprintw(start_y + i, (COLS - max_length) / 2, "%s", lines[i].c_str());
    }
}

void TUI::draw_logo(int y) {
    static const std::vector<std::string> logo = {
        "        .",
        "       / \\         _       _  _      ___  ___",
        "      /^  \\      _| |_    | || |_  _|   \\| __|",
        "     /  _  \\    |_   _|   | __ | || | |) | _|",
        "    /  | | ~\\     |_|     |_||_|\\_, |___/|___|",
        "   /.-'   '-.\\                  |__/"
    };
    attron(COLOR_PAIR(1));
    draw_centered_multiline(y, logo);
    attroff(COLOR_PAIR(1));
}

// Function Implementations
void update_logs(const std::vector<std::string>& logs, int max_y) {
    int start_y = 10;  // Start after the ASCII art box
    size_t max_lines = max_y - 20 - start_y - 2;  // Available space for logs
    
    // Keep only the last max_lines entries
    size_t start_idx = (logs.size() > max_lines) ? logs.size() - max_lines : 0;
    
    for (size_t i = 0; i < std::min(logs.size() - start_idx, static_cast<size_t>(max_lines)); ++i) {
        const std::string& line = logs[start_idx + i];
        
        // Check if the line contains error indicators
        bool is_error = line.find("error") != std::string::npos ||
                       line.find("Error") != std::string::npos ||
                       line.find("ERROR") != std::string::npos ||
                       line.find("No such file") != std::string::npos ||
                       line.find("failed") != std::string::npos;
        
        if (is_error) {
            attron(COLOR_PAIR(4));  // Red color for errors
        } else {
            attron(COLOR_PAIR(5));  // White color for normal text
        }
        
        mvprintw(start_y + i, 2, "%s", line.c_str());
        
        if (is_error) {
            attroff(COLOR_PAIR(4));
        } else {
            attroff(COLOR_PAIR(5));
        }
    }
}

void update_system_info(int max_y, int max_x) {
    int start_y = max_y - 17;  // Start position for system info
    int start_x = max_x / 2 + 2;  // Start from the right panel
    
    // Get system info using fastfetch
    FILE* pipe = popen("fastfetch --logo none --config ~/.config/fastfetch/tui.jsonc", "r");
    if (pipe) {
        char buffer[256];
        int line_count = 0;
        
        attron(COLOR_PAIR(5));
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr && line_count < 10) {
            std::string line = buffer;
            // Remove trailing newline
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.erase(line.length()-1);
            }
            
            // Skip empty lines
            if (line.empty()) continue;
            
            // Display the line
            mvprintw(start_y + line_count, start_x, "%s", line.c_str());
            line_count++;
        }
        attroff(COLOR_PAIR(5));
        
        pclose(pipe);
    }
}

void execute_script(const std::string& script_name, UserData& data) {
    // Add debug message
    data.logs.push_back("Attempting to execute: " + script_name);
    
    // Check if script exists
    if (access(script_name.c_str(), F_OK) == -1) {
        data.logs.push_back("Error: Script not found: " + script_name);
        return;
    }
    
    // Make sure script is executable
    chmod(script_name.c_str(), S_IRWXU);
    
    // Execute script and capture both stdout and stderr
    std::string command = "bash " + script_name + " 2>&1";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        data.logs.push_back("Error: Failed to execute " + script_name);
        return;
    }

    // Clear previous logs for this script
    data.logs.clear();
    data.logs.push_back("Executing: " + script_name);
    data.logs.push_back("----------------------------------------");

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line = buffer;
        // Remove trailing newline
        if (!line.empty() && line[line.length()-1] == '\n') {
            line.erase(line.length()-1);
        }
        data.logs.push_back(line);
        
        // Force refresh the display
        refresh();
    }
    
    int status = pclose(pipe);
    data.logs.push_back("----------------------------------------");
    if (status != 0) {
        data.logs.push_back("Script exited with status: " + std::to_string(status));
    } else {
        data.logs.push_back("Script completed successfully");
    }
    
    // Add a small delay to ensure all output is captured
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

std::pair<UserData, Alerts> main_menu(int max_y, int max_x, UserData& data) {
    // Get current username
    struct passwd *pw = getpwuid(getuid());
    std::string username = pw ? pw->pw_name : "User";

    Alerts alerts{
        "<Q> : quit  ||  <Arrow-Keys> : navigate || <TAB> : multiselect || <Enter> : continue",
        ""  // Initialize warning_text with empty string
    };

    attron(COLOR_PAIR(5));
    mvprintw(max_y - 2, (max_x - alerts.info_text.length()) / 2, "%s", alerts.info_text.c_str());

    alerts.info_text = "Welcome, " + username;
    mvprintw(max_y - 18, (max_x / 2 - alerts.info_text.length()) / 2, "%s", alerts.info_text.c_str());

    for (size_t i = 0; i < data.options.size(); ++i) {
        if (i == data.selected) {
            attron(COLOR_PAIR(2) | A_REVERSE);
        } else {
            attron(COLOR_PAIR(5));
        }
        mvprintw((max_y - 16) + i, (max_x / 2 - 15) / 2, "%s", data.options[i].c_str());
        attroff(COLOR_PAIR(2) | A_REVERSE);
        attroff(COLOR_PAIR(5));
    }

    int ch = getch();
    switch (ch) {
        case KEY_UP:
        case KEY_LEFT:
            data.selected = (data.selected - 1 + data.options.size()) % data.options.size();
            break;
        case KEY_DOWN:
        case KEY_RIGHT:
            data.selected = (data.selected + 1) % data.options.size();
            break;
        case 10: { // ENTER
            std::string script_name;
            switch (data.selected) {
                case 0: 
                    script_name = "fresh_install.sh";
                    data.logs.push_back("Starting fresh installation...");
                    break;
                case 1: 
                    script_name = "update.sh";
                    data.logs.push_back("Starting system update...");
                    break;
                case 2: 
                    script_name = "themepatcher.sh";
                    data.logs.push_back("Starting theme patcher...");
                    break;
            }
            execute_script(script_name, data);
            break;
        }
        case 'q':
        case 'Q':
            data.fresh_install = "QUIT";
            break;
    }

    return {data, alerts};
}

int main() {
    NcursesManager ncurses;
    TUI ui;
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    UserData data{
        0,
        {"1) Fresh Install", "2) Update", "3) Theme Patcher", "4) Editor"},
        "", "", "", {}, ""
    };

    while (true) {
        // Draw the layout
        ui.layout(max_y, max_x);
        ui.draw_logo(1);
        update_system_info(max_y, max_x);
        
        auto [updated_data, alerts] = main_menu(max_y, max_x, data);
        data = updated_data;
        
        // Update all panels
        update_logs(data.logs, max_y);
         
        // Display help text
        attron(COLOR_PAIR(5));
        mvprintw(max_y - 2, (max_x - alerts.info_text.length()) / 2, "%s", alerts.info_text.c_str());
        attroff(COLOR_PAIR(5));
        
        refresh();
        
        if (data.fresh_install == "QUIT") break;
    }

    return EXIT_SUCCESS;
}