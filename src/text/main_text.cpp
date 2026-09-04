#include "text/TextGame.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <locale.h>
#endif

// 初始化控制台编码：Windows 控制台默认 GBK(CP936)，中文 UTF-8 输出会乱码，
// 这里把输入/输出代码页切到 UTF-8，并以二进制模式读写避免 CRT 二次转码；
// 同时开启 VT 转义序列支持（ENABLE_VIRTUAL_TERMINAL_PROCESSING），使 ANSI 彩色文本生效。
static void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
    _setmode(_fileno(stdin), _O_BINARY);
    setlocale(LC_ALL, ".UTF-8");
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
#else
    // Linux / macOS 终端默认 UTF-8，无需处理
#endif
}

// 纯文字版 MUD 入口：不使用二维地图/方向键，改用命令式文字输入。
int main(int argc, char** argv) {
    setupConsole();
    unsigned seed = (argc > 1) ? static_cast<unsigned>(std::atoi(argv[1])) : static_cast<unsigned>(time(nullptr));
    srand(seed);
    std::cout << "===== 地牢探险（文字版）=====\n";
    std::cout << "输入「帮助」查看指令。\n\n";
    dq::TextGame game;
    game.run();
    return 0;
}
