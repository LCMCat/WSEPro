#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <windows.h>
#include <string>
#include <queue>
#include <vector>

class Settings;

class UIManager {
private:
    Settings* settings;
    HANDLE consoleOutput;
    HANDLE consoleInput;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    std::queue<std::string> logQueue;
    std::string inputBuffer;
    std::string savedCommand;
    int maxLogLines;
    int inputLine;
    bool isVisible;
    bool commandReady;
    
    // 初始化控制台
    void InitializeConsole();
    
    // 更新输入显示
    void UpdateInputDisplay();
public:
    UIManager();
    ~UIManager();
    
    // 初始化
    void Initialize(Settings* settings);
    
    // 处理控制台事件
    void HandleConsoleEvent(INPUT_RECORD& ir);
    
    // 添加日志
    void AddLog(const std::string& log);
    
    // 获取输入
    std::string GetInput();
    
    // 刷新界面
    void Refresh();
    
    // 清屏
    void Clear();
    
    // 设置可见性
    void SetVisible(bool visible);
    
    // 获取可见性
    bool IsVisible() const;
    
    // 切换界面可见性
    void ToggleVisibility();
    
    // 获取控制台句柄
    HANDLE GetConsoleOutput() const;
    HANDLE GetConsoleInput() const;
    
    // 获取控制台信息
    const CONSOLE_SCREEN_BUFFER_INFO& GetConsoleInfo() const;
    
    // 获取日志队列
    const std::queue<std::string>& GetLogQueue() const;
    
    // 获取输入缓冲区
    const std::string& GetInputBuffer() const;
    
    // 设置输入缓冲区
    void SetInputBuffer(const std::string& buffer);
    
    // 获取最大日志行数
    int GetMaxLogLines() const;
    
    // 设置最大日志行数
    void SetMaxLogLines(int lines);
    
    // 获取输入行
    int GetInputLine() const;
    
    // 设置输入行
    void SetInputLine(int line);
    
    // 检查命令是否就绪
    bool IsCommandReady() const;
    
    // 重置命令就绪状态
    void ResetCommandReady();
};

#endif // UIMANAGER_H