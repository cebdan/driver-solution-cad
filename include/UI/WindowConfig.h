#pragma once

#include <string>
#include <vector>
#include <map>

namespace CADCore {

/// Window settings structure
struct WindowSettings {
    std::string windowName;
    int x = -1;              // -1 means use default
    int y = -1;
    int width = -1;
    int height = -1;
    bool maximized = false;
    bool minimized = false;
    int monitorIndex = -1;   // -1 means use default/primary monitor
    std::string monitorName; // Name of the monitor (if available)
    
    // Additional settings
    bool visible = true;
    bool focused = false;
};

/// Configuration manager for window settings
class WindowConfig {
public:
    WindowConfig(const std::string& configPath = "close.config");
    
    // Save window settings
    void saveWindowSettings(const std::string& windowName, const WindowSettings& settings);
    
    // Load window settings
    WindowSettings loadWindowSettings(const std::string& windowName) const;
    
    // Save all settings to file
    bool saveToFile() const;
    
    // Load all settings from file
    bool loadFromFile();
    
    // Get config file path
    std::string getConfigPath() const { return configPath_; }
    
private:
    std::string configPath_;
    std::map<std::string, WindowSettings> settings_;
    
    // Helper methods for parsing
    std::string serializeSettings(const WindowSettings& settings) const;
    WindowSettings deserializeSettings(const std::string& data) const;
    std::string escapeString(const std::string& str) const;
    std::string unescapeString(const std::string& str) const;
};

} // namespace CADCore

