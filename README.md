# PacTUI

**PacTUI** is a Terminal User Interface for managing the pacman package manager on Arch Linux systems.

## Features

- Interactive package installation, removal, and updates
- Package search with filtering
- System update management
- Detailed package information viewing
- Package dependency visualization
- Configuration management
- Installation history and logs
- Favorites/bookmarks for frequently used packages

## Requirements

- Arch Linux or Arch-based distribution (Manjaro, EndeavourOS, etc.)
- C++ compiler (g++ or clang++)
- ncurses development library
- sudo privileges for package management operations

## Building from Source

1. Make sure you have the required dependencies:
   ```
   sudo pacman -S gcc ncurses
   ```

2. Build the application:
   ```
   g++ -o pactui src/*.cpp -lncurses -std=c++17
   ```

3. Run PacTUI:
   ```
   ./pactui
   ```

## Usage

PacTUI provides a simple terminal interface with the following keyboard controls:

- Up/Down arrows: Navigate menus and lists
- Enter: Select item or confirm action
- Backspace/Delete: Edit text input
- ESC: Go back or cancel
- q: Quit application
- ?: Display help

## Configuration

PacTUI creates a configuration file at `~/.config/pactui/config`. You can edit this file to change various settings, including:

- UI theme
- Confirmation prompts
- Sorting options
- Refresh rate
- Logging level

## License

MIT License