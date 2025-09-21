#include "Settings.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

Settings::Settings() {
    setDefaults();
}

Settings::~Settings() {
}

void Settings::setDefaults() {
    stringSettings["KeyForeground"] = "";
    stringSettings["KeyBinded"] = "";
    stringSettings["KeyMain"] = "";
    stringSettings["KeyBindWindow"] = "";
    stringSettings["KeyStackShow"] = "";
    
    boolSettings["PreventSystematicBind"] = true;
    boolSettings["PreventConsoleBind"] = true;
    
    boolSettings["TrayMinimized"] = true;
    intSettings["ConsoleLine"] = 20;
    intSettings["ConsoleColumn"] = 70;
    stringSettings["ConsoleColor"] = "9f";
    boolSettings["ExitRestoration"] = true;
    
    boolSettings["StackMode"] = false;
    boolSettings["AutoStartHook"] = true;
}

bool Settings::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (boolSettings.find(key) != boolSettings.end()) {
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            boolSettings[key] = (value == "true" || value == "1" || value == "yes" || value == "是");
        } else if (intSettings.find(key) != intSettings.end()) {
            try {
                intSettings[key] = std::stoi(value);
            } catch (...) {
            }
        } else {
            stringSettings[key] = value;
        }
    }
    
    file.close();
    return true;
}

bool Settings::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "# 窗口隐藏器 Pro 配置文件\n";
    file << "# Windows Showing Engine Pro Settings\n\n";
    
    for (const auto& pair : stringSettings) {
        file << pair.first << "=" << pair.second << "\n";
    }
    
    for (const auto& pair : intSettings) {
        file << pair.first << "=" << pair.second << "\n";
    }
    
    for (const auto& pair : boolSettings) {
        file << pair.first << "=" << (pair.second ? "true" : "false") << "\n";
    }
    
    file.close();
    return true;
}

std::string Settings::getString(const std::string& key, const std::string& defaultValue) {
    auto it = stringSettings.find(key);
    if (it != stringSettings.end()) {
        return it->second;
    }
    return defaultValue;
}

void Settings::setString(const std::string& key, const std::string& value) {
    stringSettings[key] = value;
}

int Settings::getInt(const std::string& key, int defaultValue) {
    auto it = intSettings.find(key);
    if (it != intSettings.end()) {
        return it->second;
    }
    return defaultValue;
}

void Settings::setInt(const std::string& key, int value) {
    intSettings[key] = value;
}

bool Settings::getBool(const std::string& key, bool defaultValue) {
    auto it = boolSettings.find(key);
    if (it != boolSettings.end()) {
        return it->second;
    }
    return defaultValue;
}

void Settings::setBool(const std::string& key, bool value) {
    boolSettings[key] = value;
}

bool Settings::hasSetting(const std::string& key) {
    return (stringSettings.find(key) != stringSettings.end() ||
            intSettings.find(key) != intSettings.end() ||
            boolSettings.find(key) != boolSettings.end());
}

void Settings::removeSetting(const std::string& key) {
    stringSettings.erase(key);
    intSettings.erase(key);
    boolSettings.erase(key);
}

void Settings::clear() {
    stringSettings.clear();
    intSettings.clear();
    boolSettings.clear();
    setDefaults();
}