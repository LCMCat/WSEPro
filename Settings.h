#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <map>

class Settings {
private:
    std::map<std::string, std::string> stringSettings;
    std::map<std::string, int> intSettings;
    std::map<std::string, bool> boolSettings;
    
    // 默认设置
    void setDefaults();
    
public:
    Settings();
    ~Settings();
    
    // 从文件加载设置
    bool loadFromFile(const std::string& filename);
    
    // 保存设置到文件
    bool saveToFile(const std::string& filename);
    
    // 获取字符串设置
    std::string getString(const std::string& key, const std::string& defaultValue = "");
    
    // 设置字符串值
    void setString(const std::string& key, const std::string& value);
    
    // 获取整型设置
    int getInt(const std::string& key, int defaultValue = 0);
    
    // 设置整型值
    void setInt(const std::string& key, int value);
    
    // 获取布尔型设置
    bool getBool(const std::string& key, bool defaultValue = false);
    
    // 设置布尔型值
    void setBool(const std::string& key, bool value);
    
    // 检查设置是否存在
    bool hasSetting(const std::string& key);
    
    // 删除设置
    void removeSetting(const std::string& key);
    
    // 清除所有设置
    void clear();
};

#endif // SETTINGS_H