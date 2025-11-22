/*
 * 俄罗斯方块游戏 - 基于 One Lone Coder 代码的增强修改版本
 * 
 * 原始代码版权: Copyright OneLoneCoder.com
 * 原始代码来源: https://github.com/OneLoneCoder/Javidx9/blob/master/SimplyCode/OneLoneCoder_Tetris.cpp
 * 原始许可证: GNU General Public License v3.0
 * 
 * 修改者: [gujingqiuyun]
 * 修改日期: [2025-11-22]
 * 
 * 主要修改内容:
 * 1. 添加完整的游戏状态管理系统（菜单、游戏、暂停、结束）
 * 2. 实现彩色俄罗斯方块显示系统
 * 3. 完全中文化用户界面
 * 4. 添加下一个方块预览功能
 * 5. 实现会话最高分记录系统
 * 6. 改进控制方式（方向键控制）
 * 7. 增强用户界面和操作提示
 * 8. 重构代码结构，提高模块化程度
 */




#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <conio.h>
using namespace std;
using namespace std::chrono;

#include <stdio.h>
#include <Windows.h>

// ========== 颜色定义以获得更好的视觉效果 ==========
#include <map>
#include <string>
std::map<int, WORD> pieceColors = {
    {0, FOREGROUND_BLUE | FOREGROUND_INTENSITY},           // I - 青色 
    {1, FOREGROUND_BLUE},                                  // J - 蓝色 
    {2, FOREGROUND_RED | FOREGROUND_GREEN},                // O - 黄色 
    {3, FOREGROUND_RED},                                   // L - 橙色 
    {4, FOREGROUND_GREEN | FOREGROUND_INTENSITY},          // S - 绿色 
    {5, FOREGROUND_RED | FOREGROUND_INTENSITY},            // Z - 红色 
    {6, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY} // T - 紫色 
};

int nScreenWidth = 80;			// 控制台屏幕宽度（列数）
int nScreenHeight = 30;			// 控制台屏幕高度（行数）
wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

// 游戏状态枚举
enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

int Rotate(int px, int py, int r)
{
    int pi = 0;
    switch (r % 4)
    {
    case 0: // 0度			// 0  1  2  3
        pi = py * 4 + px;			// 4  5  6  7
        break;						// 8  9 10 11
        //12 13 14 15

    case 1: // 90度			//12  8  4  0
        pi = 12 + py - (px * 4);	//13  9  5  1
        break;						//14 10  6  2
        //15 11  7  3

    case 2: // 180度			//15 14 13 12
        pi = 15 - (py * 4) - px;	//11 10  9  8
        break;						// 7  6  5  4
        // 3  2  1  0

    case 3: // 270度			// 3  7 11 15
        pi = 3 - py + (px * 4);		// 2  6 10 14
        break;						// 1  5  9 13
    }								// 0  4  8 12

    return pi;
}


// ========== 绘制彩色俄罗斯方块块的函数 ==========
void DrawColoredPiece(int x, int y, int pieceType, wchar_t* screen, HANDLE hConsole) {
    if (pieceType >= 0 && pieceType < 7) {
        screen[y * nScreenWidth + x] = L'█'; // 使用方块字符 
        // 颜色将在绘制整个屏幕时设置 
    }
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
    // 所有场地单元格 >0 的都被占用
    for (int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++)
        {
            // 获取方块的索引
            int pi = Rotate(px, py, nRotation);

            // 获取场地的索引
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            // 检查测试是否在边界内。注意越界不一定意味着失败，
            // 因为长垂直方块可能有位于边界外的单元格，所以我们忽略它们
            if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
            {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
                {
                    // 在边界内，进行碰撞检查
                    if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0)
                        return false; // 第一次碰撞就失败
                }
            }
        }

    return true;
}

// ========== 通用绘制函数，更新了参数 ==========
void DrawScreen(wchar_t* screen, HANDLE hConsole, CHAR_INFO* charBuffer,
    unsigned char* pField, int nCurrentPiece, int nNextPiece,
    int nCurrentX, int nCurrentY,
    std::map<int, WORD>& pieceColors,
    int nScreenWidth, int nScreenHeight, int nFieldWidth, int nFieldHeight) {

    // 填充字符缓冲区
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
        charBuffer[i].Char.UnicodeChar = screen[i];
        charBuffer[i].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // 默认白色

        // 设置颜色
        if (screen[i] == L'█') {
            bool found = false;
            for (int x = 0; x < nFieldWidth && !found; x++) {
                for (int y = 0; y < nFieldHeight && !found; y++) {
                    int screenPos = (y + 2) * nScreenWidth + (x + 2);
                    if (i == screenPos && pField[y * nFieldWidth + x] > 0 && pField[y * nFieldWidth + x] < 8) {
                        charBuffer[i].Attributes = pieceColors[pField[y * nFieldWidth + x] - 1];
                        found = true;
                    }
                }
            }
            // 检查是否是当前方块
            if (!found) {
                for (int px = 0; px < 4 && !found; px++) {
                    for (int py = 0; py < 4 && !found; py++) {
                        int screenPos = (nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2);
                        if (i == screenPos) {
                            charBuffer[i].Attributes = pieceColors[nCurrentPiece];
                            found = true;
                        }
                    }
                }
            }

            // 检查是否是下一个方块预览 - 修复颜色问题
            if (!found) {
                for (int px = 0; px < 4 && !found; px++) {
                    for (int py = 0; py < 4 && !found; py++) {
                        // 修复：使用正确的位置计算
                        int screenPos = (9 + py) * nScreenWidth + (nFieldWidth + 8 + px);
                        if (i == screenPos && tetromino[nNextPiece][Rotate(px, py, 0)] != L'.') {
                            charBuffer[i].Attributes = pieceColors[nNextPiece];
                            found = true;
                        }
                    }
                }
            }
        }
        else if (screen[i] == L'=') {
            charBuffer[i].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        }
        else if (screen[i] == L'#') {
            charBuffer[i].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
    }

    // 一次性输出整个缓冲区
    COORD bufferSize = { (short)nScreenWidth, (short)nScreenHeight };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, (short)nScreenWidth - 1, (short)nScreenHeight - 1 };
    WriteConsoleOutput(hConsole, charBuffer, bufferSize, bufferCoord, &writeRegion);
}

// ========== 开始菜单函数 ==========
void DrawMenu(wchar_t* screen, int nScreenWidth, int nScreenHeight, int nHighScore) {
    // 清屏
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

    // 绘制标题
    wstring title = L"俄罗斯方块";
    int titlePos = (nScreenWidth - (int)title.length()) / 2;
    for (int i = 0; i < title.length(); i++) {
        screen[5 * nScreenWidth + titlePos + i] = title[i];
    }

    // 绘制选项
    wstring startOption = L"按 [ENTER] 开始游戏";
    wstring exitOption = L"按 [ESC] 退出游戏";
    wstring highScoreText = L"当前会话最高分: " + to_wstring(nHighScore);
    wstring controlsText = L"游戏中: [P]暂停  [M]返回菜单";

    int optionPos = (nScreenWidth - (int)startOption.length()) / 2;
    for (int i = 0; i < startOption.length(); i++) {
        screen[10 * nScreenWidth + optionPos + i] = startOption[i];
    }

    for (int i = 0; i < exitOption.length(); i++) {
        screen[12 * nScreenWidth + optionPos + i] = exitOption[i];
    }

    for (int i = 0; i < highScoreText.length(); i++) {
        screen[15 * nScreenWidth + optionPos + i] = highScoreText[i];
    }

    for (int i = 0; i < controlsText.length(); i++) {
        screen[18 * nScreenWidth + optionPos + i] = controlsText[i];
    }
}

// ========== 简单的暂停消息 ==========
void DrawPauseMessage(wchar_t* screen, int nScreenWidth, int nScreenHeight) {
    // 只在屏幕中间显示一行简单的暂停信息
    wstring pauseText = L"游戏暂停 - 按 C 继续游戏";
    int centerX = (nScreenWidth - (int)pauseText.length()) / 2;
    int centerY = nScreenHeight / 2;

    for (int i = 0; i < pauseText.length(); i++) {
        screen[centerY * nScreenWidth + centerX + i] = pauseText[i];
    }
}

// ========== 清除暂停消息函数 ==========
void ClearPauseMessage(wchar_t* screen, int nScreenWidth, int nScreenHeight) {
    // 清除暂停信息
    wstring pauseText = L"游戏暂停 - 按 C 继续游戏";
    int centerX = (nScreenWidth - (int)pauseText.length()) / 2;
    int centerY = nScreenHeight / 2;

    for (int i = 0; i < pauseText.length(); i++) {
        screen[centerY * nScreenWidth + centerX + i] = L' ';
    }
}

// ========== 清屏函数 ==========
void ClearScreen(wchar_t* screen, int nScreenWidth, int nScreenHeight) {
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
        screen[i] = L' ';
    }
}

// ========== 游戏结束界面函数 ==========
void DrawGameOverScreen(wchar_t* screen, int nScreenWidth, int nScreenHeight, int nScore, int nHighScore) {
    // 清屏
    ClearScreen(screen, nScreenWidth, nScreenHeight);

    // 使用宽字符字符串
    wstring gameOverText = L"=== 游戏结束 ===";
    wstring scoreText = L"本次得分: " + to_wstring(nScore);
    wstring highScoreText = L"会话最高分: " + to_wstring(nHighScore);
    wstring restartText = L"按 [ENTER] 重新开始";
    wstring menuText = L"按 [ESC] 返回主菜单";

    int centerX = (nScreenWidth - 20) / 2;

    // 绘制游戏结束信息 - 使用宽字符
    for (int i = 0; i < gameOverText.length(); i++) {
        screen[8 * nScreenWidth + centerX + i] = gameOverText[i];
    }
    for (int i = 0; i < scoreText.length(); i++) {
        screen[10 * nScreenWidth + centerX + i] = scoreText[i];
    }
    for (int i = 0; i < highScoreText.length(); i++) {
        screen[12 * nScreenWidth + centerX + i] = highScoreText[i];
    }
    for (int i = 0; i < restartText.length(); i++) {
        screen[14 * nScreenWidth + centerX + i] = restartText[i];
    }
    for (int i = 0; i < menuText.length(); i++) {
        screen[16 * nScreenWidth + centerX + i] = menuText[i];
    }
}

// ========== 最高分现在仅基于会话 ==========
int nSessionHighScore = 0; // 当前会话最高分，程序退出后清零

int main()
{
    // 自动设置控制台窗口
    system("mode con cols=80 lines=30");
    system("title 俄罗斯方块");

    // 设置控制台字体
    system("chcp 65001 > nul"); // 设置UTF-8编码
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(fontInfo);
    GetCurrentConsoleFontEx(hStdOut, FALSE, &fontInfo);
    fontInfo.dwFontSize.X = 8;   // 字体宽度
    fontInfo.dwFontSize.Y = 16;  // 字体高度
    wcscpy_s(fontInfo.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hStdOut, FALSE, &fontInfo);

    // 初始化随机数种子
    srand(static_cast<unsigned int>(time(nullptr)));

    // ========== 游戏状态变量 ==========
    GameState gameState = MENU;      // 初始状态为菜单
    bool bRestartRequested = false;  // 重新开始请求标志

    // 创建屏幕缓冲区 
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    SetConsoleTitle(L"俄罗斯方块"); // 设置窗口标题

    CHAR_INFO* charBuffer = new CHAR_INFO[nScreenWidth * nScreenHeight];
    DWORD dwBytesWritten = 0;

    // Tetronimos 4x4
    tetromino[0].append(L"..█...█...█...█."); // I 方块 
    tetromino[1].append(L"..█..██...█....."); // J 方块 
    tetromino[2].append(L".....██..██....."); // O 方块
    tetromino[3].append(L"..█..██..█......"); // L 方块 
    tetromino[4].append(L".█...██...█....."); // S 方块
    tetromino[5].append(L".█...█...██....."); // Z 方块 
    tetromino[6].append(L"..█...█..██....."); // T 方块 

    pField = new unsigned char[nFieldWidth * nFieldHeight]; // 创建游戏场地缓冲区
    for (int x = 0; x < nFieldWidth; x++) // 场地边界
        for (int y = 0; y < nFieldHeight; y++)
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

    // 游戏逻辑
    bool bKey[4];
    int nCurrentPiece = rand() % 7; // 使用随机数
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;
    int nSpeed = 8;
    int nSpeedCount = 0;
    bool bForceDown = false;
    bool bRotateHold = true;
    int nPieceCount = 0;
    int nScore = 0;
    vector<int> vLines;
    bool bGameOver = false;

    // 添加下一个方块预览
    int nNextPiece = rand() % 7;

    // ========== 带有状态管理的主游戏循环 ==========
    bool bExitGame = false;

    while (!bExitGame) {
        // 处理不同游戏状态
        switch (gameState) {
        case MENU:
        {
            // 菜单状态逻辑
            DrawMenu(screen, nScreenWidth, nScreenHeight, nSessionHighScore);
            DrawScreen(screen, hConsole, charBuffer, pField, nCurrentPiece, nNextPiece,
                nCurrentX, nCurrentY, pieceColors,
                nScreenWidth, nScreenHeight, nFieldWidth, nFieldHeight);

            // 菜单输入处理
            if ((GetAsyncKeyState(VK_RETURN) & 0x8000) != 0) {
                gameState = PLAYING;
                // 清屏，确保开始游戏后界面干净
                ClearScreen(screen, nScreenWidth, nScreenHeight);
                // 重置游戏状态
                bGameOver = false;
                nScore = 0;
                nSpeed = 8;
                nPieceCount = 0;
                nSpeedCount = 0;

                // 清空场地
                for (int x = 0; x < nFieldWidth; x++)
                    for (int y = 0; y < nFieldHeight; y++)
                        pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

                // 重置当前方块
                nCurrentPiece = rand() % 7;
                nCurrentRotation = 0;
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nNextPiece = rand() % 7;
                vLines.clear();

                this_thread::sleep_for(milliseconds(200)); // 防误触
            }
            if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
                bExitGame = true; // 退出游戏
            }
            break;
        }

        case PLAYING:
        {
            // 原有的游戏逻辑
            this_thread::sleep_for(milliseconds(50));
            nSpeedCount++;
            bForceDown = (nSpeedCount == nSpeed);

            // 输入处理
            bKey[0] = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
            bKey[1] = (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0;
            bKey[2] = (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0;
            bKey[3] = (GetAsyncKeyState(VK_UP) & 0x8000) != 0;

            // 暂停键
            if ((GetAsyncKeyState('P') & 0x8000) != 0) {
                gameState = PAUSED;
                this_thread::sleep_for(milliseconds(200));
            }

            // 返回主菜单键
            if ((GetAsyncKeyState('M') & 0x8000) != 0) {
                gameState = MENU;
                this_thread::sleep_for(milliseconds(200));
            }

            // 原有的游戏移动逻辑
            nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
            nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
            nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

            // 旋转逻辑
            if (bKey[3]) {
                if (bRotateHold && DoesPieceFit(nCurrentPiece, (nCurrentRotation + 1) % 4, nCurrentX, nCurrentY)) {
                    nCurrentRotation = (nCurrentRotation + 1) % 4;
                }
                bRotateHold = false;
            }
            else {
                bRotateHold = true;
            }

            // 强制下落逻辑
            if (bForceDown) {
                nSpeedCount = 0;
                nPieceCount++;
                if (nPieceCount % 50 == 0)
                    if (nSpeed >= 10) nSpeed--;

                if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
                    nCurrentY++;
                }
                else {
                    // 锁定方块
                    for (int px = 0; px < 4; px++)
                        for (int py = 0; py < 4; py++)
                            if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
                                pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

                    // 检查消除行
                    for (int py = 0; py < 4; py++)
                        if (nCurrentY + py < nFieldHeight - 1) {
                            bool bLine = true;
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

                            if (bLine) {
                                for (int px = 1; px < nFieldWidth - 1; px++)
                                    pField[(nCurrentY + py) * nFieldWidth + px] = 8;
                                vLines.push_back(nCurrentY + py);
                            }
                        }

                    nScore += 25;
                    if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

                    // 生成新方块
                    nCurrentX = nFieldWidth / 2;
                    nCurrentY = 0;
                    nCurrentRotation = 0;
                    nCurrentPiece = nNextPiece;
                    nNextPiece = rand() % 7;

                    // 检查游戏结束
                    bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
                }
            }

            // 显示逻辑 - 每次游戏循环都重新绘制整个游戏界面
            // 用颜色绘制场地 
            for (int x = 0; x < nFieldWidth; x++) {
                for (int y = 0; y < nFieldHeight; y++) {
                    int fieldValue = pField[y * nFieldWidth + x];
                    wchar_t displayChar = L' ';

                    if (fieldValue == 0) {
                        displayChar = L' ';
                    }
                    else if (fieldValue == 8) {
                        displayChar = L'=';
                    }
                    else if (fieldValue == 9) {
                        displayChar = L'#';
                    }
                    else {
                        displayChar = L'█';
                    }

                    int screenIndex = (y + 2) * nScreenWidth + (x + 2);
                    if (screenIndex >= 0 && screenIndex < nScreenWidth * nScreenHeight) {
                        screen[screenIndex] = displayChar;
                    }
                }
            }

            // 用颜色绘制当前方块
            for (int px = 0; px < 4; px++) {
                for (int py = 0; py < 4; py++) {
                    if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.') {
                        int screenIndex = (nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2);
                        if (screenIndex >= 0 && screenIndex < nScreenWidth * nScreenHeight) {
                            screen[screenIndex] = L'█';
                        }
                    }
                }
            }

            // 绘制分数 
            wchar_t scoreBuffer[32];
            swprintf_s(scoreBuffer, 32, L"得分: %8d", nScore);
            for (int i = 0; i < 32 && scoreBuffer[i] != L'\0'; i++) {
                int pos = 2 * nScreenWidth + nFieldWidth + 6 + i;
                if (pos < nScreenWidth * nScreenHeight) {
                    screen[pos] = scoreBuffer[i];
                }
            }

            // 显示操作提示
            wchar_t controlsBuffer[] = L"操作: [P]暂停 [M]菜单";
            for (int i = 0; i < 22 && controlsBuffer[i] != L'\0'; i++) {
                int pos = 4 * nScreenWidth + nFieldWidth + 6 + i;
                if (pos < nScreenWidth * nScreenHeight) {
                    screen[pos] = controlsBuffer[i];
                }
            }

            // 绘制下一个方块预览 
            const wchar_t* nextText = L"下一个:";
            for (int i = 0; i < 6; i++) {
                int pos = 6 * nScreenWidth + nFieldWidth + 6 + i;
                if (pos < nScreenWidth * nScreenHeight) {
                    screen[pos] = nextText[i];
                }
            }
            // 清空下一个方块预览区域
            for (int px = 0; px < 6; px++) {
                for (int py = 0; py < 6; py++) {
                    int pos = (7 + py) * nScreenWidth + (nFieldWidth + 6 + px);
                    if (pos < nScreenWidth * nScreenHeight) {
                        screen[pos] = L' ';
                    }
                }
            }

            // 绘制下一个方块
            for (int px = 0; px < 4; px++) {
                for (int py = 0; py < 4; py++) {
                    if (tetromino[nNextPiece][Rotate(px, py, 0)] != L'.') {
                        int pos = (9 + py) * nScreenWidth + (nFieldWidth + 8 + px);
                        if (pos < nScreenWidth * nScreenHeight) {
                            screen[pos] = L'█';
                        }
                    }
                }
            }

            // 消除行动画
            if (!vLines.empty()) {
                DrawScreen(screen, hConsole, charBuffer, pField, nCurrentPiece, nNextPiece,
                    nCurrentX, nCurrentY, pieceColors,
                    nScreenWidth, nScreenHeight, nFieldWidth, nFieldHeight);
                this_thread::sleep_for(milliseconds(400));

                for (auto& v : vLines)
                    for (int px = 1; px < nFieldWidth - 1; px++) {
                        for (int py = v; py > 0; py--)
                            pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                        pField[0 * nFieldWidth + px] = 0;
                    }

                vLines.clear();
            }

            DrawScreen(screen, hConsole, charBuffer, pField, nCurrentPiece, nNextPiece,
                nCurrentX, nCurrentY, pieceColors,
                nScreenWidth, nScreenHeight, nFieldWidth, nFieldHeight);

            // 检查游戏结束
            if (bGameOver) {
                // 更新会话最高分
                if (nScore > nSessionHighScore) {
                    nSessionHighScore = nScore;
                }
                gameState = GAME_OVER;
            }
            break;
        }

        case PAUSED:
        {
            // 只在游戏画面上添加一行暂停提示
            DrawPauseMessage(screen, nScreenWidth, nScreenHeight);
            DrawScreen(screen, hConsole, charBuffer, pField, nCurrentPiece, nNextPiece,
                nCurrentX, nCurrentY, pieceColors,
                nScreenWidth, nScreenHeight, nFieldWidth, nFieldHeight);

            // 暂停菜单输入处理 - 按C继续
            if ((GetAsyncKeyState('C') & 0x8000) != 0) {
                // 清除暂停文字
                ClearPauseMessage(screen, nScreenWidth, nScreenHeight);
                gameState = PLAYING;
                this_thread::sleep_for(milliseconds(200));
            }
            break;
        }

        case GAME_OVER:
        {
            // 使用专门的游戏结束界面绘制函数
            DrawGameOverScreen(screen, nScreenWidth, nScreenHeight, nScore, nSessionHighScore);
            DrawScreen(screen, hConsole, charBuffer, pField, nCurrentPiece, nNextPiece,
                nCurrentX, nCurrentY, pieceColors,
                nScreenWidth, nScreenHeight, nFieldWidth, nFieldHeight);

            // 游戏结束输入处理
            if ((GetAsyncKeyState(VK_RETURN) & 0x8000) != 0) {
                // 重新开始游戏
                bRestartRequested = true;
                this_thread::sleep_for(milliseconds(200)); // 添加防误触
            }
            if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
                gameState = MENU;
                this_thread::sleep_for(milliseconds(200));
            }

            if (bRestartRequested) {
                // 重置游戏状态
                bRestartRequested = false;
                bGameOver = false;
                nScore = 0;
                nSpeed = 8;
                nPieceCount = 0;
                nSpeedCount = 0;

                // 清空场地
                for (int x = 0; x < nFieldWidth; x++)
                    for (int y = 0; y < nFieldHeight; y++)
                        pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

                // 重置当前方块
                nCurrentPiece = rand() % 7;
                nCurrentRotation = 0;
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nNextPiece = rand() % 7;
                vLines.clear();

                gameState = PLAYING;
            }
            break;
        }
        }

        this_thread::sleep_for(milliseconds(50));
    }

    // 游戏结束
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    CloseHandle(hConsole);
    SetConsoleActiveScreenBuffer(hStdOut);
    system("cls");
    system("cls");
    cout << "Game is over!! Score:" << nScore << " Session High Score:" << nSessionHighScore << endl;


    // 释放内存
    delete[] screen;
    delete[] pField;
    delete[] charBuffer;

    system("pause");
    return 0;
}