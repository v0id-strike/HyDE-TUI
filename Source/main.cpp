#include "utils.hpp"

// <<=====================Get Terminal Size==============================>>
std::pair<int, int> get_max_xy() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        return {0, 0};
    }
    return {w.ws_col, w.ws_row};
}

// <<=====================Draw Lines==============================>>
void draw_horizontal_line(int y, int start_x, int end_x, char ch = '-') {
    std::cout << "\033[" << y << ";" << start_x << "H"; // Move cursor
    for (int x = start_x; x <= end_x; ++x)
        std::cout << ch;
}
void draw_vertical_line(int x, int start_y, int end_y, char ch = '|') {
    for (int y = start_y; y <= end_y; ++y) {
        std::cout << "\033[" << y << ";" << x << "H" << ch;
    }
}

// <<===================Draw Box===============================>>
void draw_box(int left, int top, int right, int bottom) {
    // Corners
    std::cout << "\033[" << top << ";" << left << "H+";
    std::cout << "\033[" << top << ";" << right << "H+";
    std::cout << "\033[" << bottom << ";" << left << "H+";
    std::cout << "\033[" << bottom << ";" << right << "H+";

    // Sides
    draw_horizontal_line(top, left + 1, right - 1, '-');
    draw_horizontal_line(bottom, left + 1, right - 1, '-');
    draw_vertical_line(left, top + 1, bottom - 1, '|');
    draw_vertical_line(right, top + 1, bottom - 1, '|');
}

// <<=====================Draw TUI Layout=============================>>
void draw_layout(int max_x, int max_y) {
    
       
    // Draw TUI Layout
    draw_box(0, 0, max_x, 10); // ASCII Logo
    draw_box(0, 10, max_x, max_y - 10); // Logs
    draw_box(0, max_y - 10, max_x / 2, max_y - 4); // User Application
    draw_box(max_x / 2, max_y - 10, max_x, max_y - 4); // Fastfetch
    draw_box(0, max_y - 4, max_x, max_y); // Manual

    // Clear screen
    std::cout << "\033[2J";
}

// <<======================User Choice==============================>>
// Read arrow keys or Enter
int get_choice(const std::vector<std::string>& options) {
    int selected = 0;
    char ch;

    while (true) {
        ch = getchar();
        if (ch == '\033') {
            getchar();
            switch (getchar()) {
                case 'A': // Up arrow
                    selected = (selected - 1 + options.size()) % options.size();
                    break;
                case 'B': // Down arrow
                    selected = (selected + 1) % options.size();
                    break;
            }
        } else if (ch == '\n') {
            break;
        }
    }
    return selected;
}

// <<=====================MAIN=============================>>
int main() {
    // Get terminal size
    auto [max_x, max_y] = get_max_xy();
    
    // Clear screen
    std::cout << "\033[2J";

    while (true) {
        draw_layout(max_x, max_y);

        std::vector<std::string> options = {
            "Option 1",
            "Option 2",
            "Option 3"
        };
        //int choice = get_choice(options);
        //std::cout << choice;
    }
    
    return 0;
}
