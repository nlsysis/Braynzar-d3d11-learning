#include "DXinput.h"

////////Global Declaration
IDirectInputDevice8 *DIKeyboard;
IDirectInputDevice8 *DIMouse;

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;

DIMOUSESTATE mouseCurrState;
BYTE keyboardState[256];


extern HWND hwnd;
extern HRESULT hr;
bool InitDirectInput(HINSTANCE hInstance)
{
	hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL);
	//create directx input device

	hr = DirectInput->CreateDevice(GUID_SysKeyboard,
		&DIKeyboard, NULL);

	hr = DirectInput->CreateDevice(GUID_SysMouse,
		&DIMouse, NULL);
	//set data format

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	hr = DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true;
}
void DetectInput(double time)
{
	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
}

bool isPressed(BYTE key)
{
	return (keyboardState[key] & 0x80);
}

bool isMouseMoved()
{
	bool isMoved = (mouseCurrState.lX != mouseLastState.lX || mouseCurrState.lY != mouseCurrState.lY);
	mouseCurrState = mouseLastState;
	return isMoved;

}


void UninitDirectInput()
{
	DIKeyboard->Unacquire();
	DIMouse->Unacquire();
	DirectInput->Release();
}