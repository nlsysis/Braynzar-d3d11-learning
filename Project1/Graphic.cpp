#include "Graphic.h"
#include "DXinput.h"
#include "Cube.h"
#include "Camera.h"
#include "Ground.h"
#include "Skybox.h"

//Global Declarations - Interfaces//
IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;
///////////////**************new**************////////////////////
ID3D11Buffer* squareIndexBuffer;
ID3D11Buffer* squareVertBuffer;
///////////////**************new**************////////////////////
ID3D11Buffer* cbPerObjectBuffer;    //constant buffer
ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* vertLayout;

ID3D11DepthStencilView *depthStencilView;
ID3D11Texture2D *depthStencilBuffer;

//ID3D11RasterizerState* WireFrame;

//ID3D11ShaderResourceView* CubesTexture;
ID3D11SamplerState* CubesTexSamplerState;

//for blending
ID3D11BlendState* Transparency;
ID3D11RasterizerState* CCWcullMode;
ID3D11RasterizerState* CWcullMode;

//for pixel clip
ID3D11RasterizerState* noCull;

//for lighting
ID3D11Buffer* cbPerFrameBuffer;


extern HWND hwnd;
extern HRESULT hr;

XMMATRIX WVP;
XMMATRIX World;
XMMATRIX camView;
XMMATRIX camProjection;
XMMATRIX cube1World;

float rot = 0.01f;

/////for test  input
float rotx = 0;
float rotz = 0;
float scaleX = 1.0f;
float scaleY = 1.0f;



D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
};
UINT numElements = ARRAYSIZE(layout);



cbPerObject cbPerObj;

struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}
	XMFLOAT3 dir;
	float pad;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};

Light light;

//cbPerFrame Structure
struct cbPerFrame
{
	Light  light;
};

cbPerFrame constbuffPerFrame;


bool InitializeDirect3d11App(HINSTANCE hInstance)
{
	//Describe our Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = WIDTH;
	bufferDesc.Height = HEIGHT;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;                // we can go in and out of the fullscreen
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


	//Create our SwapChain
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

	//Create our BackBuffer
	ID3D11Texture2D* BackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

	//Create our Render Target
	hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
	BackBuffer->Release();


	///////////////**************new**************////////////////////
   //Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = WIDTH;
	depthStencilDesc.Height = HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//Create the DepthStencil View
	d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	//Set our Render Target
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	///////////////**************new**************////////////////////

	//hr = D3DX11CreateShaderResourceViewFromFile(d3d11Device, ".\\Resource\\braynzar.png",
	//	NULL, NULL, &CubesTexture, NULL);

	//hr = D3DX11CreateShaderResourceViewFromFile(d3d11Device, ".\\Resource\\grass.jpg",
		//NULL, NULL, &CubesTexture, NULL);

	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_NONE;

	d3d11Device->CreateRasterizerState(&rastDesc, &noCull);

	d3d11DevCon->RSSetState(NULL);
	//Draw objects that will use backface culling

	d3d11DevCon->RSSetState(noCull);

	InitSkybox(d3d11Device);
	return true;
}

void CleanUp()
{
	//Release the COM Objects we created
	SwapChain->SetFullscreenState(false, NULL);
	PostMessage(hwnd, WM_DESTROY, 0, 0);

	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	renderTargetView->Release();
	///////////////**************new**************////////////////////
	squareVertBuffer->Release();
	squareIndexBuffer->Release();
	///////////////**************new**************//ObjSamplerState//////////////////
	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();
	cbPerObjectBuffer->Release();
	//WireFrame->Release();
	Transparency->Release();
	CCWcullMode->Release();
	CWcullMode->Release();
	//blender

	//for pixel clip
	noCull->Release();

	//for lighting
	//d2dTexture->Release();
	cbPerFrameBuffer->Release();
	UninitSphere();
}

bool InitScene()
{
	
	//Compile Shaders from shader file
	hr = D3DX11CompileFromFile("Effects.fx", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
	hr = D3DX11CompileFromFile("Effects.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);
	//hr = D3DX11CompileFromFile("Effects.fx", 0, 0, "D2D_PS", "ps_4_0", 0, 0, 0, &D2D_PS_Buffer, 0, 0);

	//Create the Shader Objects
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);
	//hr = d3d11Device->CreatePixelShader(D2D_PS_Buffer->GetBufferPointer(), D2D_PS_Buffer->GetBufferSize(), NULL, &D2D_PS);

	//Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);

	///////////////**************new**************////////////////////

	//light.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//light.range = 100.0f;
	//light.att = XMFLOAT3(0.0f, 0.2f, 0.0f);
	//light.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	light.dir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	light.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	//InitCube(d3d11Device, squareVertBuffer, squareIndexBuffer);
	InitGround(d3d11Device, squareVertBuffer, squareIndexBuffer);

	d3d11DevCon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	///////////////**************new**************////////////////////

	//Set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &squareVertBuffer, &stride, &offset);

	//Create the Input Layout
	hr = d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(),
		VS_Buffer->GetBufferSize(), &vertLayout);

	//Set the Input Layout
	d3d11DevCon->IASetInputLayout(vertLayout);

	//Set Primitive Topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WIDTH;
	viewport.Height = HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//Set the Viewport
	d3d11DevCon->RSSetViewports(1, &viewport);

	//Create the buffer to send to the cbuffer in effect file
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

	InitCamera();

	//Set the Projection matrix
	camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, (float)WIDTH / HEIGHT, 1.0f, 100.0f);
	
	//set rasterizer state
	//D3D11_RASTERIZER_DESC wfdesc;
	//ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	//wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	//wfdesc.CullMode = D3D11_CULL_NONE;
	//hr = d3d11Device->CreateRasterizerState(&wfdesc, &WireFrame);

	//d3d11DevCon->RSSetState(WireFrame);

	// Describe the Sample State
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//Create the Sample State
	hr = d3d11Device->CreateSamplerState(&sampDesc, &CubesTexSamplerState);

	//blending equation
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_DEST_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	d3d11Device->CreateBlendState(&blendDesc, &Transparency);

	//Create the Counter Clockwise and Clockwise Culling States
	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	hr = d3d11Device->CreateRasterizerState(&cmdesc, &CCWcullMode);

	cmdesc.FrontCounterClockwise = false;
	hr = d3d11Device->CreateRasterizerState(&cmdesc, &CWcullMode);

	//for lighting
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerFrame);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerFrameBuffer);

	CreateSphere(d3d11Device, 10, 10);
	return true;
}

void UpdateScene()
{
	if (isPressed(DIK_ESCAPE))
	{
		PostMessage(hwnd, WM_DESTROY, 0, 0);
	}
	//Keep the cubes rotating
	rot += .0005f;
	if (rot > 6.28f)
		rot = 0.0f;

	//UpdateCube();
	

	//XMVECTOR lightVector = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//lightVector = XMVector3TransformCoord(lightVector, cube1World);

	//light.pos.x = XMVectorGetX(lightVector);
	//light.pos.y = XMVectorGetY(lightVector);
	//light.pos.z = XMVectorGetZ(lightVector);

	if (isPressed(DIK_LEFT))
	{
		rotz -= 1.0f;
	}
	if (isPressed(DIK_RIGHT))
	{
		rotz += 1.0f;
	}
	//using camera
	UpdateCamera();
	UpdateGround();
	UpdateSkybox();
}

void DrawScene()
{
	//Clear our backbuffer
	float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
	//Refresh the Depth/Stencil view
	d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	constbuffPerFrame.light = light;
	d3d11DevCon->UpdateSubresource(cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0);
	d3d11DevCon->PSSetConstantBuffers(0, 1, &cbPerFrameBuffer);

	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	//Set the default blend state (no blending) for opaque objects
	d3d11DevCon->OMSetBlendState(0, 0, 0xffffffff);

	//Reset Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);

	//DrawCube();
	DrawGround(d3d11DevCon);


	DrawSkybox(d3d11DevCon);
	
	//Set the default VS shader and depth/stencil state
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);
	d3d11DevCon->OMSetDepthStencilState(NULL, 0);
	//Present the backbuffer to the screen
	SwapChain->Present(0, 0);
}
