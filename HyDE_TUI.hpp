#pragma once

#include <ncurses.h>
#include <string>
#include <vector>
#include <memory>
#include <dlfcn.h>
#include <iostream>
#include <cstdlib>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <functional>

// Forward declarations
class SystemPatcher;
class SystemInstaller;

// Struct declarations
struct UserData {
    size_t selected = 0;
    std::vector<std::string> options;
    std::string fresh_install;
    std::string update;
    std::string themes;
    std::vector<std::string> logs;        // For storing script output
    std::string system_info;              // For storing system information
    std::string input_prompt;             // For storing user input prompts
};

struct Alerts {
    std::string info_text;
    std::string warning_text;
};

struct DlCloser { // Custom deleter to avoid noexcept warning
    void operator()(void* handle) const noexcept;
};

// Class declarations
class NcursesManager {
public:
    NcursesManager();
    ~NcursesManager();

private:
    void check_ncurses();
    void install_ncurses();
    void init_ncurses();
    void setup_transparent_background();
};

class TUI {
public:
    void draw_box(int endy, int endx, int starty = 0, int startx = 0);
    void layout(int max_y, int max_x);
    void draw_centered_multiline(int start_y, const std::vector<std::string>& lines);
    void draw_logo(int y);
};

// Function declarations
void update_logs(const std::vector<std::string>& logs, int max_y, int max_x);
void update_system_info(int max_y, int max_x);
void execute_script(const std::string& script_name, UserData& data);
std::pair<UserData, Alerts> main_menu(int max_y, int max_x, UserData& data);
// HYDE_TUI_HPP 