#include "Ground.h"

XMMATRIX groundWorld;
ID3D11ShaderResourceView* GroundTexture;
extern HRESULT hr;
extern XMMATRIX WVP;
extern XMMATRIX World;
extern XMMATRIX camView;
extern XMMATRIX camProjection;
extern ID3D11Buffer* cbPerObjectBuffer;
extern ID3D11SamplerState* CubesTexSamplerState;
extern HRESULT hr;
extern ID3D11RasterizerState* CCWcullMode;
extern cbPerObject cbPerObj;

void InitGround(ID3D11Device* p_D3d11device,ID3D11Buffer* &c_squareVertBuffer, ID3D11Buffer* &c_squareIndexBuffer)
{

	//Create the vertex buffer
	Vertex v[] =
	{
		// Bottom Face
		Vertex(-1.0f, -1.0f, -1.0f, 100.0f, 100.0f, 0.0f, 1.0f, 0.0f),
		Vertex(1.0f, -1.0f, -1.0f,   0.0f, 100.0f, 0.0f, 1.0f, 0.0f),
		Vertex(1.0f, -1.0f,  1.0f,   0.0f,   0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(-1.0f, -1.0f,  1.0f, 100.0f,   0.0f, 0.0f, 1.0f, 0.0f),
	};

	DWORD indices[] = {
		0,  1,  2,
		0,  2,  3,
	};


	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices;
	p_D3d11device->CreateBuffer(&indexBufferDesc, &iinitData, &c_squareIndexBuffer);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;
	hr = p_D3d11device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &c_squareVertBuffer);

	hr = D3DX11CreateShaderResourceViewFromFile(p_D3d11device, ".\\Resource\\grass.jpg",
		NULL, NULL, &GroundTexture, NULL);
}
void UpdateGround()
{
	groundWorld = XMMatrixIdentity();
	XMMATRIX Scale = XMMatrixScaling(500.0f, 10.0f, 500.0f);
	XMMATRIX Translation = XMMatrixTranslation(0.0f, 10.0f, 0.0f);

	//Set cube1's world space using the transformations
	groundWorld = Scale * Translation;
}
void DrawGround(ID3D11DeviceContext* d3d11DevCon)
{
	WVP = groundWorld * camView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	cbPerObj.World = XMMatrixTranspose(groundWorld);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	d3d11DevCon->PSSetShaderResources(0, 1, &GroundTexture);
	d3d11DevCon->PSSetSamplers(0, 1, &CubesTexSamplerState);

	d3d11DevCon->RSSetState(CCWcullMode);
	d3d11DevCon->DrawIndexed(6, 0, 0);
}