#pragma once
#include "Graphic.h"


void InitCube(ID3D11Device* p_D3d11device, ID3D11Buffer* &c_squareVertBuffer, ID3D11Buffer* &c_squareIndexBuffer);
void UpdateCube();
void DrawCube();