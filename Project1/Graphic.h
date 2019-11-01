#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <tchar.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>

#define WIDTH 300
#define HEIGHT 300

//Function Prototypes//
bool InitializeDirect3d11App(HINSTANCE hInstance);
void CleanUp();
bool InitScene();
void UpdateScene();
void DrawScene();

struct Vertex    //Overloaded Vertex Structure
{
	Vertex() {}
	Vertex(float x, float y, float z,
		float u, float v)
		: pos(x, y, z), texCoord(u, v) {}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
};