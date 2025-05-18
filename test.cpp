#include <ncurses.h>
#include <string>
#include <vector>
#include <memory>
#include <dlfcn.h>
#include <iostream>
#include <cstdlib>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>  // For chmod and S_IRWXU

struct UserData {
    int selected = 0;
    std::vector<std::string> options;
    std::string fresh_install;
    std::string update;
    std::string themes;
    std::vector<std::string> logs;        // For storing script output
    std::string system_info;              // For storing system information
};

struct Alerts {
    std::string info_text;
    std::string warning_text;
};

// Custom deleter to avoid noexcept warning
struct DlCloser {
    void operator()(void* handle) const noexcept {
        if (handle) dlclose(handle);
    }
};

class NcursesManager {
public:
    NcursesManager() {
        check_ncurses();
        init_ncurses();
    }
    
    ~NcursesManager() {
        endwin();
    }

private:
    void check_ncurses() {
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

    void install_ncurses() {
        if (system("sudo pacman -S --noconfirm ncurses") != 0) {
            std::cerr << "Installation failed. Please run:\n";
            std::cerr << "  sudo pacman -S ncurses\n";
            exit(EXIT_FAILURE);
        }
    }

    void init_ncurses() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        start_color();
        setup_transparent_background();
    }

    void setup_transparent_background() {
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
};

class TUI {
public:
    void draw_box(int endy, int endx, int starty = 0, int startx = 0) {
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

    void layout(int max_y, int max_x) {
        draw_box(9, max_x);
        draw_box(max_y - 20, max_x, 9);
        draw_box(max_y - 3, max_x / 2, max_y - 20);
        draw_box(max_y - 3, max_x, max_y - 20, max_x / 2);
        draw_box(max_y, max_x, max_y -3);
    }

    void draw_centered_multiline(int start_y, const std::vector<std::string>& lines) {
        size_t max_length = 0;
        for (const auto& line : lines) {
            max_length = std::max(max_length, line.length());
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            mvprintw(start_y + i, (COLS - max_length) / 2, "%s", lines[i].c_str());
        }
    }

    void draw_logo(int y) {
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
};

void update_logs(const std::vector<std::string>& logs, int max_y, int max_x) {
    int start_y = 10;  // Start after the ASCII art box
    int max_lines = max_y - 20 - start_y - 2;  // Available space for logs
    
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

void update_system_info(const std::string& info, int max_y, int max_x) {
    // Implementation of update_system_info function
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

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line = buffer;
        // Remove trailing newline
        if (!line.empty() && line[line.length()-1] == '\n') {
            line.erase(line.length()-1);
        }
        data.logs.push_back(line);
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        data.logs.push_back("Script exited with status: " + std::to_string(status));
    }
}

std::pair<UserData, Alerts> main_menu(int max_y, int max_x, UserData& data) {
    // Get current username
    struct passwd *pw = getpwuid(getuid());
    std::string username = pw ? pw->pw_name : "User";

    Alerts alerts{
        "<Q> : quit  ||  <Arrow-Keys> : navigate || <Space> : multiselect || <Enter> : continue"
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
                    script_name = "Scripts/fresh_install.sh";
                    data.logs.push_back("Starting fresh installation...");
                    break;
                case 1: 
                    script_name = "Scripts/update.sh";
                    data.logs.push_back("Starting system update...");
                    break;
                case 2: 
                    script_name = "Scripts/themepatcher.sh";
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
        {"1) Fresh Install", "2) Update", "3) Theme Patcher"},
        "", "", "", {}, ""
    };

    while (true) {
        clear();
        ui.layout(max_y, max_x);
        ui.draw_logo(1);
        
        auto [updated_data, alerts] = main_menu(max_y, max_x, data);
        data = updated_data;
        
        // Update all panels
        update_logs(data.logs, max_y, max_x);
        update_system_info(data.system_info, max_y, max_x);
        
        // Display help text
        attron(COLOR_PAIR(5));
        mvprintw(max_y - 2, (max_x - alerts.info_text.length()) / 2, "%s", alerts.info_text.c_str());
        attroff(COLOR_PAIR(5));
        
        refresh();
        
        if (data.fresh_install == "QUIT") break;
        
        if (!data.fresh_install.empty()) {
            attron(COLOR_PAIR(3));
            mvprintw(max_y - 8, 10, "fresh_install: %s", data.fresh_install.c_str());
            attroff(COLOR_PAIR(3));
        }
        if (!data.update.empty()) {
            attron(COLOR_PAIR(3));
            mvprintw(max_y - 7, 10, "update: %s", data.update.c_str());
            attroff(COLOR_PAIR(3));
        }
        if (!data.themes.empty()) {
            attron(COLOR_PAIR(3));
            mvprintw(max_y - 6, 10, "themes: %s", data.themes.c_str());
            attroff(COLOR_PAIR(3));
        }
    }

    return EXIT_SUCCESS;
}