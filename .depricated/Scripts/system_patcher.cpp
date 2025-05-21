#include "global.hpp"
#include <sys/stat.h>

namespace fs = std::filesystem;

class SystemPatcher {
private:
    std::vector<std::string> logs;
    bool is_initialized;
    std::string sudo_password;

    bool execute_sudo_command(const std::string& command, std::string& output) {
        // Create a temporary script to handle sudo
        std::string script_path = "/tmp/sudo_script_" + std::to_string(getpid()) + ".sh";
        std::ofstream script(script_path);
        script << "#!/bin/bash\n";
        script << "echo '" << sudo_password << "' | sudo -S " << command << "\n";
        script.close();
        
        // Make the script executable
        chmod(script_path.c_str(), 0700);
        
        // Execute the script with a timeout
        bool result = execute_command(script_path, output, 60);
        
        // Clean up
        unlink(script_path.c_str());
        return result;
    }

public:
    SystemPatcher() : is_initialized(false) {}

    bool initialize(const std::string& password) {
        if (is_initialized) {
            add_log("SystemPatcher already initialized");
            return true;
        }

        sudo_password = password;
        std::string output;
        if (!execute_sudo_command("true", output)) {
            add_log("Error: Invalid sudo password");
            return false;
        }

        is_initialized = true;
        add_log("SystemPatcher initialized successfully");
        return true;
    }

    void add_log(const std::string& message) {
        logs.push_back(message);
    }

    std::vector<std::string> get_logs() const {
        return logs;
    }

    void clear_logs() {
        logs.clear();
    }

    bool update_mirrorlist() {
        if (!is_initialized) {
            add_log("Error: SystemPatcher not initialized");
            return false;
        }

        std::string output;
        if (execute_sudo_command("reflector --latest 5 --sort rate --save /etc/pacman.d/mirrorlist", output)) {
            add_log("Mirrorlist updated successfully");
            return true;
        }
        add_log("Failed to update mirrorlist");
        return false;
    }

    bool check_aur() {
        if (!is_initialized) return false;
        
        if (check_aur_helper()) {
            add_log("AUR helper detected");
            return true;
        }
        
        add_log("No AUR helper detected");
        return false;
    }

    std::vector<std::string> get_aur_options() {
        return {"yay", "paru"};
    }

    bool install_aur_helper(const std::string& helper) {
        if (!is_initialized) return false;

        std::string output;
        if (helper == "yay") {
            if (execute_sudo_command("git clone https://aur.archlinux.org/yay.git /tmp/yay && cd /tmp/yay && makepkg -si --noconfirm", output)) {
                add_log("yay installed successfully");
                return true;
            }
        } else if (helper == "paru") {
            if (execute_sudo_command("git clone https://aur.archlinux.org/paru.git /tmp/paru && cd /tmp/paru && makepkg -si --noconfirm", output)) {
                add_log("paru installed successfully");
                return true;
            }
        }
        
        add_log("Failed to install AUR helper: " + helper);
        return false;
    }

    // Example patch function
    bool apply_patch(const std::string& patch_name) {
        if (!is_initialized) {
            add_log("Error: SystemPatcher not initialized");
            return false;
        }

        add_log("Applying patch: " + patch_name);
        // Add actual patching logic here
        return true;
    }
};

// C-style interface for compatibility with the TUI
extern "C" {
    SystemPatcher* create_patcher() {
        return new SystemPatcher();
    }

    void destroy_patcher(SystemPatcher* patcher) {
        delete patcher;
    }

    bool initialize_patcher(SystemPatcher* patcher, const char* password) {
        return patcher->initialize(password);
    }

    void add_patcher_log(SystemPatcher* patcher, const char* message) {
        patcher->add_log(message);
    }

    char* get_patcher_logs(SystemPatcher* patcher) {
        std::string combined_logs;
        for (const auto& log : patcher->get_logs()) {
            combined_logs += log + "\n";
        }
        
        char* result = new char[combined_logs.length() + 1];
        strcpy(result, combined_logs.c_str());
        return result;
    }

    void free_patcher_logs(char* logs) {
        delete[] logs;
    }

    bool update_system_mirrorlist(SystemPatcher* patcher) {
        return patcher->update_mirrorlist();
    }

    bool check_aur_availability(SystemPatcher* patcher) {
        return patcher->check_aur();
    }

    bool install_aur_helper(SystemPatcher* patcher, const char* helper) {
        return patcher->install_aur_helper(helper);
    }

    bool apply_system_patch(SystemPatcher* patcher, const char* patch_name) {
        return patcher->apply_patch(patch_name);
    }
} 