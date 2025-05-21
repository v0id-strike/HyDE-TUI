#include <iostream>
#include <string>

// Terminal size (example)
int width = 50;
int height = 20;

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

void draw_box(int top, int left, int bottom, int right) {
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

int main() {
    std::cout << "\033[2J"; // Clear screen
    draw_box(2, 5, 10, 40);
    std::cout << "\033[" << height + 1 << ";0H"; // Move cursor out of the way
    return 0;
}
