#pragma once
#include "Graphic.h"

void CreateSphere(ID3D11Device* p_D3d11device,int LatLines, int LongLines);
void UninitSphere();
void InitSkybox(ID3D11Device* p_D3d11device);
void UpdateSkybox();
void DrawSkybox(ID3D11DeviceContext* d3d11DevCon);