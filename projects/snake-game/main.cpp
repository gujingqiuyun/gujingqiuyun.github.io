/*
 * 贪吃蛇游戏 - 基于 One Lone Coder 代码的修改版本
 * 
 * 原始代码版权: Copyright OneLoneCoder.com
 * 原始代码来源: hhttps://github.com/OneLoneCoder/Javidx9/blob/master/SimplyCode/OneLoneCoder_Snake.cpp
 * 原始许可证: GNU General Public License v3.0
 * 
 * 修改者: [gujingqiuyun]
 * 修改内容:
 * - 改进了控制方式（方向键控制）
 * - 添加了防反向移动逻辑
 * - 改进了边框绘制
 * - 添加了食物位置边界检查
 * - 更新了显示信息
 */


#include <iostream>
#include <list>
#include <thread>
using namespace std;

#include <chrono>
using namespace std::chrono_literals;

#include <Windows.h>

int nScreenWidth = 120;
int nScreenHeight = 30;

struct sSnakeSegment
{
	int x;
	int y;
};

int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	SetConsoleTitle(L"贪吃蛇");
	DWORD dwBytesWritten = 0;

	while (1)
	{
		// Reset to known state
		list<sSnakeSegment> snake = { {60,15},{61,15},{62,15},{63,15},{64,15},{65,15},{66,15},{67,15},{68,15},{69,15} };
		int nFoodX = 30;
		int nFoodY = 15;
		int nScore = 0;
		int nSnakeDirection = 3;
		bool bDead = false;
		bool bKeyLeft = false, bKeyRight = false, bKeyUp = false, bKeyDown = false;
		bool bKeyLeftOld = false, bKeyRightOld = false, bKeyUpOld = false, bKeyDownOld = false;

		while (!bDead)
		{
			// Frame Timing, compensate for aspect ratio of command line
			auto t1 = chrono::system_clock::now();
			while ((chrono::system_clock::now() - t1) < ((nSnakeDirection % 2 == 1) ? 120ms : 200ms))
			{
				// Get Input, 
				bKeyUp = (0x8000 & GetAsyncKeyState(VK_UP)) != 0;
				bKeyDown = (0x8000 & GetAsyncKeyState(VK_DOWN)) != 0;
				bKeyLeft = (0x8000 & GetAsyncKeyState(VK_LEFT)) != 0;
				bKeyRight = (0x8000 & GetAsyncKeyState(VK_RIGHT)) != 0;

				
				if (bKeyUp && !bKeyUpOld && nSnakeDirection != 2) 
				{
					nSnakeDirection = 0;
				}
				
				if (bKeyDown && !bKeyDownOld && nSnakeDirection != 0) 
				{
					nSnakeDirection = 2;
				}
				
				if (bKeyLeft && !bKeyLeftOld && nSnakeDirection != 1) 
				{
					nSnakeDirection = 3;
				}

				if (bKeyRight && !bKeyRightOld && nSnakeDirection != 3) 
				{
					nSnakeDirection = 1;
				}

				bKeyUpOld = bKeyUp;
				bKeyDownOld = bKeyDown;
				bKeyLeftOld = bKeyLeft;
				bKeyRightOld = bKeyRight;
			}

			// ==== Logic

			// Update Snake Position, place a new head at the front of the list in
			// the right direction
			switch (nSnakeDirection)
			{
			case 0: // UP
				snake.push_front({ snake.front().x, snake.front().y - 1 });
				break;
			case 1: // RIGHT
				snake.push_front({ snake.front().x + 1, snake.front().y });
				break;
			case 2: // DOWN
				snake.push_front({ snake.front().x, snake.front().y + 1 });
				break;
			case 3: // LEFT
				snake.push_front({ snake.front().x - 1, snake.front().y });
				break;
			}

			// Collision Detect Snake V Food
			if (snake.front().x == nFoodX && snake.front().y == nFoodY)
			{
				nScore++;
				while (screen[nFoodY * nScreenWidth + nFoodX] != L' ')
				{
					nFoodX = rand() % nScreenWidth;
					nFoodY = (rand() % (nScreenHeight - 3)) + 3;
				}

				for (int i = 0; i < 5; i++)
					snake.push_back({ snake.back().x, snake.back().y });
			}

			// Collision Detect Snake V World
			if (snake.front().x < 0 || snake.front().x >= nScreenWidth)
				bDead = true;
			if (snake.front().y < 3 || snake.front().y >= nScreenHeight)
				bDead = true;

			// Collision Detect Snake V Snake
			for (list<sSnakeSegment>::iterator i = snake.begin(); i != snake.end(); i++)
				if (i != snake.begin() && i->x == snake.front().x && i->y == snake.front().y)
					bDead = true;

			// Chop off Snakes tail :-/
			snake.pop_back();

			// ==== Presentation

			// Clear Screen
			for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

			// Draw Stats & Border
			for (int i = 0; i < nScreenWidth; i++)//upper border
			{
				screen[i] = L'=';
			}
			if (nScreenHeight > 1)//lower border
			{
				for (int i = 0; i < nScreenWidth; i++)
				{
					screen[(nScreenHeight - 1) * nScreenWidth + i] = L'=';
				}
			}

			wsprintf(&screen[nScreenWidth + 5], L"gujingqiuyun.github.io - S N A K E ! !                SCORE: %d", nScore);


			// Draw Snake Body
			for (auto s : snake)
				screen[s.y * nScreenWidth + s.x] = bDead ? L'+' : L'O';

			// Draw Snake Head

			screen[snake.front().y * nScreenWidth + snake.front().x] = bDead ? L'X' : L'@';

			// Draw Food(&check border)
			if (nFoodX >= 0 && nFoodX < nScreenWidth && nFoodY >= 0 && nFoodY < nScreenHeight)
			{
				screen[nFoodY * nScreenWidth + nFoodX] = L'%';
			}
			

			if (bDead)
				wsprintf(&screen[15 * nScreenWidth + 40], L"    PRESS 'SPACE' TO PLAY AGAIN    ");

			// Display Frame
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		}

		// Wait for space
		while ((0x8000 & GetAsyncKeyState((unsigned char)('\x20'))) == 0);
	}

	return 0;
}