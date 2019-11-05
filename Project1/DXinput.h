#pragma once

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include <dinput.h>
#include "Graphic.h"

bool InitDirectInput(HINSTANCE hInstance);
void DetectInput(double time);
bool isPressed(BYTE key);
void UninitDirectInput();

