@echo off
chcp 65001
echo 正在编译窗口隐藏器 Pro...
echo.

g++ -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=1 -static-libgcc -static-libstdc++ -finput-charset=UTF-8 -fexec-charset=GBK main.cpp Settings.cpp WindowManager.cpp KeyboardHook.cpp UIManager.cpp -o WSEPro.exe -luser32 -lgdi32 -lkernel32 -lshell32 -lpsapi 

if %ERRORLEVEL% EQU 0 (
    echo.
    echo 编译成功！
    echo.
    echo 程序已生成: WSEPro.exe
    echo.
    echo 运行程序:
    echo   WSEPro.exe
) else (
    echo.
    echo 编译失败！请检查错误信息。
)

pause