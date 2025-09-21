# Window Showing Engine

![Windows Version](https://img.shields.io/badge/Windows-7%2B-blue) 
![License](https://img.shields.io/badge/License-MIT-green)

##The best gift for myself in 2022.

## ✨ Features
- Real-time keyboard hooking technology
- Stack-based window management
- Cross-process window manipulation
- INI-based configuration system

## 🛠️ Build Instructions
### Requirements
- MinGW-w64 with POSIX threads
- Windows 7+

```bash
git clone https://github.com/LCMCat/WSEPro.git
cd WSEPro

# Debug build
g++ -std=c++11 -g -DDEBUG src/*.cpp -Iinclude -o bin/WSEPro.exe \
    -luser32 -lgdi32 -lkernel32 -lshell32 -lpsapi
```

## 📜 License
MIT License
Copyright (c) 2022 CCat Team
