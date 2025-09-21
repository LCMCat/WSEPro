#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <conio.h>
#include <shlobj.h>
#include <tlhelp32.h>

#include "Settings.h"
#include "WindowManager.h"
#include "KeyboardHook.h"
#include "UIManager.h"

Settings settings;
WindowManager windowManager;
KeyboardHook keyboardHook;
UIManager uiManager;
bool isRunning = true;

void InitializeConsole();
void LoadSettings();
void SaveSettings();
void ProcessCommand(const std::string& command);
void ShowHelp(const std::string& topic = "");
void ShowAbout();
void ShowInfo();
void ProcessBindCommand(const std::vector<std::string>& args);
void ProcessBindWindowCommand(const std::vector<std::string>& args);
void ProcessSettingsCommand(const std::vector<std::string>& args);
void ToggleKeyboardHook();

int main() {
    InitializeConsole();
    
    LoadSettings();
    
    windowManager.Initialize(&settings);
    windowManager.SetUIManager(&uiManager);
    
    keyboardHook.Initialize(&settings, &windowManager);
    keyboardHook.SetUIManager(&uiManager);
    keyboardHook.Hook();
    
    uiManager.Initialize(&settings);
    
    uiManager.AddLog("窗口隐藏器 高级版 已启动");
    uiManager.AddLog("输入 'help' 查看帮助");
    
    HANDLE consoleInput = uiManager.GetConsoleInput();
    DWORD mode;
    GetConsoleMode(consoleInput, &mode);
    mode |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    SetConsoleMode(consoleInput, mode);
    
    while (isRunning) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        DWORD numberOfEvents;
        GetNumberOfConsoleInputEvents(consoleInput, &numberOfEvents);
        
        if (numberOfEvents > 0) {
            INPUT_RECORD ir;
            DWORD eventsRead;
            
            ReadConsoleInput(consoleInput, &ir, 1, &eventsRead);
            
            uiManager.HandleConsoleEvent(ir);
        }
        
        if (uiManager.IsCommandReady()) {
            std::string input = uiManager.GetInput();
            
            if (!input.empty()) {
                ProcessCommand(input);
            }
            
            uiManager.ResetCommandReady();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    SaveSettings();
    
    if (settings.getBool("ExitRestoration", true)) {
        windowManager.RestoreAllWindows();
    }
    
    return 0;
}

void InitializeConsole() {
    system("title WSE Pro");
    
    int cols = 70;
    int lines = 20;
    
    std::ifstream configFile("settings.ini");
    if (configFile.is_open()) {
        std::string line;
        while (std::getline(configFile, line)) {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "ConsoleColumn") {
                    try { cols = std::stoi(value); } catch (...) {}
                } else if (key == "ConsoleLine") {
                    try { lines = std::stoi(value); } catch (...) {}
                }
            }
        }
        configFile.close();
    }
    
    std::string sizeCmd = "mode con:cols=" + std::to_string(cols) + " lines=" + std::to_string(lines);
    system(sizeCmd.c_str());
    
    std::string color = "9f";
    
    configFile.open("settings.ini");
    if (configFile.is_open()) {
        std::string line;
        while (std::getline(configFile, line)) {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "ConsoleColor") {
                    color = value;
                    break;
                }
            }
        }
        configFile.close();
    }
    
    std::string colorCmd = "color " + color;
    system(colorCmd.c_str());
    
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow) {
        HMENU hMenu = GetSystemMenu(consoleWindow, FALSE);
        if (hMenu) {
            DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
            DrawMenuBar(consoleWindow);
        }
        
        LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        SetWindowLong(consoleWindow, GWL_STYLE, style);
        SetWindowPos(consoleWindow, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}

void LoadSettings() {
    settings.loadFromFile("settings.ini");
}

void SaveSettings() {
    settings.saveToFile("settings.ini");
}

void ProcessCommand(const std::string& command) {
    std::istringstream iss(command);
    std::vector<std::string> args;
    std::string arg;
    
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    if (args.empty()) {
        return;
    }
    
    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    if (cmd == "help" || cmd == "帮助" || cmd == "?" || cmd == "？") {
        if (args.size() > 1) {
            ShowHelp(args[1]);
        } else {
            ShowHelp();
        }
    } else if (cmd == "bind" || cmd == "绑定" || cmd == "b") {
        ProcessBindCommand(std::vector<std::string>(args.begin() + 1, args.end()));
    } else if (cmd == "bindwindow" || cmd == "绑定窗口" || cmd == "bw") {
        ProcessBindWindowCommand(std::vector<std::string>(args.begin() + 1, args.end()));
    } else if (cmd == "settings" || cmd == "设置" || cmd == "set" || cmd == "s") {
        ProcessSettingsCommand(std::vector<std::string>(args.begin() + 1, args.end()));
    } else if (cmd == "toggle" || cmd == "t") {
        ToggleKeyboardHook();
    } else if (cmd == "about" || cmd == "关于" || cmd == "a") {
        ShowAbout();
    } else if (cmd == "info" || cmd == "信息" || cmd == "i") {
        ShowInfo();
    } else if (cmd == "exit" || cmd == "quit" || cmd == "q") {
        isRunning = false;
        uiManager.AddLog("程序即将退出...");
    } else {
        uiManager.AddLog("未知命令: " + cmd + "，输入 'help' 查看帮助");
    }
}

void ShowHelp(const std::string& topic) {
    if (topic.empty()) {
        uiManager.AddLog("可用命令:");
        uiManager.AddLog("  help [cmd]       - 显示帮助信息");
        uiManager.AddLog("  bind             - 按键绑定设置");
        uiManager.AddLog("  bindwindow       - 绑定窗口");
        uiManager.AddLog("  settings         - 修改设置");
        uiManager.AddLog("  toggle           - 切换键盘监听器");
        uiManager.AddLog("  about            - 显示关于信息");
        uiManager.AddLog("  info             - 显示隐藏窗口信息");
        uiManager.AddLog("  exit             - 退出程序");
        uiManager.AddLog("输入 'help [命令名]' 查看特定命令的详细帮助");
    } else {
        std::string cmd = topic;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
        
        if (cmd == "bind" || cmd == "绑定" || cmd == "b") {
            uiManager.AddLog("bind 命令用法:");
            uiManager.AddLog("  bind windowkey <foreground/binded/main/stackshow> <key>");
            uiManager.AddLog("    - 绑定按键:切换前台窗口、切换绑定窗口、切换主界面、显示栈内隐藏窗口");
            uiManager.AddLog("  bind bindkey <key>");
            uiManager.AddLog("    - 绑定按键:绑定前台窗口为窗口");
            uiManager.AddLog("  bind show");
            uiManager.AddLog("    - 显示所有按键绑定");
            uiManager.AddLog("  bind clear [foreground/binded/main/stackshow/bindkey]");
            uiManager.AddLog("    - 清除按键绑定");
            uiManager.AddLog("示例: bind windowkey foreground ctrl+a");
        } else if (cmd == "bindwindow" || cmd == "绑定窗口" || cmd == "bw") {
            uiManager.AddLog("bindwindow 命令用法:");
            uiManager.AddLog("  bindwindow <sec>");
            uiManager.AddLog("    - 在sec秒后将前台窗口绑定为窗口A");
            uiManager.AddLog("示例: bindwindow 3.5");
        } else if (cmd == "settings" || cmd == "设置" || cmd == "set" || cmd == "s") {
            uiManager.AddLog("settings 命令用法:");
            uiManager.AddLog("  settings <entry> <value>");
            uiManager.AddLog("可用设置项:");
            uiManager.AddLog("  PreventSystematicBind - 是否阻止绑定系统窗体(true/false)");
            uiManager.AddLog("  PreventConsoleBind    - 是否阻止绑定主程序(true/false)");
            uiManager.AddLog("  TrayMinimized         - 是否最小化到托盘图标(true/false)");
            uiManager.AddLog("  ConsoleLine           - 主程序窗口行数(整数)");
            uiManager.AddLog("  ConsoleColumn         - 主程序窗口列数(整数)");
            uiManager.AddLog("  ConsoleColor          - 主程序控制台颜色('9f')");
            uiManager.AddLog("  ExitRestoration       - 是否在退出时复原(true/false)");
            uiManager.AddLog("  StackMode             - 是否启动堆栈窗口模式(true/false)");
            uiManager.AddLog("示例: settings ConsoleColor 0f");
        } else if (cmd == "toggle" || cmd == "t") {
            uiManager.AddLog("toggle 命令用法:");
            uiManager.AddLog("  toggle");
            uiManager.AddLog("    - 切换键盘监听器的打开或关闭状态");
        } else if (cmd == "about" || cmd == "关于" || cmd == "a") {
            uiManager.AddLog("about 命令用法:");
            uiManager.AddLog("  about");
            uiManager.AddLog("    - 显示关于信息");
        } else if (cmd == "info" || cmd == "信息" || cmd == "i") {
            uiManager.AddLog("info 命令用法:");
            uiManager.AddLog("  info");
            uiManager.AddLog("    - 列出所有被隐藏窗口的标题和栈内序号");
        } else {
            uiManager.AddLog("没有找到关于 '" + topic + "' 的帮助信息");
        }
    }
}

void ShowAbout() {
    std::thread aboutThread([]() {
        MessageBoxA(NULL,
                   "窗口隐藏器 高级版\nWindows Showing Engine Pro\n版本: 3.0.0.250921\n\n一款控制窗口显示状态的软件。\n为CYY和雷健康老师设计。\n\nCCat Team. 2022",
                   "关于",
                   MB_OK | MB_ICONINFORMATION);
    });
    aboutThread.detach();
    
    uiManager.AddLog("已显示关于信息");
}

void ShowInfo() {
    windowManager.ShowHiddenWindowsInfo(&uiManager);
}

void ProcessBindCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        uiManager.AddLog("bind 命令需要子命令，输入 'help bind' 查看帮助");
        return;
    }
    
    std::string subCmd = args[0];
    std::transform(subCmd.begin(), subCmd.end(), subCmd.begin(), ::tolower);
    
    if (subCmd == "windowkey") {
        if (args.size() < 3) {
            uiManager.AddLog("windowkey 命令需要两个参数，输入 'help bind' 查看帮助");
            return;
        }
        
        std::string keyType = args[1];
        std::transform(keyType.begin(), keyType.end(), keyType.begin(), ::tolower);
        
        std::string key = args[2];
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        
        std::string settingName;
        if (keyType == "foreground") {
            settingName = "KeyForeground";
        } else if (keyType == "binded") {
            settingName = "KeyBinded";
        } else if (keyType == "main") {
            settingName = "KeyMain";
        } else if (keyType == "stackshow") {
            settingName = "KeyStackShow";
        } else {
            uiManager.AddLog("无效的按键类型: " + keyType);
            return;
        }
        
        settings.setString(settingName, key);
        SaveSettings();
        uiManager.AddLog("已绑定 " + keyType + " 按键为: " + key);
        
    } else if (subCmd == "bindkey") {
        if (args.size() < 2) {
            uiManager.AddLog("bindkey 命令需要一个参数，输入 'help bind' 查看帮助");
            return;
        }
        
        std::string key = args[1];
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        
        settings.setString("KeyBindWindow", key);
        SaveSettings();
        uiManager.AddLog("已绑定绑定窗口按键为: " + key);
        
    } else if (subCmd == "show") {
        uiManager.AddLog("当前按键绑定:");
        uiManager.AddLog("  切换前台窗口: " + settings.getString("KeyForeground", "未设置"));
        uiManager.AddLog("  切换绑定窗口: " + settings.getString("KeyBinded", "未设置"));
        uiManager.AddLog("  切换主界面: " + settings.getString("KeyMain", "未设置"));
        uiManager.AddLog("  绑定窗口: " + settings.getString("KeyBindWindow", "未设置"));
        uiManager.AddLog("  显示栈内隐藏窗口: " + settings.getString("KeyStackShow", "未设置"));
        
    } else if (subCmd == "clear") {
        if (args.size() < 2) {
            uiManager.AddLog("clear 命令需要一个参数，输入 'help bind' 查看帮助");
            return;
        }
        
        std::string keyType = args[1];
        std::transform(keyType.begin(), keyType.end(), keyType.begin(), ::tolower);
        
        std::string settingName;
        std::string displayName;
        
        if (keyType == "foreground") {
            settingName = "KeyForeground";
            displayName = "切换前台窗口";
        } else if (keyType == "binded") {
            settingName = "KeyBinded";
            displayName = "切换绑定窗口";
        } else if (keyType == "main") {
            settingName = "KeyMain";
            displayName = "切换主界面";
        } else if (keyType == "stackshow") {
            settingName = "KeyStackShow";
            displayName = "显示栈内隐藏窗口";
        } else if (keyType == "bindkey") {
            settingName = "KeyBindWindow";
            displayName = "绑定窗口";
        } else {
            uiManager.AddLog("无效的按键类型: " + keyType);
            return;
        }
        
        settings.setString(settingName, "");
        SaveSettings();
        uiManager.AddLog("已清除 " + displayName + " 按键绑定");
        
    } else {
        uiManager.AddLog("未知的 bind 子命令: " + subCmd + "，输入 'help bind' 查看帮助");
    }
}

void ProcessBindWindowCommand(const std::vector<std::string>& args) {
    if (args.size() < 1) {
        uiManager.AddLog("bindwindow 命令需要一个参数，输入 'help bindwindow' 查看帮助");
        return;
    }
    
    if (settings.getBool("StackMode", false)) {
        uiManager.AddLog("堆栈模式已启用, bindwindow 命令在当前模式下不生效");
        return;
    }
    
    try {
        float sec = std::stof(args[0]);
        if (sec <= 0) {
            uiManager.AddLog("时间必须大于0");
            return;
        }
        
        uiManager.AddLog("将在 " + std::to_string(sec) + " 秒后绑定前台窗口");
        
        std::thread bindThread([sec]() {
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(sec * 1000)));
            
            HWND foregroundWindow = GetForegroundWindow();
            if (foregroundWindow) {
                windowManager.BindWindow(foregroundWindow);
                uiManager.AddLog("已绑定前台窗口为窗口");
            } else {
                uiManager.AddLog("无法获取前台窗口");
            }
        });
        bindThread.detach();
        
    } catch (const std::exception& e) {
        uiManager.AddLog("无效的时间参数: " + args[0]);
    }
}

void ProcessSettingsCommand(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        uiManager.AddLog("settings 命令需要两个参数，输入 'help settings' 查看帮助");
        return;
    }
    
    std::string entry = args[0];
    std::transform(entry.begin(), entry.end(), entry.begin(), ::tolower);
    
    std::string value = args[1];
    
    std::map<std::string, std::string> settingMap = {
        {"preventsystematicbind", "PreventSystematicBind"},
        {"preventconsolebind", "PreventConsoleBind"},
        {"trayminimized", "TrayMinimized"},
        {"consoleline", "ConsoleLine"},
        {"consolecolumn", "ConsoleColumn"},
        {"consolecolor", "ConsoleColor"},
        {"exitrestoration", "ExitRestoration"},
        {"stackmode", "StackMode"}
    };
    
    auto it = settingMap.find(entry);
    if (it == settingMap.end()) {
        uiManager.AddLog("未知的设置项: " + entry);
        return;
    }
    
    std::string settingName = it->second;
    
    if (settingName == "PreventSystematicBind" || 
        settingName == "PreventConsoleBind" || 
        settingName == "TrayMinimized" || 
        settingName == "ExitRestoration" || 
        settingName == "StackMode") {
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        bool boolValue = (value == "true" || value == "1" || value == "yes" || value == "是");
        settings.setBool(settingName, boolValue);
        uiManager.AddLog("已设置 " + settingName + " 为: " + (boolValue ? "true" : "false"));
    } else if (settingName == "ConsoleLine" || settingName == "ConsoleColumn") {
        try {
            int intValue = std::stoi(value);
            if (intValue <= 0) {
                uiManager.AddLog(settingName + " 必须大于0");
                return;
            }
            settings.setInt(settingName, intValue);
            uiManager.AddLog("已设置 " + settingName + " 为: " + std::to_string(intValue));
            
            if (settingName == "ConsoleLine" || settingName == "ConsoleColumn") {
                int cols = settings.getInt("ConsoleColumn", 20);
                int lines = settings.getInt("ConsoleLine", 70);
                std::string sizeCmd = "mode con:cols=" + std::to_string(cols) + " lines=" + std::to_string(lines);
                system(sizeCmd.c_str());
            }
        } catch (const std::exception& e) {
            uiManager.AddLog("无效的整数值: " + value);
            return;
        }
    } else if (settingName == "ConsoleColor") {
        settings.setString(settingName, value);
        uiManager.AddLog("已设置 " + settingName + " 为: " + value);
        
        std::string colorCmd = "color " + value;
        system(colorCmd.c_str());
    }
    
    SaveSettings();
}

void ToggleKeyboardHook() {
    if (keyboardHook.IsHooked()) {
        keyboardHook.Unhook();
        uiManager.AddLog("键盘监听器已关闭");
    } else {
        if (keyboardHook.Hook()) {
            uiManager.AddLog("键盘监听器已开启");
        } else {
            uiManager.AddLog("无法开启键盘监听器");
        }
    }
}