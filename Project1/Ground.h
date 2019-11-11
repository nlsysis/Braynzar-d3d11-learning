#pragma once
#include "Graphic.h"

void InitGround(ID3D11Device* p_D3d11device, ID3D11Buffer* &c_squareVertBuffer, ID3D11Buffer* &c_squareIndexBuffer);
void UpdateGround();
void DrawGround(ID3D11DeviceContext* d3d11DevCon);