#include "UI/WindowConfig.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace CADCore {

WindowConfig::WindowConfig(const std::string& configPath)
    : configPath_(configPath) {
    loadFromFile();
}

void WindowConfig::saveWindowSettings(const std::string& windowName, const WindowSettings& settings) {
    settings_[windowName] = settings;
}

WindowSettings WindowConfig::loadWindowSettings(const std::string& windowName) const {
    auto it = settings_.find(windowName);
    if (it != settings_.end()) {
        return it->second;
    }
    // Return default settings if not found
    WindowSettings defaults;
    defaults.windowName = windowName;
    return defaults;
}

bool WindowConfig::saveToFile() const {
    std::ofstream file(configPath_);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file for writing: " << configPath_ << std::endl;
        return false;
    }
    
    // Write header
    file << "# Window configuration file\n";
    file << "# Format: window_name|key=value|key=value|...\n";
    file << "# Keys: x, y, width, height, maximized, minimized, monitorIndex, monitorName, visible, focused\n";
    file << "# Lines starting with # are comments\n\n";
    
    // Write settings for each window
    for (const auto& pair : settings_) {
        file << pair.first << "|" << serializeSettings(pair.second) << "\n";
    }
    
    file.close();
    return true;
}

bool WindowConfig::loadFromFile() {
    settings_.clear();
    
    std::ifstream file(configPath_);
    if (!file.is_open()) {
        // File doesn't exist yet, that's OK
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Find window name (before first |)
        size_t pipePos = line.find('|');
        if (pipePos == std::string::npos) {
            continue;
        }
        
        std::string windowName = line.substr(0, pipePos);
        std::string settingsData = line.substr(pipePos + 1);
        
        WindowSettings settings = deserializeSettings(settingsData);
        settings.windowName = windowName;
        settings_[windowName] = settings;
    }
    
    file.close();
    return true;
}

std::string WindowConfig::serializeSettings(const WindowSettings& settings) const {
    std::ostringstream oss;
    
    if (settings.x != -1) oss << "x=" << settings.x << "|";
    if (settings.y != -1) oss << "y=" << settings.y << "|";
    if (settings.width != -1) oss << "width=" << settings.width << "|";
    if (settings.height != -1) oss << "height=" << settings.height << "|";
    if (settings.maximized) oss << "maximized=1|";
    if (settings.minimized) oss << "minimized=1|";
    if (settings.monitorIndex != -1) oss << "monitorIndex=" << settings.monitorIndex << "|";
    if (!settings.monitorName.empty()) oss << "monitorName=" << escapeString(settings.monitorName) << "|";
    if (!settings.visible) oss << "visible=0|";
    if (settings.focused) oss << "focused=1|";
    
    std::string result = oss.str();
    // Remove trailing |
    if (!result.empty() && result.back() == '|') {
        result.pop_back();
    }
    
    return result;
}

WindowSettings WindowConfig::deserializeSettings(const std::string& data) const {
    WindowSettings settings;
    
    std::istringstream iss(data);
    std::string token;
    
    while (std::getline(iss, token, '|')) {
        size_t eqPos = token.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }
        
        std::string key = token.substr(0, eqPos);
        std::string value = token.substr(eqPos + 1);
        
        if (key == "x") {
            settings.x = std::stoi(value);
        } else if (key == "y") {
            settings.y = std::stoi(value);
        } else if (key == "width") {
            settings.width = std::stoi(value);
        } else if (key == "height") {
            settings.height = std::stoi(value);
        } else if (key == "maximized") {
            settings.maximized = (value == "1");
        } else if (key == "minimized") {
            settings.minimized = (value == "1");
        } else if (key == "monitorIndex") {
            settings.monitorIndex = std::stoi(value);
        } else if (key == "monitorName") {
            settings.monitorName = unescapeString(value);
        } else if (key == "visible") {
            settings.visible = (value != "0");
        } else if (key == "focused") {
            settings.focused = (value == "1");
        }
    }
    
    return settings;
}

std::string WindowConfig::escapeString(const std::string& str) const {
    std::string result;
    for (char c : str) {
        if (c == '|' || c == '=' || c == '\\') {
            result += '\\';
        }
        result += c;
    }
    return result;
}

std::string WindowConfig::unescapeString(const std::string& str) const {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            result += str[i + 1];
            ++i;
        } else {
            result += str[i];
        }
    }
    return result;
}

} // namespace CADCore

