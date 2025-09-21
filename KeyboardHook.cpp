#include "KeyboardHook.h"
#include "WindowManager.h"
#include "UIManager.h"
#include "Settings.h"
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

static HHOOK hHook = NULL;

KeyboardHook* KeyboardHook::instance = nullptr;

LRESULT CALLBACK KeyboardHook::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && KeyboardHook::instance) {
        bool isKeyDown = !(lParam & 0x80000000);
        int keyCode = (int)wParam;
        
        // if (KeyboardHook::instance->uiManager) {
        //     std::string keyName = KeyboardHook::instance->GetKeyName(keyCode);
        //     std::string eventType = isKeyDown ? "按下" : "释放";
        //     KeyboardHook::instance->uiManager->AddLog("[调试] 键盘回调: " + keyName + " (" + eventType + ")");
        // }
        
        KeyboardHook::instance->HandleKeyEvent(keyCode, isKeyDown);
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

KeyboardHook::KeyboardHook() : settings(nullptr), windowManager(nullptr), uiManager(nullptr), 
                                hook(nullptr), isHooked(false), hookInstalled(false) {
    instance = this;
    InitializeKeyMap();
    
    foregroundKey.clear();
    bindedKey.clear();
    mainKey.clear();
    bindKey.clear();
    stackShowKey.clear();
}

KeyboardHook::~KeyboardHook() {
    if (hookInstalled) {
        Unhook();
    }
    instance = nullptr;
}

void KeyboardHook::InitializeKeyMap() {
    keyMap["ctrl"] = VK_CONTROL;
    keyMap["alt"] = VK_MENU;
    keyMap["shift"] = VK_SHIFT;
    keyMap["win"] = VK_LWIN;
    keyMap["capslk"] = VK_CAPITAL;
    keyMap["tab"] = VK_TAB;
    keyMap["enter"] = VK_RETURN;
    keyMap["esc"] = VK_ESCAPE;
    keyMap["space"] = VK_SPACE;
    keyMap["back"] = VK_BACK;
    keyMap["delete"] = VK_DELETE;
    keyMap["insert"] = VK_INSERT;
    keyMap["home"] = VK_HOME;
    keyMap["end"] = VK_END;
    keyMap["pageup"] = VK_PRIOR;
    keyMap["pagedown"] = VK_NEXT;
    keyMap["up"] = VK_UP;
    keyMap["down"] = VK_DOWN;
    keyMap["left"] = VK_LEFT;
    keyMap["right"] = VK_RIGHT;
    
    keyMap["f1"] = VK_F1;
    keyMap["f2"] = VK_F2;
    keyMap["f3"] = VK_F3;
    keyMap["f4"] = VK_F4;
    keyMap["f5"] = VK_F5;
    keyMap["f6"] = VK_F6;
    keyMap["f7"] = VK_F7;
    keyMap["f8"] = VK_F8;
    keyMap["f9"] = VK_F9;
    keyMap["f10"] = VK_F10;
    keyMap["f11"] = VK_F11;
    keyMap["f12"] = VK_F12;
    
    for (int i = 0; i <= 9; i++) {
        keyMap[std::to_string(i)] = 0x30 + i;
    }
    
    for (char c = 'a'; c <= 'z'; c++) {
        keyMap[std::string(1, c)] = 0x41 + (c - 'a');
    }
    
    keyMap["."] = VK_OEM_PERIOD;
    keyMap[","] = VK_OEM_COMMA;
    keyMap[";"] = VK_OEM_1;
    keyMap["'"] = VK_OEM_7;
    keyMap["/"] = VK_OEM_2;
    keyMap["\\"] = VK_OEM_5;
    keyMap["["] = VK_OEM_4;
    keyMap["]"] = VK_OEM_6;
    keyMap["-"] = VK_OEM_MINUS;
    keyMap["="] = VK_OEM_PLUS;
    keyMap["`"] = VK_OEM_3;
    keyMap["num0"] = VK_NUMPAD0;
    keyMap["num1"] = VK_NUMPAD1;
    keyMap["num2"] = VK_NUMPAD2;
    keyMap["num3"] = VK_NUMPAD3;
    keyMap["num4"] = VK_NUMPAD4;
    keyMap["num5"] = VK_NUMPAD5;
    keyMap["num6"] = VK_NUMPAD6;
    keyMap["num7"] = VK_NUMPAD7;
    keyMap["num8"] = VK_NUMPAD8;
    keyMap["num9"] = VK_NUMPAD9;
    keyMap["num*"] = VK_MULTIPLY;
    keyMap["num+"] = VK_ADD;
    keyMap["num-"] = VK_SUBTRACT;
    keyMap["num."] = VK_DECIMAL;
    keyMap["num/"] = VK_DIVIDE;
}

std::vector<int> KeyboardHook::ParseKeyString(const std::string& keyString) {
    std::vector<int> keys;
    std::istringstream iss(keyString);
    std::string token;
    
    while (std::getline(iss, token, '+')) {
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        
        auto it = keyMap.find(token);
        if (it != keyMap.end()) {
            keys.push_back(it->second);
        }
    }
    
    return keys;
}

bool KeyboardHook::CheckKeyCombination(const std::vector<int>& combination) {
    if (combination.empty()) {
        return false;
    }
    
    for (int key : combination) {
        if (!(GetAsyncKeyState(key) & 0x8000)) {
            return false;
        }
    }
    
    return true;
}

void KeyboardHook::HandleKeyEvent(int keyCode, bool isKeyDown) {
    if (!isKeyDown) {
        return;
    }
    
    bool isModifier = (keyCode == VK_CONTROL || keyCode == VK_MENU || keyCode == VK_SHIFT || keyCode == VK_LWIN || keyCode == VK_RWIN);
    
    // if (uiManager) {
    //     std::string keyName = GetKeyName(keyCode);
    //     uiManager->AddLog("[调试] 按键按下: " + keyName + " (代码: " + std::to_string(keyCode) + ")");
    // }
    
    // 检查按键绑定
    if (CheckKeyCombination(foregroundKey) && windowManager) {
        // if (uiManager) {
        //     uiManager->AddLog("[调试] 匹配到前台窗口切换按键");
        // }
        
        // 按键1：切换前台窗口显隐性
        if (settings->getBool("StackMode")) {
            HWND foregroundWindow = GetForegroundWindow();
            if (foregroundWindow) {
                windowManager->PushToStack(foregroundWindow);
            }
        } else {
            windowManager->ToggleForegroundWindow();
        }
        return;
    }
    
    if (CheckKeyCombination(bindedKey) && windowManager) {
        if (!settings->getBool("StackMode")) {
            windowManager->ToggleBindedWindow();
        }
        return;
    }
    
    if (CheckKeyCombination(mainKey) && uiManager) {
        uiManager->ToggleVisibility();
        return;
    }
    
    if (CheckKeyCombination(bindKey) && windowManager) {
        if (!settings->getBool("StackMode")) {
            windowManager->BindForegroundWindow();
        }
        return;
    }
    
    if (CheckKeyCombination(stackShowKey) && windowManager) {
        if (settings->getBool("StackMode")) {
            windowManager->PopFromStack();
        }
        return;
    }
}

void KeyboardHook::SetWindowManager(WindowManager* wm) {
    windowManager = wm;
}

void KeyboardHook::SetUIManager(UIManager* ui) {
    uiManager = ui;
}

void KeyboardHook::SetSettings(Settings* s) {
    settings = s;
}

void KeyboardHook::Initialize(Settings* settings, WindowManager* windowManager) {
    this->settings = settings;
    this->windowManager = windowManager;
    LoadKeyBindings();
}

bool KeyboardHook::Hook() {
    if (hookInstalled) {
        isHooked = true;
        return true;
    }
    
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook::KeyboardProc, GetModuleHandle(NULL), 0);
    if (hHook == NULL) {
        isHooked = false;
        if (uiManager) {
            uiManager->AddLog("[调试] 键盘钩子安装失败");
        }
        return false;
    }
    
    hookInstalled = true;
    isHooked = true;
    
    // if (uiManager) {
    //     uiManager->AddLog("[调试] 键盘钩子安装成功");
    // }
    
    return true;
}

void KeyboardHook::Unhook() {
    if (!hookInstalled) {
        return;
    }
    
    UnhookWindowsHookEx(hHook);
    hHook = NULL;
    hookInstalled = false;
    isHooked = false;
}

void KeyboardHook::LoadKeyBindings() {
    if (!settings) {
        return;
    }
    
    std::string fgKeyStr = settings->getString("KeyForeground");
    foregroundKey = ParseKeyString(fgKeyStr);
    
    // if (uiManager) {
    //     uiManager->AddLog("[调试] 已加载前台窗口按键: " + fgKeyStr + " (解析出 " + std::to_string(foregroundKey.size()) + " 个键)");
    // }
    
    std::string bdKeyStr = settings->getString("KeyBinded");
    bindedKey = ParseKeyString(bdKeyStr);
    
    std::string mainKeyStr = settings->getString("KeyMain");
    mainKey = ParseKeyString(mainKeyStr);
    
    std::string bindKeyStr = settings->getString("KeyBindWindow");
    bindKey = ParseKeyString(bindKeyStr);
    
    std::string stackKeyStr = settings->getString("KeyStackShow");
    stackShowKey = ParseKeyString(stackKeyStr);
}

void KeyboardHook::SaveKeyBindings() {
    if (!settings) {
        return;
    }
    
    std::string fgKeyStr = "";
    for (size_t i = 0; i < foregroundKey.size(); i++) {
        if (i > 0) fgKeyStr += "+";
        fgKeyStr += GetKeyName(foregroundKey[i]);
    }
    settings->setString("KeyForeground", fgKeyStr);
    
    std::string bdKeyStr = "";
    for (size_t i = 0; i < bindedKey.size(); i++) {
        if (i > 0) bdKeyStr += "+";
        bdKeyStr += GetKeyName(bindedKey[i]);
    }
    settings->setString("KeyBinded", bdKeyStr);
    
    std::string mainKeyStr = "";
    for (size_t i = 0; i < mainKey.size(); i++) {
        if (i > 0) mainKeyStr += "+";
        mainKeyStr += GetKeyName(mainKey[i]);
    }
    settings->setString("KeyMain", mainKeyStr);
    
    std::string bindKeyStr = "";
    for (size_t i = 0; i < bindKey.size(); i++) {
        if (i > 0) bindKeyStr += "+";
        bindKeyStr += GetKeyName(bindKey[i]);
    }
    settings->setString("KeyBindWindow", bindKeyStr);
    
    std::string stackKeyStr = "";
    for (size_t i = 0; i < stackShowKey.size(); i++) {
        if (i > 0) stackKeyStr += "+";
        stackKeyStr += GetKeyName(stackShowKey[i]);
    }
    settings->setString("KeyStackShow", stackKeyStr);
    
    settings->saveToFile("settings.ini");
}

std::string KeyboardHook::GetKeyName(int vkCode) {
    for (const auto& pair : keyMap) {
        if (pair.second == vkCode) {
            return pair.first;
        }
    }
    
    return "";
}

void KeyboardHook::SetForegroundKey(const std::vector<int>& keys) {
    foregroundKey = keys;
    SaveKeyBindings();
}

void KeyboardHook::SetBindedKey(const std::vector<int>& keys) {
    bindedKey = keys;
    SaveKeyBindings();
}

void KeyboardHook::SetMainKey(const std::vector<int>& keys) {
    mainKey = keys;
    SaveKeyBindings();
}

void KeyboardHook::SetBindKey(const std::vector<int>& keys) {
    bindKey = keys;
    SaveKeyBindings();
}

void KeyboardHook::SetStackShowKey(const std::vector<int>& keys) {
    stackShowKey = keys;
    SaveKeyBindings();
}

std::vector<int> KeyboardHook::GetForegroundKey() const {
    return foregroundKey;
}

std::vector<int> KeyboardHook::GetBindedKey() const {
    return bindedKey;
}

std::vector<int> KeyboardHook::GetMainKey() const {
    return mainKey;
}

std::vector<int> KeyboardHook::GetBindKey() const {
    return bindKey;
}

std::vector<int> KeyboardHook::GetStackShowKey() const {
    return stackShowKey;
}

bool KeyboardHook::IsHooked() {
    return isHooked;
}

bool KeyboardHook::IsHookInstalled() const {
    return hookInstalled;
}