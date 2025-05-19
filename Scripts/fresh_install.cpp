#include "global.hpp"

namespace fs = std::filesystem;

class SystemInstaller {
private:
    std::vector<std::string> pacman_packages;
    std::vector<std::string> aur_packages;
    std::vector<std::string> logs;

    void add_log(const std::string& message) {
        logs.push_back(message);
    }

    bool categorize_packages() {
        try {
            json requirements = read_requirements();
            for (const auto& package : requirements["packages"]) {
                std::string pkg = package.get<std::string>();
                if (check_pacman_package(pkg)) {
                    pacman_packages.push_back(pkg);
                    add_log("Package " + pkg + " will be installed via pacman");
                } else {
                    aur_packages.push_back(pkg);
                    add_log("Package " + pkg + " will be installed via AUR");
                }
            }
            return true;
        } catch (const std::exception& e) {
            add_log("Error reading requirements: " + std::string(e.what()));
            return false;
        }
    }

    bool install_pacman_packages() {
        if (pacman_packages.empty()) {
            add_log("No packages to install via pacman");
            return true;
        }

        std::string package_list;
        for (const auto& pkg : pacman_packages) {
            package_list += pkg + " ";
        }

        std::string output;
        if (execute_command("pacman -S --noconfirm " + package_list, output)) {
            add_log("Successfully installed pacman packages");
            return true;
        }
        add_log("Failed to install pacman packages");
        return false;
    }

    bool install_aur_packages() {
        if (aur_packages.empty()) {
            add_log("No packages to install via AUR");
            return true;
        }

        std::string aur_helper;
        if (execute_command("which yay", aur_helper)) {
            aur_helper = "yay";
        } else if (execute_command("which paru", aur_helper)) {
            aur_helper = "paru";
        } else {
            add_log("No AUR helper found");
            return false;
        }

        for (const auto& pkg : aur_packages) {
            std::string output;
            if (!execute_command(aur_helper + " -S --noconfirm " + pkg, output)) {
                add_log("Failed to install AUR package: " + pkg);
                return false;
            }
            add_log("Successfully installed AUR package: " + pkg);
        }
        return true;
    }

    bool check_root() {
        return geteuid() == 0;
    }

public:
    bool install() {
        if (!check_root()) {
            add_log("Error: This script must be run as root");
            return false;
        }

        add_log("Starting package installation...");

        if (!categorize_packages()) {
            return false;
        }

        if (!install_pacman_packages()) {
            return false;
        }

        if (!install_aur_packages()) {
            return false;
        }

        add_log("All packages installed successfully!");
        return true;
    }

    std::vector<std::string> get_logs() const {
        return logs;
    }

    bool configure_system() {
        std::cout << "Configuring system..." << std::endl;

        // Set timezone
        std::string output;
        if (!execute_command("ln -sf /usr/share/zoneinfo/UTC /etc/localtime", output)) {
            std::cerr << "Failed to set timezone" << std::endl;
            return false;
        }

        // Generate locale
        if (!execute_command("locale-gen", output)) {
            std::cerr << "Failed to generate locale" << std::endl;
            return false;
        }

        // Enable NetworkManager
        if (!execute_command("systemctl enable NetworkManager", output)) {
            std::cerr << "Failed to enable NetworkManager" << std::endl;
            return false;
        }

        std::cout << "System configuration completed!" << std::endl;
        return true;
    }
};

// C-style interface for compatibility with the TUI
extern "C" {
    SystemInstaller* create_installer() {
        return new SystemInstaller();
    }

    void destroy_installer(SystemInstaller* installer) {
        delete installer;
    }

    bool run_installation(SystemInstaller* installer) {
        return installer->install();
    }

    char* get_installer_logs(SystemInstaller* installer) {
        std::string combined_logs;
        for (const auto& log : installer->get_logs()) {
            combined_logs += log + "\n";
        }
        
        char* result = new char[combined_logs.length() + 1];
        strcpy(result, combined_logs.c_str());
        return result;
    }

    void free_installer_logs(char* logs) {
        delete[] logs;
    }
}

int main() {
    SystemInstaller installer;
    
    if (!installer.install()) {
        std::cerr << "Installation failed!" << std::endl;
        return 1;
    }

    if (!installer.configure_system()) {
        std::cerr << "System configuration failed!" << std::endl;
        return 1;
    }

    std::cout << "Fresh installation completed successfully!" << std::endl;
    return 0;
} 