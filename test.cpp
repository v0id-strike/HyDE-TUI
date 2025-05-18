#include <ncurses.h>
#include <string>
#include <vector>
#include <memory>
#include <dlfcn.h>
#include <iostream>
#include <cstdlib>

// Constants for UI layout
constexpr int HEADER_HEIGHT = 9;
constexpr int MAIN_PANEL_OFFSET = 20;
constexpr int LEFT_PANEL_WIDTH_DIVIDER = 2;

struct UserData {
    int selected = 0;
    std::vector<std::string> options;
    std::string fresh_install;
    std::string update;
    std::string themes;
};

struct Alerts {
    std::string info_text;
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
        draw_box(HEADER_HEIGHT, max_x);
        draw_box(max_y - MAIN_PANEL_OFFSET, max_x, HEADER_HEIGHT);
        draw_box(max_y, max_x / LEFT_PANEL_WIDTH_DIVIDER, max_y - MAIN_PANEL_OFFSET);
        draw_box(max_y, max_x, max_y - MAIN_PANEL_OFFSET, max_x / LEFT_PANEL_WIDTH_DIVIDER);
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

std::pair<UserData, Alerts> main_menu(int max_y, int max_x, UserData& data) {
    Alerts alerts{
        "Use arrow keys to navigate, ENTER to select, Q to quit"
    };

    attron(COLOR_PAIR(5));
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
            data.selected = (data.selected - 1 + data.options.size()) % data.options.size();
            break;
        case KEY_DOWN:
            data.selected = (data.selected + 1) % data.options.size();
            break;
        case 10: { // ENTER
            echo();
            curs_set(1);
            move(max_y - 2, 10);
            clrtoeol();
            
            const char* prompts[] = {
                "What is your fresh_install? ",
                "What is your update? ",
                "Where are you from? "
            };
            printw("%s", prompts[data.selected]);
            
            char input[256];
            getnstr(input, sizeof(input) - 1);
            
            std::string* targets[] = {&data.fresh_install, &data.update, &data.themes};
            *targets[data.selected] = input;
            
            noecho();
            curs_set(0);
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
        {"1) fresh_install", "2) update", "3) themes"},
        "", "", ""
    };

    while (true) {
        ui.layout(max_y, max_x);
        ui.draw_logo(1);
        
        auto [updated_data, alerts] = main_menu(max_y, max_x, data);
        data = updated_data;
        
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