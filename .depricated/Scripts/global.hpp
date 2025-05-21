#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

// Common structures and functions for all scripts
struct ScriptOutput {
    std::string message;
    enum class Type { INFO, SUCCESS, ERROR } type;
};

// Function to read requirements.json
json read_requirements() {
    std::ifstream file("requirements.json");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open requirements.json");
    }
    json data;
    file >> data;
    return data;
}

// Function to check root privileges
bool check_root() {
    return geteuid() == 0;
}

// Function to execute command and capture output with timeout
bool execute_command(const std::string& command, std::string& output, int timeout_seconds = 300) {
    std::string cmd = "timeout " + std::to_string(timeout_seconds) + " " + command + " 2>&1";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        output = "Failed to execute command: " + command;
        return false;
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        output += buffer;
    }
    
    int status = pclose(pipe);
    if (status == 124) { // timeout exit code
        output += "\nCommand timed out after " + std::to_string(timeout_seconds) + " seconds";
        return false;
    }
    return status == 0;
}

// Function to check if package exists in pacman
bool check_pacman_package(const std::string& package) {
    std::string output;
    return execute_command("pacman -Ss " + package, output) && output.find(package) != std::string::npos;
}

// Function to check if AUR helper is installed
bool check_aur_helper() {
    std::string output;
    return execute_command("which yay", output) || execute_command("which paru", output);
}