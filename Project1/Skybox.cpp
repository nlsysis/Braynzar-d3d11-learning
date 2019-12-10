#include <vector>
#include "Skybox.h"
#include "camera.h"


ID3D11Buffer* sphereIndexBuffer;
ID3D11Buffer* sphereVertBuffer;

ID3D11VertexShader* SKYMAP_VS;
ID3D11PixelShader* SKYMAP_PS;
ID3D10Blob* SKYMAP_VS_BUFFER;
ID3D10Blob* SKYMAP_PS_BUFFER;

ID3D11ShaderResourceView* smrv;

ID3D11DepthStencilState* DSLessEqual;
ID3D11RasterizerState* RSCullNone;

int NumSphereVertices;
int NumSphereFaces;

XMMATRIX sphereWorld,Scale_sky,Translation_sky, WVP_cube;

//cbPerObject cube_perObject;
XMMATRIX Rotationx_cube;
XMMATRIX Rotationy_cube;

extern cbPerObject cbPerObj;
extern HRESULT hr;
extern XMVECTOR camPosition;
extern XMMATRIX camView;
extern XMMATRIX camProjection;
extern ID3D11Buffer* cbPerObjectBuffer;
extern ID3D11SamplerState* CubesTexSamplerState;
extern ID3D11VertexShader* VS;

//create a sphere and map the sky onto it
void CreateSphere(ID3D11Device* p_D3d11device,int LatLines, int LongLines)
{
	NumSphereVertices = ((LatLines - 2) * LongLines) + 2;
	NumSphereFaces = ((LatLines - 3)*(LongLines) * 2) + (LongLines * 2);

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;

	std::vector<Vertex> vertices(NumSphereVertices);

	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	vertices[0].pos.x = 0.0f;
	vertices[0].pos.y = 0.0f;
	vertices[0].pos.z = 1.0f;

	for (DWORD i = 0; i < LatLines - 2; ++i)
	{
		spherePitch = (i + 1) * (3.14 / (LatLines - 1));
		Rotationx_cube = XMMatrixRotationX(spherePitch);
		for (DWORD j = 0; j < LongLines; ++j)
		{
			sphereYaw = j * (6.28 / (LongLines));
			Rotationy_cube = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx_cube * Rotationy_cube));
			currVertPos = XMVector3Normalize(currVertPos);
			vertices[i*LongLines + j + 1].pos.x = XMVectorGetX(currVertPos);
			vertices[i*LongLines + j + 1].pos.y = XMVectorGetY(currVertPos);
			vertices[i*LongLines + j + 1].pos.z = XMVectorGetZ(currVertPos);
		}
	}

	vertices[NumSphereVertices - 1].pos.x = 0.0f;
	vertices[NumSphereVertices - 1].pos.y = 0.0f;
	vertices[NumSphereVertices - 1].pos.z = -1.0f;


	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * NumSphereVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	hr = p_D3d11device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &sphereVertBuffer);


	std::vector<DWORD> indices(NumSphereFaces * 3);

	int k = 0;
	for (DWORD l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = 0;
		indices[k + 1] = l + 1;
		indices[k + 2] = l + 2;
		k += 3;
	}

	indices[k] = 0;
	indices[k + 1] = LongLines;
	indices[k + 2] = 1;
	k += 3;

	for (DWORD i = 0; i < LatLines - 3; ++i)
	{
		for (DWORD j = 0; j < LongLines - 1; ++j)
		{
			indices[k] = i * LongLines + j + 1;
			indices[k + 1] = i * LongLines + j + 2;
			indices[k + 2] = (i + 1)*LongLines + j + 1;

			indices[k + 3] = (i + 1)*LongLines + j + 1;
			indices[k + 4] = i * LongLines + j + 2;
			indices[k + 5] = (i + 1)*LongLines + j + 2;

			k += 6; // next quad
		}

		indices[k] = (i*LongLines) + LongLines;
		indices[k + 1] = (i*LongLines) + 1;
		indices[k + 2] = ((i + 1)*LongLines) + LongLines;

		indices[k + 3] = ((i + 1)*LongLines) + LongLines;
		indices[k + 4] = (i*LongLines) + 1;
		indices[k + 5] = ((i + 1)*LongLines) + 1;

		k += 6;
	}

	for (DWORD l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = NumSphereVertices - 1;
		indices[k + 1] = (NumSphereVertices - 1) - (l + 1);
		indices[k + 2] = (NumSphereVertices - 1) - (l + 2);
		k += 3;
	}

	indices[k] = NumSphereVertices - 1;
	indices[k + 1] = (NumSphereVertices - 1) - LongLines;
	indices[k + 2] = NumSphereVertices - 2;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * NumSphereFaces * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	p_D3d11device->CreateBuffer(&indexBufferDesc, &iinitData, &sphereIndexBuffer);
}

void UninitSphere()
{
	sphereIndexBuffer->Release();
	sphereVertBuffer->Release();
	SKYMAP_PS->Release();
	SKYMAP_VS->Release();
	SKYMAP_VS_BUFFER->Release();
	SKYMAP_PS_BUFFER->Release();

	smrv->Release();
	DSLessEqual->Release();
	RSCullNone->Release();
}

void InitSkybox(ID3D11Device* p_D3d11device)
{
	hr = D3DX11CompileFromFile("Effects.fx",0,0,"SKYMAP_VS","vs_4_0",0,0,0,&SKYMAP_VS_BUFFER,0,0);
	hr = D3DX11CompileFromFile("Effects.fx", 0, 0, "SKYMAP_PS", "ps_4_0", 0, 0, 0, &SKYMAP_PS_BUFFER, 0, 0);

	hr = p_D3d11device->CreateVertexShader(SKYMAP_VS_BUFFER->GetBufferPointer(),SKYMAP_VS_BUFFER->GetBufferSize(),0,&SKYMAP_VS);
	hr = p_D3d11device->CreatePixelShader(SKYMAP_PS_BUFFER->GetBufferPointer(), SKYMAP_PS_BUFFER->GetBufferSize(), 0, &SKYMAP_PS);

	//load the cube map
	D3DX11_IMAGE_LOAD_INFO loadSMInfo;
	loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* SMTexture = 0;
	hr = D3DX11CreateTextureFromFile(p_D3d11device, ".\\Resource\\skymap.dds",&loadSMInfo,0,
		(ID3D11Resource**) &SMTexture,0);

	//get description of our texture for creating resource view description that matched that texture we loaded in
	D3D11_TEXTURE2D_DESC SMTextureDesc;
	SMTexture->GetDesc(&SMTextureDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = SMTextureDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = SMTextureDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	hr = p_D3d11device->CreateShaderResourceView(SMTexture, &SMViewDesc, &smrv);
	
	//disable culling
	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;
	cmdesc.FrontCounterClockwise = false;
	hr = p_D3d11device->CreateRasterizerState(&cmdesc, &RSCullNone);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;   //make sure the far view can be drawn no the pixel of skymap

	p_D3d11device->CreateDepthStencilState(&dssDesc, &DSLessEqual);
}

void UpdateSkybox()
{
	//reset sphereWorld
	sphereWorld = XMMatrixIdentity();
	//Define sphereWorld's world space matrix
	Scale_sky = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	//Make sure the sphere is always centered around camera
	Translation_sky = XMMatrixTranslation(XMVectorGetX(camPosition), XMVectorGetY(camPosition), XMVectorGetZ(camPosition));

	//Set sphereWorld's world space using the transformations
	sphereWorld = Scale_sky * Translation_sky;
}

void DrawSkybox(ID3D11DeviceContext* d3d11DevCon)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetIndexBuffer(sphereIndexBuffer, DXGI_FORMAT_R32_UINT,0);
	d3d11DevCon->IASetVertexBuffers(0,1,&sphereVertBuffer,&stride,&offset);

	WVP_cube = sphereWorld * camView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP_cube);
	cbPerObj.World = XMMatrixTranspose(sphereWorld);

	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	d3d11DevCon->PSSetShaderResources(0, 1, &smrv);
	d3d11DevCon->PSSetSamplers(0, 1, &CubesTexSamplerState);

	d3d11DevCon->VSSetShader(SKYMAP_VS, 0, 0);
	d3d11DevCon->PSSetShader(SKYMAP_PS, 0, 0);
	d3d11DevCon->OMSetDepthStencilState(DSLessEqual, 0);
	d3d11DevCon->RSSetState(RSCullNone);
	d3d11DevCon->DrawIndexed(NumSphereFaces * 3, 0, 0);

	

}