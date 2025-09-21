#ifndef KEYBOARDHOOK_H
#define KEYBOARDHOOK_H

#include <windows.h>
#include <string>
#include <map>
#include <vector>

class Settings;
class WindowManager;
class UIManager;

class KeyboardHook {
private:
    Settings* settings;
    WindowManager* windowManager;
    UIManager* uiManager;
    HHOOK hook;
    bool isHooked;
    bool hookInstalled;
    
    // 键码映射
    std::map<std::string, int> keyMap;
    
    // 按键绑定
    std::vector<int> foregroundKey;   // 切换前台窗口显隐性
    std::vector<int> bindedKey;      // 切换绑定窗口显隐性
    std::vector<int> mainKey;        // 切换主界面窗口显隐性
    std::vector<int> bindKey;        // 绑定前台窗口
    std::vector<int> stackShowKey;   // 显示栈内隐藏窗口
    
    // 按键状态
    std::map<int, bool> keyStates;
    
    // 初始化键码映射
    void InitializeKeyMap();
    
    // 解析按键字符串
    std::vector<int> ParseKeyString(const std::string& keyStr);
    
    // 检查按键组合是否匹配
    bool CheckKeyCombination(const std::vector<int>& keys);
    
    // 键盘钩子回调函数
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    
public:
    static KeyboardHook* instance;
    
    KeyboardHook();
    ~KeyboardHook();
    
    // 初始化
    void Initialize(Settings* settings, WindowManager* windowManager);
    
    // 安装钩子
    bool Hook();
    
    // 卸载钩子
    void Unhook();
    
    // 获取按键绑定
    std::vector<int> GetForegroundKey() const;
    std::vector<int> GetBindedKey() const;
    std::vector<int> GetMainKey() const;
    std::vector<int> GetBindKey() const;
    std::vector<int> GetStackShowKey() const;
    
    // 检查钩子是否已安装
    bool IsHookInstalled() const;
    bool IsHooked();
    
    // 处理键盘事件
    void HandleKeyEvent(int keyCode, bool isKeyDown);
    
    // 设置管理器
    void SetWindowManager(WindowManager* wm);
    void SetUIManager(UIManager* ui);
    void SetSettings(Settings* s);
    
    // 加载和保存按键绑定
    void LoadKeyBindings();
    void SaveKeyBindings();
    
    // 获取键名
    std::string GetKeyName(int vkCode);
    
    // 设置按键绑定
    void SetForegroundKey(const std::vector<int>& keys);
    void SetBindedKey(const std::vector<int>& keys);
    void SetMainKey(const std::vector<int>& keys);
    void SetBindKey(const std::vector<int>& keys);
    void SetStackShowKey(const std::vector<int>& keys);
};

#endif // KEYBOARDHOOK_H