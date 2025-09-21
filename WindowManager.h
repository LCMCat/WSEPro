#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>

class Settings;
class UIManager;

struct WindowInfo {
    HWND hwnd;
    std::string title;
    bool isHidden;
    WINDOWPLACEMENT placement;
    bool isMaximized;
};

class WindowManager {
private:
    Settings* settings;
    UIManager* uiManager;
    HWND bindedWindow;  // 绑定的窗口
    std::vector<WindowInfo> hiddenWindows;  // 隐藏的窗口列表（用于堆栈模式）
    std::map<HWND, WindowInfo> windowMap;   // 窗口映射（用于非堆栈模式）
    
    // 检查是否为系统窗口
    bool IsSystemWindow(HWND hwnd);
    
    // 检查是否为控制台窗口
    bool IsConsoleWindow(HWND hwnd);
    
    // 获取窗口标题
    std::string GetWindowTitle(HWND hwnd);
    
public:
    WindowManager();
    ~WindowManager();
    
    // 初始化
    void Initialize(Settings* settings);
    
    // 设置UI管理器
    void SetUIManager(UIManager* uiManager);
    
    // 绑定窗口
    void BindWindow(HWND hwnd);
    
    // 切换窗口显示/隐藏状态
    void ToggleWindow(HWND hwnd);
    
    // 切换绑定窗口A的显示/隐藏状态
    void ToggleBindedWindow();
    
    // 切换主窗口的显示/隐藏状态
    void ToggleMainWindow();
    
    // 显示堆栈中最顶层的窗口
    void ShowTopStackWindow();
    
    // 恢复所有隐藏的窗口
    void RestoreAllWindows();
    
    // 显示隐藏窗口信息
    void ShowHiddenWindowsInfo(void* uiManager);
    
    // 获取绑定窗口
    HWND GetBindedWindow();
    
    // 检查窗口是否隐藏
    bool IsWindowHidden(HWND hwnd);
    
    // 获取隐藏窗口数量
    int GetHiddenWindowCount();
    
    // 从栈中弹出窗口
    bool PopFromStack();
    
    // 将窗口推入栈中
    bool PushToStack(HWND hwnd);
    
    // 切换前台窗口显隐性
    bool ToggleForegroundWindow();
    
    // 绑定前台窗口
    bool BindForegroundWindow();
};

#endif // WINDOWMANAGER_H