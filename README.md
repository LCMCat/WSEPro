# Window Show/Hide Engine (WSEPro) [v3.0]

![Windows Version](https://img.shields.io/badge/Windows-10%2B-blue) 
![License](https://img.shields.io/badge/License-MIT-green)

## ✨ Features
- Real-time keyboard hooking technology
- Stack-based window management
- Cross-process window manipulation
- INI-based configuration system

## 🛠️ Build Instructions
### Requirements
- MinGW-w64 with POSIX threads
- Windows SDK 10.0+

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