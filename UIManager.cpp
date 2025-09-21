#include "UIManager.h"
#include "Settings.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <conio.h>

UIManager::UIManager() : settings(nullptr), consoleOutput(nullptr), consoleInput(nullptr),
                        maxLogLines(100), inputLine(20), isVisible(true), commandReady(false) {
}

UIManager::~UIManager() {
}

void UIManager::Initialize(Settings* settings) {
    this->settings = settings;
    consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    consoleInput = GetStdHandle(STD_INPUT_HANDLE);
    maxLogLines = 100;
    isVisible = true;
    
    InitializeConsole();
    
    SetConsoleTitleA("WSE Pro");
    
    int cols = settings->getInt("ConsoleColumn", 70);
    int lines = settings->getInt("ConsoleLine", 20);
    
    std::string cmd = "mode con:cols=" + std::to_string(cols) + " lines=" + std::to_string(lines);
    system(cmd.c_str());
    
    std::string color = settings->getString("ConsoleColor", "9f");
    cmd = "color " + color;
    system(cmd.c_str());
    
    GetConsoleScreenBufferInfo(consoleOutput, &csbi);
    inputLine = csbi.srWindow.Bottom - csbi.srWindow.Top;
}

void UIManager::InitializeConsole() {
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow) {
        HMENU hMenu = GetSystemMenu(consoleWindow, FALSE);
        if (hMenu) {
            DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
            DeleteMenu(hMenu, SC_SIZE, MF_BYCOMMAND);
            DrawMenuBar(consoleWindow);
        }
        
        LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        SetWindowLong(consoleWindow, GWL_STYLE, style);
        
        SetWindowPos(consoleWindow, NULL, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    
    GetConsoleScreenBufferInfo(consoleOutput, &csbi);
}

void UIManager::HandleConsoleEvent(INPUT_RECORD& ir) {
    switch (ir.EventType) {
        case KEY_EVENT: {
            KEY_EVENT_RECORD keyEvent = ir.Event.KeyEvent;
            
            if (keyEvent.bKeyDown) {
                if (keyEvent.wVirtualKeyCode == VK_RETURN) {
                    if (!inputBuffer.empty()) {
                        commandReady = true;
                        
                        savedCommand = inputBuffer;
                        
                        COORD cursorPos;
                        DWORD charsWritten;
                        GetConsoleScreenBufferInfo(consoleOutput, &csbi);
                        
                        cursorPos.X = 5;
                        cursorPos.Y = inputLine;
                        SetConsoleCursorPosition(consoleOutput, cursorPos);
                        
                        int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
                        int clearLength = consoleWidth - 5;
                        
                        std::string spaces(clearLength, ' ');
                        WriteConsoleA(consoleOutput, spaces.c_str(), spaces.length(), &charsWritten, NULL);
                        
                        cursorPos.X = 5;
                        SetConsoleCursorPosition(consoleOutput, cursorPos);
                        
                        inputBuffer.clear();
                    }
                }
                else if (keyEvent.wVirtualKeyCode == VK_BACK) {
                    if (!inputBuffer.empty()) {
                        bool isChinese = false;
                        if (inputBuffer.length() >= 3) {
                            unsigned char lastByte = static_cast<unsigned char>(inputBuffer.back());
                            unsigned char secondLastByte = static_cast<unsigned char>(inputBuffer[inputBuffer.length() - 2]);
                            unsigned char thirdLastByte = static_cast<unsigned char>(inputBuffer[inputBuffer.length() - 3]);
                            
                            if ((thirdLastByte & 0xE0) == 0xE0 && 
                                (secondLastByte & 0xC0) == 0x80 && 
                                (lastByte & 0xC0) == 0x80) {
                                isChinese = true;
                                inputBuffer.pop_back();
                                inputBuffer.pop_back();
                                inputBuffer.pop_back();
                            }
                        }
                        
                        if (!isChinese) {
                            inputBuffer.pop_back();
                        }
                        
                        COORD cursorPos;
                        DWORD charsWritten;
                        GetConsoleScreenBufferInfo(consoleOutput, &csbi);
                        
                        cursorPos.X = 5;
                        cursorPos.Y = inputLine;
                        SetConsoleCursorPosition(consoleOutput, cursorPos);
                        
                        int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
                        int clearLength = consoleWidth - 5;
                        
                        std::string spaces(clearLength, ' ');
                        WriteConsoleA(consoleOutput, spaces.c_str(), spaces.length(), &charsWritten, NULL);
                        
                        cursorPos.X = 5;
                        SetConsoleCursorPosition(consoleOutput, cursorPos);
                        
                        if (!inputBuffer.empty()) {
                            WriteConsoleA(consoleOutput, inputBuffer.c_str(), inputBuffer.length(), &charsWritten, NULL);
                            
                            cursorPos.X = 5 + inputBuffer.length();
                            SetConsoleCursorPosition(consoleOutput, cursorPos);
                        }
                    }
                }
                else if (keyEvent.uChar.AsciiChar >= 32 && keyEvent.uChar.AsciiChar <= 126) {
                    inputBuffer += keyEvent.uChar.AsciiChar;
                    UpdateInputDisplay();
                }
                else if (keyEvent.uChar.UnicodeChar >= 0x4E00 && keyEvent.uChar.UnicodeChar <= 0x9FFF) {
                    wchar_t wc = keyEvent.uChar.UnicodeChar;
                    char utf8[4] = {0};
                    
                    if (wc <= 0x7F) {
                        utf8[0] = (char)wc;
                    } else if (wc <= 0x7FF) {
                        utf8[0] = 0xC0 | ((wc >> 6) & 0x1F);
                        utf8[1] = 0x80 | (wc & 0x3F);
                    } else if (wc <= 0xFFFF) {
                        utf8[0] = 0xE0 | ((wc >> 12) & 0x0F);
                        utf8[1] = 0x80 | ((wc >> 6) & 0x3F);
                        utf8[2] = 0x80 | (wc & 0x3F);
                    }
                    
                    inputBuffer += utf8;
                    UpdateInputDisplay();
                }
            }
            break;
        }
    }
}

void UIManager::AddLog(const std::string& log) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    std::stringstream ss;
    ss << "[" << st.wHour << ":" << st.wMinute << ":" << st.wSecond << "]" << log;
    
    logQueue.push(ss.str());
    
    while (logQueue.size() > static_cast<size_t>(maxLogLines)) {
        logQueue.pop();
    }
    
    Refresh();
}

std::string UIManager::GetInput() {
    if (!savedCommand.empty()) {
        std::string cmd = savedCommand;
        savedCommand.clear();
        return cmd;
    }
    return inputBuffer;
}

void UIManager::Refresh() {
    Clear();
    
    GetConsoleScreenBufferInfo(consoleOutput, &csbi);
    inputLine = csbi.srWindow.Bottom - csbi.srWindow.Top;
    
    COORD cursorPos;
    DWORD charsWritten;
    
    cursorPos.X = 0;
    cursorPos.Y = 0;
    SetConsoleCursorPosition(consoleOutput, cursorPos);
    
    std::string title = "窗口隐藏器 高级版";
    WriteConsoleA(consoleOutput, title.c_str(), title.length(), &charsWritten, NULL);
    
    cursorPos.X = 0;
    cursorPos.Y = 1;
    SetConsoleCursorPosition(consoleOutput, cursorPos);
    
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    
    std::string separator(consoleWidth, '-');
    WriteConsoleA(consoleOutput, separator.c_str(), separator.length(), &charsWritten, NULL);
    
    std::queue<std::string> tempQueue = logQueue;
    int line = 2;
    
    int availableLines = inputLine - line;
    int totalLogLines = tempQueue.size();
    int skipLines = (totalLogLines > availableLines) ? (totalLogLines - availableLines) : 0;
    
    for (int i = 0; i < skipLines && !tempQueue.empty(); i++) {
        tempQueue.pop();
    }
    
    while (!tempQueue.empty() && line < inputLine) {
        cursorPos.X = 0;
        cursorPos.Y = line;
        SetConsoleCursorPosition(consoleOutput, cursorPos);
        
        std::string logLine = tempQueue.front();
        tempQueue.pop();
        
        if (logLine.length() > static_cast<size_t>(consoleWidth)) {
            const int timestampLength = 15;
            
            std::string firstLine = logLine.substr(0, consoleWidth);
            WriteConsoleA(consoleOutput, firstLine.c_str(), firstLine.length(), &charsWritten, NULL);
            line++;
            
            size_t startPos = consoleWidth;
            while (startPos < logLine.length() && line < inputLine) {
                cursorPos.X = 0;
                cursorPos.Y = line;
                SetConsoleCursorPosition(consoleOutput, cursorPos);
                
                std::string indent(timestampLength, ' ');
                WriteConsoleA(consoleOutput, indent.c_str(), indent.length(), &charsWritten, NULL);
                
                size_t charsToShow = std::min(static_cast<size_t>(consoleWidth - timestampLength), 
                                             logLine.length() - startPos);
                
                std::string lineContent = logLine.substr(startPos, charsToShow);
                WriteConsoleA(consoleOutput, lineContent.c_str(), lineContent.length(), &charsWritten, NULL);
                
                startPos += charsToShow;
                line++;
            }
        } else {
            WriteConsoleA(consoleOutput, logLine.c_str(), logLine.length(), &charsWritten, NULL);
            line++;
        }
    }
    
    cursorPos.X = 0;
    cursorPos.Y = inputLine;
    SetConsoleCursorPosition(consoleOutput, cursorPos);
    
    std::string prompt = "WSE> ";
    WriteConsoleA(consoleOutput, prompt.c_str(), prompt.length(), &charsWritten, NULL);
    
    if (!inputBuffer.empty()) {
        int maxInputLength = consoleWidth - prompt.length();
        std::string displayInput = inputBuffer;
        
        if (displayInput.length() > static_cast<size_t>(maxInputLength)) {
            displayInput = displayInput.substr(displayInput.length() - maxInputLength);
        }
        
        WriteConsoleA(consoleOutput, displayInput.c_str(), displayInput.length(), &charsWritten, NULL);
        
        cursorPos.X = prompt.length() + displayInput.length();
        SetConsoleCursorPosition(consoleOutput, cursorPos);
    }
}

void UIManager::Clear() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = { 0, 0 };
    
    if (consoleOutput == INVALID_HANDLE_VALUE) return;
    
    GetConsoleScreenBufferInfo(consoleOutput, &csbi);
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;
    
    if (!FillConsoleOutputCharacter(consoleOutput, ' ', cellCount, homeCoords, &count)) return;
    
    if (!FillConsoleOutputAttribute(consoleOutput, csbi.wAttributes, cellCount, homeCoords, &count)) return;
    
    SetConsoleCursorPosition(consoleOutput, homeCoords);
}

void UIManager::SetVisible(bool visible) {
    isVisible = visible;
    
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow) {
        if (visible) {
            ShowWindow(consoleWindow, SW_SHOW);
            SetForegroundWindow(consoleWindow);
        } else {
            ShowWindow(consoleWindow, SW_HIDE);
        }
    }
}

bool UIManager::IsVisible() const {
    return isVisible;
}

void UIManager::ToggleVisibility() {
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow) {
        if (IsWindowVisible(consoleWindow)) {
            ShowWindow(consoleWindow, SW_HIDE);
            isVisible = false;
        } else {
            ShowWindow(consoleWindow, SW_SHOW);
            SetForegroundWindow(consoleWindow);
            isVisible = true;
        }
    }
}

HANDLE UIManager::GetConsoleOutput() const {
    return consoleOutput;
}

HANDLE UIManager::GetConsoleInput() const {
    return consoleInput;
}

const CONSOLE_SCREEN_BUFFER_INFO& UIManager::GetConsoleInfo() const {
    return csbi;
}

const std::queue<std::string>& UIManager::GetLogQueue() const {
    return logQueue;
}

const std::string& UIManager::GetInputBuffer() const {
    return inputBuffer;
}

void UIManager::SetInputBuffer(const std::string& buffer) {
    inputBuffer = buffer;
    Refresh();
}

int UIManager::GetMaxLogLines() const {
    return maxLogLines;
}

void UIManager::SetMaxLogLines(int lines) {
    maxLogLines = lines;
    
    while (logQueue.size() > static_cast<size_t>(maxLogLines)) {
        logQueue.pop();
    }
    
    Refresh();
}

int UIManager::GetInputLine() const {
    return inputLine;
}

void UIManager::SetInputLine(int line) {
    inputLine = line;
    Refresh();
}

void UIManager::UpdateInputDisplay() {
    GetConsoleScreenBufferInfo(consoleOutput, &csbi);
    
    COORD cursorPos;
    DWORD charsWritten;
    
    cursorPos.X = 0;
    cursorPos.Y = inputLine;
    SetConsoleCursorPosition(consoleOutput, cursorPos);
    
    std::string prompt = "WSE> ";
    WriteConsoleA(consoleOutput, prompt.c_str(), prompt.length(), &charsWritten, NULL);
    
    if (!inputBuffer.empty()) {
        int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int maxInputLength = consoleWidth - prompt.length();
        std::string displayInput = inputBuffer;
        
        if (displayInput.length() > static_cast<size_t>(maxInputLength)) {
            displayInput = displayInput.substr(displayInput.length() - maxInputLength);
        }
        
        WriteConsoleA(consoleOutput, displayInput.c_str(), displayInput.length(), &charsWritten, NULL);
        
        cursorPos.X = prompt.length() + displayInput.length();
        SetConsoleCursorPosition(consoleOutput, cursorPos);
    }
}

bool UIManager::IsCommandReady() const {
    return commandReady;
}

void UIManager::ResetCommandReady() {
    commandReady = false;
    inputBuffer.clear();
}