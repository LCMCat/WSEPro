#include "WindowManager.h"
#include "Settings.h"
#include "UIManager.h"
#include <psapi.h>
#include <iostream>
#include <algorithm>

WindowManager::WindowManager() {
    bindedWindow = NULL;
}

WindowManager::~WindowManager() {
}

void WindowManager::Initialize(Settings* settings) {
    this->settings = settings;
    this->uiManager = nullptr;
}

void WindowManager::SetUIManager(UIManager* uiManager) {
    this->uiManager = uiManager;
}

bool WindowManager::IsSystemWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return true;
    }
    
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!process) {
        return true;
    }
    
    char processName[MAX_PATH] = {0};
    if (!GetModuleFileNameExA(process, NULL, processName, MAX_PATH)) {
        CloseHandle(process);
        return true;
    }
    
    CloseHandle(process);
    
    std::string processNameStr(processName);
    std::transform(processNameStr.begin(), processNameStr.end(), processNameStr.begin(), ::tolower);
    
    if (processNameStr.find("explorer.exe") != std::string::npos ||
        processNameStr.find("system32") != std::string::npos ||
        processNameStr.find("syswow64") != std::string::npos) {
        return true;
    }
    
    return false;
}

bool WindowManager::IsConsoleWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return false;
    }
    
    HWND consoleWnd = GetConsoleWindow();
    
    return (hwnd == consoleWnd);
}

std::string WindowManager::GetWindowTitle(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return "";
    }
    
    char title[256];
    if (GetWindowTextA(hwnd, title, sizeof(title)) > 0) {
        return std::string(title);
    }
    
    return "";
}

void WindowManager::BindWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return;
    }
    
    if (this->settings->getBool("PreventSystematicBind", true) && IsSystemWindow(hwnd)) {
        return;
    }
    
    if (this->settings->getBool("PreventConsoleBind", true) && IsConsoleWindow(hwnd)) {
        return;
    }
    
    bindedWindow = hwnd;
    
    WINDOWPLACEMENT placement = { sizeof(placement) };
    GetWindowPlacement(hwnd, &placement);
    
    bool isMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
    
    for (auto it = hiddenWindows.begin(); it != hiddenWindows.end(); ++it) {
        if (it->hwnd == hwnd) {
            hiddenWindows.erase(it);
            break;
        }
    }
    
    auto it = windowMap.find(hwnd);
    if (it != windowMap.end()) {
        windowMap.erase(it);
    }
    
    WindowInfo info;
    info.hwnd = hwnd;
    info.title = GetWindowTitle(hwnd);
    info.isHidden = false;
    info.placement = placement;
    info.isMaximized = isMaximized;
    
    windowMap[hwnd] = info;
}

void WindowManager::ToggleWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return;
    }
    
    bool stackMode = this->settings->getBool("StackMode", false);
    
    if (stackMode) {
        auto it = std::find_if(hiddenWindows.begin(), hiddenWindows.end(), 
                              [hwnd](const WindowInfo& info) { return info.hwnd == hwnd; });
        
        if (it != hiddenWindows.end()) {
            ShowWindow(hwnd, it->isMaximized ? SW_SHOWMAXIMIZED : SW_RESTORE);
            SetWindowPlacement(hwnd, &it->placement);
            if (uiManager) {
                uiManager->AddLog("已显示窗口: " + it->title + " (句柄: " + std::to_string((long long)hwnd) + ")");
            }
            hiddenWindows.erase(it);
        } else {
            WINDOWPLACEMENT placement = { sizeof(placement) };
            GetWindowPlacement(hwnd, &placement);
            
            bool isMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
            
            WindowInfo info;
            info.hwnd = hwnd;
            info.title = GetWindowTitle(hwnd);
            info.isHidden = true;
            info.placement = placement;
            info.isMaximized = isMaximized;
            
            hiddenWindows.push_back(info);
            ShowWindow(hwnd, SW_HIDE);
            if (uiManager) {
                uiManager->AddLog("已隐藏窗口: " + info.title + " (句柄: " + std::to_string((long long)hwnd) + ")");
            }
        }
    } else {
        auto it = windowMap.find(hwnd);
        if (it != windowMap.end()) {
            if (it->second.isHidden) {
                ShowWindow(hwnd, it->second.isMaximized ? SW_SHOWMAXIMIZED : SW_RESTORE);
                SetWindowPlacement(hwnd, &it->second.placement);
                it->second.isHidden = false;
                if (uiManager) {
                    uiManager->AddLog("已显示窗口: " + it->second.title + " (句柄: " + std::to_string((long long)hwnd) + ")");
                }
            } else {
                WINDOWPLACEMENT placement = { sizeof(placement) };
                GetWindowPlacement(hwnd, &placement);
                
                bool isMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
                
                it->second.isHidden = true;
                it->second.placement = placement;
                it->second.isMaximized = isMaximized;
                
                ShowWindow(hwnd, SW_HIDE);
                if (uiManager) {
                    uiManager->AddLog("已隐藏窗口: " + it->second.title + " (句柄: " + std::to_string((long long)hwnd) + ")");
                }
            }
        } else {
            WINDOWPLACEMENT placement = { sizeof(placement) };
            GetWindowPlacement(hwnd, &placement);
            
            bool isMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
            
            WindowInfo info;
            info.hwnd = hwnd;
            info.title = GetWindowTitle(hwnd);
            info.isHidden = true;
            info.placement = placement;
            info.isMaximized = isMaximized;
            
            windowMap[hwnd] = info;
            ShowWindow(hwnd, SW_HIDE);
            if (uiManager) {
                uiManager->AddLog("已隐藏窗口: " + info.title + " (句柄: " + std::to_string((long long)hwnd) + ")");
            }
        }
    }
}

void WindowManager::ToggleBindedWindow() {
    if (!bindedWindow || !IsWindow(bindedWindow)) {
        return;
    }
    
    ToggleWindow(bindedWindow);
}

void WindowManager::ToggleMainWindow() {
    HWND consoleWnd = GetConsoleWindow();
    if (consoleWnd) {
        ToggleWindow(consoleWnd);
    }
}

void WindowManager::ShowTopStackWindow() {
    if (!settings->getBool("StackMode", false) || hiddenWindows.empty()) {
        return;
    }
    
    WindowInfo info = hiddenWindows.back();
    hiddenWindows.pop_back();
    
    ShowWindow(info.hwnd, info.isMaximized ? SW_SHOWMAXIMIZED : SW_RESTORE);
    SetWindowPlacement(info.hwnd, &info.placement);
    
    if (uiManager) {
        uiManager->AddLog("已显示窗口: " + info.title + " (句柄: " + std::to_string((long long)info.hwnd) + ")");
    }
}

void WindowManager::RestoreAllWindows() {
    bool stackMode = settings->getBool("StackMode", false);
    
    if (stackMode) {
        for (auto& info : hiddenWindows) {
            if (IsWindow(info.hwnd)) {
                ShowWindow(info.hwnd, info.isMaximized ? SW_SHOWMAXIMIZED : SW_RESTORE);
                SetWindowPlacement(info.hwnd, &info.placement);
            }
        }
        hiddenWindows.clear();
    } else {
        for (auto& pair : windowMap) {
            if (pair.second.isHidden && IsWindow(pair.first)) {
                ShowWindow(pair.first, pair.second.isMaximized ? SW_SHOWMAXIMIZED : SW_RESTORE);
                SetWindowPlacement(pair.first, &pair.second.placement);
            }
        }
        windowMap.clear();
    }
    
    if (bindedWindow && IsWindow(bindedWindow)) {
        auto it = windowMap.find(bindedWindow);
        if (it != windowMap.end() && it->second.isHidden) {
            ShowWindow(bindedWindow, it->second.isMaximized ? SW_SHOWMAXIMIZED : SW_RESTORE);
            SetWindowPlacement(bindedWindow, &it->second.placement);
        }
    }
}

void WindowManager::ShowHiddenWindowsInfo(void* uiManager) {
    UIManager* ui = static_cast<UIManager*>(uiManager);
    
    bool stackMode = settings->getBool("StackMode", false);
    
    if (stackMode) {
        if (hiddenWindows.empty()) {
            ui->AddLog("当前没有隐藏的窗口");
            return;
        }
        
        ui->AddLog("堆栈中的隐藏窗口:");
        for (size_t i = 0; i < hiddenWindows.size(); i++) {
            std::string log = "  [" + std::to_string(i + 1) + "] " + hiddenWindows[i].title;
            ui->AddLog(log);
        }
    } else {
        bool hasHidden = false;
        
        ui->AddLog("隐藏的窗口:");
        
        if (bindedWindow && IsWindow(bindedWindow)) {
            auto it = windowMap.find(bindedWindow);
            if (it != windowMap.end() && it->second.isHidden) {
                ui->AddLog("  [绑定窗口] " + it->second.title);
                hasHidden = true;
            }
        }
        
        for (auto& pair : windowMap) {
            if (pair.second.isHidden && pair.first != bindedWindow) {
                ui->AddLog("  " + pair.second.title);
                hasHidden = true;
            }
        }
        
        if (!hasHidden) {
            ui->AddLog("  当前没有隐藏的窗口");
        }
    }
}

HWND WindowManager::GetBindedWindow() {
    return bindedWindow;
}

bool WindowManager::IsWindowHidden(HWND hwnd) {
    bool stackMode = settings->getBool("StackMode", false);
    
    if (stackMode) {
        auto it = std::find_if(hiddenWindows.begin(), hiddenWindows.end(), 
                              [hwnd](const WindowInfo& info) { return info.hwnd == hwnd; });
        return (it != hiddenWindows.end());
    } else {
        auto it = windowMap.find(hwnd);
        return (it != windowMap.end() && it->second.isHidden);
    }
}

int WindowManager::GetHiddenWindowCount() {
    return hiddenWindows.size();
}

bool WindowManager::PopFromStack() {
    if (hiddenWindows.empty()) {
        return false;
    }
    
    WindowInfo& windowInfo = hiddenWindows.back();
    
    ShowWindow(windowInfo.hwnd, SW_SHOW);
    SetWindowPlacement(windowInfo.hwnd, &windowInfo.placement);
    
    if (uiManager) {
        uiManager->AddLog("已显示窗口: " + windowInfo.title + " (句柄: " + std::to_string((long long)windowInfo.hwnd) + ")");
    }
    
    hiddenWindows.pop_back();
    
    return true;
}

bool WindowManager::PushToStack(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return false;
    }
    
    auto it = std::find_if(hiddenWindows.begin(), hiddenWindows.end(), 
                          [hwnd](const WindowInfo& info) { return info.hwnd == hwnd; });
    
    if (it != hiddenWindows.end()) {
        hiddenWindows.erase(it);
    }
    
    WINDOWPLACEMENT placement = { sizeof(placement) };
    GetWindowPlacement(hwnd, &placement);
    
    bool isMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
    
    WindowInfo info;
    info.hwnd = hwnd;
    info.title = GetWindowTitle(hwnd);
    info.isHidden = true;
    info.placement = placement;
    info.isMaximized = isMaximized;
    
    hiddenWindows.push_back(info);
    
    ShowWindow(hwnd, SW_HIDE);
    
    if (uiManager) {
        uiManager->AddLog("已隐藏窗口: " + info.title + " (句柄: " + std::to_string((long long)hwnd) + ")");
    }
    
    return true;
}

bool WindowManager::ToggleForegroundWindow() {
    HWND foregroundWindow = GetForegroundWindow();
    if (!foregroundWindow) {
        return false;
    }
    
    if (IsSystemWindow(foregroundWindow) || IsConsoleWindow(foregroundWindow)) {
        return false;
    }
    
    auto it = windowMap.find(foregroundWindow);
    if (it != windowMap.end()) {
        WindowInfo& windowInfo = it->second;
        if (windowInfo.isHidden) {
            ShowWindow(foregroundWindow, SW_SHOW);
            SetWindowPlacement(foregroundWindow, &windowInfo.placement);
            windowInfo.isHidden = false;
            if (uiManager) {
                uiManager->AddLog("已显示窗口: " + windowInfo.title + " (句柄: " + std::to_string((long long)foregroundWindow) + ")");
            }
        } else {
            WINDOWPLACEMENT placement;
            GetWindowPlacement(foregroundWindow, &placement);
            windowInfo.placement = placement;
            windowInfo.isMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
            ShowWindow(foregroundWindow, SW_HIDE);
            windowInfo.isHidden = true;
            if (uiManager) {
                uiManager->AddLog("已隐藏窗口: " + windowInfo.title + " (句柄: " + std::to_string((long long)foregroundWindow) + ")");
            }
        }
    } else {
        WindowInfo windowInfo;
        windowInfo.hwnd = foregroundWindow;
        windowInfo.title = GetWindowTitle(foregroundWindow);
        windowInfo.isHidden = true;
        
        WINDOWPLACEMENT placement;
        GetWindowPlacement(foregroundWindow, &placement);
        windowInfo.placement = placement;
        windowInfo.isMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
        
        ShowWindow(foregroundWindow, SW_HIDE);
        windowMap[foregroundWindow] = windowInfo;
        if (uiManager) {
            uiManager->AddLog("已隐藏窗口: " + windowInfo.title + " (句柄: " + std::to_string((long long)foregroundWindow) + ")");
        }
    }
    
    return true;
}

bool WindowManager::BindForegroundWindow() {
    HWND foregroundWindow = GetForegroundWindow();
    if (!foregroundWindow) {
        return false;
    }
    
    if (this->settings->getBool("PreventSystematicBind", true) && IsSystemWindow(foregroundWindow)) {
        return false;
    }
    
    if (this->settings->getBool("PreventConsoleBind", true) && IsConsoleWindow(foregroundWindow)) {
        return false;
    }
    
    bindedWindow = foregroundWindow;
    
    return true;
}