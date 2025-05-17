#include <ncurses.h>
#include <string>
#include <vector>
#include <cstring> // for strstr
#include <iostream>
#include <dlfcn.h>
#include <cstdlib>

void install_ncurses() {
    std::cout << "Attempting to install ncurses...\n";
    if (system("sudo pacman -S --noconfirm ncurses") != 0) {
        std::cerr << "Installation failed. Please run:\n";
        std::cerr << "  sudo pacman -S ncurses\n";
        exit(1);
    }
}

void setup_transparent_background() {
    // Try to use default terminal background
    use_default_colors(); // Supported by many modern terminals
    
    // Initialize color pairs with transparent background (-1)
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_GREEN, -1);
    
    // Fallback for terminals that don't support default colors
    if (!has_colors() || !can_change_color()) {
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
    }
}

void draw_box(int height, int width, int starty, int startx) {
    // Use ACS characters with default background
    attron(COLOR_PAIR(0));
    mvaddch(starty, startx, ACS_ULCORNER);
    mvaddch(starty, width - 1, ACS_URCORNER);
    mvaddch(height - 1, startx, ACS_LLCORNER);
    mvaddch(height - 1, width - 1, ACS_LRCORNER);
    
    for (int i = startx + 1; i < width - 1; i++) {
        mvaddch(starty, i, ACS_HLINE);
        mvaddch(height - 1, i, ACS_HLINE);
    }
    
    for (int i = starty + 1; i < height - 1; i++) {
        mvaddch(i, startx, ACS_VLINE);
        mvaddch(i, width - 1, ACS_VLINE);
    }
    attroff(COLOR_PAIR(0));
}

void draw_centered_text(int y, const std::string& text) {
    int x = (COLS - text.length()) / 2;
    mvprintw(y, x, "%s", text.c_str());
}

void draw_divider(int y) {
    attron(COLOR_PAIR(0));
    mvaddch(y, 0, ACS_LTEE);
    for (int i = 1; i < COLS - 1; i++) {
        mvaddch(y, i, ACS_HLINE);
    }
    mvaddch(y, COLS - 1, ACS_RTEE);
    attroff(COLOR_PAIR(0));
}

int main() {

    // Check if we can load ncurses
    if (dlopen("libncurses++w.so", RTLD_LAZY) == nullptr) {
        std::cout << "NCurses not found. Install? [y/N] ";
        char answer;
        std::cin >> answer;
        
        if (tolower(answer) == 'y') {
            install_ncurses();
        } else {
            exit(1);
        }
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // Enable color with transparent background support
    start_color();
    setup_transparent_background();
    
    // Get screen dimensions
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Draw the main box (will use default background)
    draw_box(max_y, max_x, 0, 0);
  
    // ASCII Art section
    attron(COLOR_PAIR(2));
    draw_centered_text(3, "ASCII");
    attroff(COLOR_PAIR(2));
    
    // First divider
    draw_divider(5);
    
    // Options section (middle)
    std::vector<std::string> options = {"1) Age", "2) Name", "3) Country"};
    int selected = 0;
    
    // Info/log section
    std::string info_text = "Use arrow keys to navigate, ENTER to select";
    mvprintw(7, (max_x - info_text.length()) / 2, "%s", info_text.c_str());
    
    // User input storage
    std::string age, name, country;
    
    while (true) {
        // Draw options
        for (int i = 0; i < options.size(); i++) {
            if (i == selected) {
                attron(A_REVERSE);
            }
            mvprintw(10 + i, (max_x - options[i].length()) / 2, "%s", options[i].c_str());
            if (i == selected) {
                attroff(A_REVERSE);
            }
        }
        
        // Second divider
        draw_divider(max_y - 10);
        
        // Display collected info - with transparent background
        attron(COLOR_PAIR(3));
        if (!age.empty()) {
            mvprintw(max_y - 8, 10, "Age: %s", age.c_str());
        }
        if (!name.empty()) {
            mvprintw(max_y - 7, 10, "Name: %s", name.c_str());
        }
        if (!country.empty()) {
            mvprintw(max_y - 6, 10, "Country: %s", country.c_str());
        }
        attroff(COLOR_PAIR(3));
        
        // User input section
        mvprintw(max_y - 3, 10, "Press Q to quit");
        
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + options.size()) % options.size();
                break;
            case KEY_DOWN:
                selected = (selected + 1) % options.size();
                break;
            case 10: // ENTER
                {
                    echo();
                    curs_set(1);
                    move(max_y - 2, 10);
                    clrtoeol();
                    
                    std::string prompt;
                    switch (selected) {
                        case 0: prompt = "What is your age? "; break;
                        case 1: prompt = "What is your name? "; break;
                        case 2: prompt = "Where are you from? "; break;
                    }
                    
                    printw("%s", prompt.c_str());
                    char input[256];
                    getstr(input);
                    
                    switch (selected) {
                        case 0: age = input; break;
                        case 1: name = input; break;
                        case 2: country = input; break;
                    }
                    
                    noecho();
                    curs_set(0);
                }
                break;
            case 'q':
            case 'Q':
                endwin();
                return 0;
        }
    }
    
    endwin();
    return 0;
}
