#!/bin/bash

# Theme patcher script for HyDE-TUI
# Reads theme URLs from themes.lst and patches themes to system locations

THEMES_FILE="themes.lst"
THEMES_DIR="$HOME/.config/hyprland/themes"

# System directories for different theme components
declare -A THEME_DIRS=(
    ["gtk"]="$HOME/.local/share/themes"
    ["icons"]="$HOME/.local/share/icons"
    ["cursors"]="$HOME/.local/share/icons"
    ["sddm"]="/usr/share/sddm/themes"
    ["fonts"]="$HOME/.local/share/fonts"
    ["wallpapers"]="$HOME/.local/share/wallpapers"
    ["hyprland"]="$HOME/.config/hyprland"
)

# Function to print status messages that will be captured by TUI
print_status() {
    echo "[STATUS] $1"
}

print_error() {
    echo "[ERROR] $1"
}

print_success() {
    echo "[SUCCESS] $1"
}

# Function to create directory if it doesn't exist
ensure_dir() {
    local dir="$1"
    if [ ! -d "$dir" ]; then
        print_status "Creating directory: $dir"
        mkdir -p "$dir"
    fi
}

# Function to patch a theme component
patch_component() {
    local source="$1"
    local target="$2"
    local component="$3"

    if [ -d "$source" ]; then
        print_status "Patching $component to $target"
        if [ -w "$target" ]; then
            cp -rf "$source"/* "$target/"
            print_success "Patched $component successfully"
        else
            print_error "No write permission for $target"
            print_status "Attempting to patch with sudo..."
            if sudo cp -rf "$source"/* "$target/"; then
                print_success "Patched $component successfully with sudo"
            else
                print_error "Failed to patch $component"
            fi
        fi
    fi
}

# Check if themes.lst exists
if [ ! -f "$THEMES_FILE" ]; then
    print_error "themes.lst not found in current directory"
    exit 1
fi

# Create all required theme directories
for dir in "${THEME_DIRS[@]}"; do
    ensure_dir "$dir"
done

# Read and process each theme URL
while IFS= read -r theme_url || [ -n "$theme_url" ]; do
    # Skip empty lines and comments
    [[ -z "$theme_url" || "$theme_url" =~ ^[[:space:]]*# ]] && continue
    
    # Extract theme name from URL
    theme_name=$(basename "$theme_url" .git)
    theme_path="$THEMES_DIR/$theme_name"
    
    print_status "Processing theme: $theme_name"
    
    # Check if theme already exists
    if [ -d "$theme_path" ]; then
        print_status "Theme $theme_name already exists, updating..."
        cd "$theme_path" || exit 1
        git pull
        if [ $? -eq 0 ]; then
            print_success "Updated theme: $theme_name"
        else
            print_error "Failed to update theme: $theme_name"
            continue
        fi
    else
        print_status "Cloning theme: $theme_name"
        git clone "$theme_url" "$theme_path"
        if [ $? -ne 0 ]; then
            print_error "Failed to clone theme: $theme_name"
            continue
        fi
        print_success "Cloned theme: $theme_name"
    fi

    # Patch theme components
    cd "$theme_path" || continue

    # Look for theme components in common locations
    if [ -d "gtk" ]; then
        patch_component "gtk" "${THEME_DIRS[gtk]}" "GTK theme"
    fi
    if [ -d "icons" ]; then
        patch_component "icons" "${THEME_DIRS[icons]}" "icons"
    fi
    if [ -d "cursors" ]; then
        patch_component "cursors" "${THEME_DIRS[cursors]}" "cursors"
    fi
    if [ -d "sddm" ]; then
        patch_component "sddm" "${THEME_DIRS[sddm]}" "SDDM theme"
    fi
    if [ -d "fonts" ]; then
        patch_component "fonts" "${THEME_DIRS[fonts]}" "fonts"
    fi
    if [ -d "wallpapers" ]; then
        patch_component "wallpapers" "${THEME_DIRS[wallpapers]}" "wallpapers"
    fi
    if [ -d "hyprland" ]; then
        patch_component "hyprland" "${THEME_DIRS[hyprland]}" "Hyprland config"
    fi

    # Check for theme files in root directory
    for file in *.theme *.conf config.*; do
        if [ -f "$file" ]; then
            print_status "Found theme file: $file"
            cp -f "$file" "${THEME_DIRS[hyprland]}/"
            print_success "Patched theme file: $file"
        fi
    done

done < "$THEMES_FILE"

print_success "Theme patching completed"
