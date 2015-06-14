#include "MathFunc.h"
#include "TimerClass.h"
#include "FPSClass.h"
#include "CPUClass.h"

#include <ctime>
#include <string>

#include <dinput.h>
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

#define BUFFER_WIDTH	1280
#define BUFFER_HEIGHT	768


class GraphicsProject {

	//	Application data
	HINSTANCE				application;
	WNDPROC					appWndProc;
	HWND					window;

	//	D3D11 Data
	D3D11_VIEWPORT			viewport;

	IDXGISwapChain*			swapChain = nullptr;
	ID3D11Device*			device = nullptr;
	ID3D11DeviceContext*	devContext = nullptr;
	ID3D11RenderTargetView* rtView = nullptr;

	ID3D11InputLayout*		vertLayout = nullptr;

	ID3D11Buffer*			cbPerObjectBuffer = nullptr;
	ID3D11Buffer*			iBuffer_Cube = nullptr;
	ID3D11Buffer*			vBuffer_Cube = nullptr;
	
	ID3D11VertexShader*		VS = nullptr;
	ID3D11PixelShader*		PS = nullptr;

	ID3D10Blob*				VS_Buffer = nullptr;
	ID3D10Blob*				PS_Buffer = nullptr;

	ID3D11ShaderResourceView* CubeTexture = nullptr;
	ID3D11SamplerState*		ssCube = nullptr;

	ID3D11Texture2D*		dsBuffer = nullptr;
	ID3D11DepthStencilView* dsView = nullptr;
	
	ID3D11RasterizerState*	rState_B_AA = nullptr;
	ID3D11RasterizerState*	rState_B = nullptr;
	ID3D11RasterizerState*	rState_F_AA = nullptr;
	ID3D11RasterizerState*	rState_F = nullptr;
	ID3D11RasterizerState*	rState_Wire = nullptr;

	//	Model Loading
	Model					m_model;
	ID3D11Buffer*			iBuffer_Model = nullptr;
	ID3D11Buffer*			vBuffer_Model = nullptr;

	//	Input Data
	IDirectInputDevice8*	DIKeyboard;
	IDirectInputDevice8*	DIMouse;

	DIMOUSESTATE			mouseLastState;
	LPDIRECTINPUT8			DirectInput;

	//	Systems Data
	FPSClass				fpsTracker;
	TimerClass				timeTracker;
	CpuClass				cpuTracker;

	//	Objects
	cbPerObject cbPerObj;

	MATRIX4X4 WVP;
	MATRIX4X4 cube1World;
	MATRIX4X4 cube2World;
	MATRIX4X4 modelWorld;
	MATRIX4X4 camView;
	MATRIX4X4 camProjection;

	FLOAT4 camPosition;
	FLOAT4 camTarget;
	FLOAT4 camUp;

	MATRIX4X4 Rotation;
	MATRIX4X4 Scale;
	MATRIX4X4 Translation;
	float rot = 0.01f;

public:

	GraphicsProject();
	GraphicsProject(HINSTANCE hinst, WNDPROC proc);

	bool InitScene();
	bool Update();
	void Render();
	bool ShutDown();

	void ResizeWin();
	void ChangeTitleBar(std::string _str);

	void DetectInput(double time, float w, float h);
	bool InitDirectInput(HINSTANCE hInstance);

	bool loadOBJ(const char* path);
};

GraphicsProject* pApp = nullptr;

GraphicsProject::GraphicsProject(HINSTANCE hinst, WNDPROC proc){

	HRESULT result;

#pragma region App & Window
	pApp = this;
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"Project", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
#pragma endregion

#pragma region Initialize Trackers
	fpsTracker.Initialize();
	timeTracker.Initialize();
	cpuTracker.Initialize();

	if (!InitDirectInput(hinst)){
		MessageBox(0, L"Direct Input Initialization - Failed",
			L"Error", MB_OK);
		return;
	}
#pragma endregion

#pragma region Device SwapChain
	D3D_FEATURE_LEVEL features[3];

	D3D_FEATURE_LEVEL pFeatureLevels;
	features[0] = D3D_FEATURE_LEVEL_11_0;
	features[1] = D3D_FEATURE_LEVEL_10_1;
	features[2] = D3D_FEATURE_LEVEL_10_0;

	DXGI_MODE_DESC buffDesc;
	ZeroMemory(&buffDesc, sizeof(DXGI_MODE_DESC));
	buffDesc.Width = BUFFER_WIDTH;
	buffDesc.Height = BUFFER_HEIGHT;
	buffDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	buffDesc.RefreshRate.Denominator = 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = buffDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.Windowed = TRUE;

	UINT deviceFlags = 0;
#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	result = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		deviceFlags,
		features,
		3,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain,
		&device,
		&pFeatureLevels,
		&devContext
		);
#pragma endregion

#pragma region RenderTargetView
	ID3D11Resource* pBB;
	swapChain->GetBuffer(0, __uuidof(pBB),
		reinterpret_cast<void**>(&pBB));
	result = device->CreateRenderTargetView(pBB, NULL, &rtView);
	pBB->Release();
#pragma endregion

#pragma region DepthStencil
	D3D11_TEXTURE2D_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(D3D11_TEXTURE2D_DESC));
	dsDesc.Width = BUFFER_WIDTH;
	dsDesc.Height = BUFFER_HEIGHT;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.SampleDesc.Count = 1;

	result = device->CreateTexture2D(&dsDesc, NULL, &dsBuffer);
	result = device->CreateDepthStencilView(dsBuffer, NULL, &dsView);

	devContext->OMSetRenderTargets(1, &rtView, dsView);
#pragma endregion

}

bool GraphicsProject::InitScene(){

	HRESULT result;

#pragma region Compile .fx Shaders
	result = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "main", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
	result = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

	result = device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);	//	vShader cube
	result = device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);	//	pShader cube
#pragma endregion

#pragma region Load Model
	loadOBJ("Link_tri.obj");
#pragma endregion

#pragma region Cube Setup
	VERTEX Cube[] =
	{
		// Front Face
		VERTEX(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		VERTEX(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f),
		VERTEX(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f),
		VERTEX(1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f),

		// Back Face
		VERTEX(-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f),
		VERTEX(1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f),
		VERTEX(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		VERTEX(-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f),

		// Top Face
		VERTEX(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, -1.0f),
		VERTEX(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),
		VERTEX(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		VERTEX(1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f),

		// Bottom Face
		VERTEX(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		VERTEX(1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f),
		VERTEX(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f),
		VERTEX(-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 1.0f),

		// Left Face
		VERTEX(-1.0f, -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 1.0f),
		VERTEX(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),
		VERTEX(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 1.0f, -1.0f),
		VERTEX(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f),

		// Right Face
		VERTEX(1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f),
		VERTEX(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f),
		VERTEX(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		VERTEX(1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f),
	};

	UINT iCube[] = {
		// Front Face
		0, 1, 2,
		0, 2, 3,

		// Back Face
		4, 5, 6,
		4, 6, 7,

		// Top Face
		8, 9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};
#pragma endregion

#pragma region IndexBuffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * 12 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA iinitData;
	ZeroMemory(&iinitData, sizeof(D3D11_SUBRESOURCE_DATA));
	iinitData.pSysMem = iCube;

	result = device->CreateBuffer(&indexBufferDesc, &iinitData, &iBuffer_Cube);
#pragma endregion

#pragma region VertexBuffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VERTEX) * 24;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertBufferData;
	ZeroMemory(&vertBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertBufferData.pSysMem = Cube;

	result = device->CreateBuffer(&vertexBufferDesc, &vertBufferData, &vBuffer_Cube);
#pragma endregion

#pragma region InputLayer
	D3D11_INPUT_ELEMENT_DESC layout[3];
	layout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	UINT arrSize = ARRAYSIZE(layout);
	result = device->CreateInputLayout(layout, arrSize, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);
#pragma endregion

#pragma region Viewport
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.Height = BUFFER_HEIGHT;
	viewport.Width = BUFFER_WIDTH;
	viewport.MaxDepth = 1.0f;
#pragma endregion

#pragma region ConstBuffer
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	result = device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);
#pragma endregion

#pragma region Cam Info
	camPosition = FLOAT4(0.0f, 3.0f, -8.0f, 0.0f);
	camTarget = FLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = FLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	camView = CreateViewMatrix(camPosition, camTarget, camUp);

	unsigned int aspect = (float)BUFFER_WIDTH / BUFFER_HEIGHT;
	camProjection = CreateProjectionMatrix(100.0f, 0.1f, 72, aspect);
#pragma endregion

#pragma region Load Textures
	result = D3DX11CreateShaderResourceViewFromFile(device, L"grass.jpg", NULL, NULL, &CubeTexture, NULL);
#pragma endregion

#pragma region SamplerState
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&sampDesc, &ssCube);
#pragma endregion

#pragma region RasterDesc
	D3D11_RASTERIZER_DESC rasDesc;
	ZeroMemory(&rasDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK; 
	rasDesc.DepthClipEnable = TRUE;
	result = device->CreateRasterizerState(&rasDesc, &rState_B);	//	back cull - no AA

	rasDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
	result = device->CreateRasterizerState(&rasDesc, &rState_F);	//	front cull - no AA

	rasDesc.AntialiasedLineEnable = TRUE;
	result = device->CreateRasterizerState(&rasDesc, &rState_F_AA);	//	front cull - AA

	rasDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	result = device->CreateRasterizerState(&rasDesc, &rState_B_AA);	//	back cull - AA

	rasDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	result = device->CreateRasterizerState(&rasDesc, &rState_Wire);	//	back cull - Wireframe
#pragma endregion

	return true;
}

bool GraphicsProject::Update() {

#pragma region Update perFrame
	fpsTracker.Frame();
	timeTracker.Frame();
	cpuTracker.Frame();

	DetectInput(timeTracker.GetTime(), (float)BUFFER_WIDTH, (float)BUFFER_HEIGHT);

	std::string lpwinname;
	lpwinname = "FPS : ";
	lpwinname += std::to_string(fpsTracker.GetFps());
	lpwinname += ", Elapsed Time : ";
	lpwinname += std::to_string(timeTracker.GetElapsedTime());
	lpwinname += ", CPU : ";
	lpwinname += std::to_string(cpuTracker.GetCpuPercentage());
	lpwinname += " %";
	pApp->ChangeTitleBar(lpwinname);
#pragma endregion

	rot += timeTracker.GetTime() * 0.0025f;
	if (rot > 6.26f)
		rot = 0.0f;

	//Reset cube1World
	cube1World = Identity();

	//Define cube1's world space matrix
	cube1World = Translate(cube1World, 5.0f, 0.0f, 3.0f);
	cube1World = RotateZ_Local(cube1World, rot);

	//Reset cube2World
	cube2World = Identity();

	//Define cube2's world space matrix
	cube2World = Scale_4x4(cube2World, 0.5f, 0.5f, 0.5f);
	cube2World = RotateZ_Local(cube2World, -rot);
	cube2World = RotateX_Local(cube2World, -rot);

	//Reset cube2World
	modelWorld = Identity();

	//Define cube2's world space matrix
	modelWorld = Scale_4x4(modelWorld, 0.2f, 0.2f, 0.2f);
	modelWorld = Translate(modelWorld, 0.0f, 0.0f, 6.0f);
	modelWorld = RotateZ_Local(modelWorld, 6.28f * 2.0f);

	Render();

	return true;
}

void GraphicsProject::Render(){

	//	Background Color
	FLOAT RGBA[4]; RGBA[0] = 0; RGBA[1] = 0; RGBA[2] = 0; RGBA[3] = 1;

	//	Clear views
	devContext->ClearRenderTargetView(rtView, RGBA);
	devContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	devContext->OMSetRenderTargets(1, &rtView, dsView);
	devContext->RSSetViewports(1, &viewport);


	//	Link Model
	WVP = Mult_4x4(modelWorld, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.WVP = WVP;

	UINT stride = sizeof(Vert);
	UINT offset = 0;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	devContext->RSSetState(rState_B);
	devContext->IASetVertexBuffers(0, 1, &vBuffer_Model, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Model, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(VS, NULL, 0);
	devContext->PSSetShader(PS, NULL, 0);
	devContext->IASetInputLayout(vertLayout);
	devContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_BUFFER_DESC ibufferDesc;
	iBuffer_Model->GetDesc(&ibufferDesc);
	UINT numIndicies = ibufferDesc.ByteWidth / sizeof(UINT);
	devContext->DrawIndexed(numIndicies, 0, 0);


		//	1st Cube
	WVP = Mult_4x4(cube1World, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.WVP = WVP;

	stride = sizeof(VERTEX);	
	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);	
	devContext->IASetVertexBuffers(0, 1, &vBuffer_Cube, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(VS, NULL, 0);
	devContext->PSSetShader(PS, NULL, 0);
	devContext->IASetInputLayout(vertLayout);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->PSSetShaderResources(0, 1, &CubeTexture);
	devContext->PSSetSamplers(0, 1, &ssCube);
	devContext->DrawIndexed(36, 0, 0);

		//	2nd Cube
	WVP = Mult_4x4(cube2World, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	devContext->RSSetState(rState_Wire);
	devContext->IASetVertexBuffers(0, 1, &vBuffer_Cube, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(VS, NULL, 0);
	devContext->PSSetShader(PS, NULL, 0);
	devContext->IASetInputLayout(vertLayout);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->PSSetShaderResources(0, 1, &CubeTexture);
	devContext->PSSetSamplers(0, 1, &ssCube);
	devContext->DrawIndexed(36, 0, 0);

	swapChain->Present(0, 0);
}

void GraphicsProject::ChangeTitleBar(std::string _str){
	char *data = new char[_str.size() + 1];
	data[_str.size()] = 0;
	memcpy(data, _str.c_str(), _str.size());
	SetWindowTextA(pApp->window, data);
	delete[] data;
}

void GraphicsProject::ResizeWin() {

	// Safety check
	if (this->swapChain == nullptr || this->devContext == nullptr || this->device == nullptr)
		return;

	//	Find out new width & height
	HRESULT hr;
	RECT rcClient;
	GetClientRect(this->window, &rcClient);
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;

	//	redefine #defines
#undef BUFFER_WIDTH
#define BUFFER_WIDTH	width
#undef BUFFER_HEIGHT
#define BUFFER_HEIGHT	height

	//	clear some stuff
	devContext->ClearState();
	devContext->OMSetRenderTargets(NULL, NULL, NULL);
	rtView->Release();
	rtView = nullptr;
	dsBuffer->Release();
	dsView->Release();
	dsView = nullptr;

	//	resize
	hr = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

	//	Create new rtv
	ID3D11Resource* pBB;
	swapChain->GetBuffer(0, __uuidof(pBB),
		reinterpret_cast<void**>(&pBB));

	DXGI_SWAP_CHAIN_DESC scDesc;
	swapChain->GetDesc(&scDesc);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

	hr = device->CreateRenderTargetView(pBB, NULL, &rtView);
	pBB->Release();

	D3D11_TEXTURE2D_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(D3D11_TEXTURE2D_DESC));
	dsDesc.Width = BUFFER_WIDTH;
	dsDesc.Height = BUFFER_HEIGHT;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.SampleDesc.Count = 1;

	hr = device->CreateTexture2D(&dsDesc, NULL, &dsBuffer);
	hr = device->CreateDepthStencilView(dsBuffer, NULL, &dsView);

	devContext->OMSetRenderTargets(1, &rtView, dsView);

	// Set new proj matrix
	float ar = abs((float)width / (float)height);
	camProjection = CreateProjectionMatrix(100.0f, 0.1f, 72, ar);

	//	Change view port
	viewport.Width = BUFFER_WIDTH;
	viewport.Height = BUFFER_HEIGHT;
}

bool GraphicsProject::InitDirectInput(HINSTANCE hInstance){

	HRESULT result;

	result = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL);

	result = DirectInput->CreateDevice(GUID_SysKeyboard,
		&DIKeyboard,
		NULL);

	result = DirectInput->CreateDevice(GUID_SysMouse,
		&DIMouse,
		NULL);

	result = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	result = DIKeyboard->SetCooperativeLevel(pApp->window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	result = DIMouse->SetDataFormat(&c_dfDIMouse);
	result = DIMouse->SetCooperativeLevel(pApp->window, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true;
}

void GraphicsProject::DetectInput(double time, float w, float h){

	BYTE keyboardState[256];

	DIKeyboard->Acquire();

	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	if (keyboardState[DIK_ESCAPE] & 0x80)
		PostMessage(pApp->window, WM_DESTROY, 0, 0);

#pragma region Camera&Object Movement
	//	Movement amount per frame
	float negMove = -(0.0025f * (float)time);
	float posMove = 0.0025f * (float)time;

	float negRotate = -(0.0025f * (float)time);
	float posRotate = 0.0025f * (float)time;

		//	DIK_ & 0x80	
#pragma endregion

	return;
}

bool GraphicsProject::loadOBJ(const char* path){

	std::vector<unsigned int> posIndicies, uvIndicies, normIndicies;
	std::vector<FLOAT3> tmp_Pos;
	std::vector<FLOAT2> tmp_Uvs;
	std::vector<FLOAT3> tmp_Norms;

	FILE* file = fopen(path, "r");
	if (file == NULL){
		printf("Impossible to open!\n");
		return false;
	}

	while (true){
		char lineHeader[128];

		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		//	pos
		if (strcmp(lineHeader, "v") == 0){
			FLOAT3 f3;
			fscanf(file, "%f %f %f\n", &f3.x, &f3.y, &f3.z);
			tmp_Pos.push_back(f3);
		}
		//	uvs
		else if (strcmp(lineHeader, "vt") == 0){
			FLOAT2 uv;
			fscanf(file, "%f %f\n", &uv.u, &uv.v);
			tmp_Uvs.push_back(uv);
		}
		//	normals
		else if (strcmp(lineHeader, "vn") == 0){
			FLOAT3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			tmp_Norms.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9){
				printf("Cannot be read properly!");
				return false;
			}
			posIndicies.push_back(vertexIndex[0]);
			posIndicies.push_back(vertexIndex[1]);
			posIndicies.push_back(vertexIndex[2]);
			uvIndicies.push_back(uvIndex[0]);
			uvIndicies.push_back(uvIndex[1]);
			uvIndicies.push_back(uvIndex[2]);
			normIndicies.push_back(normalIndex[0]);
			normIndicies.push_back(normalIndex[1]);
			normIndicies.push_back(normalIndex[2]);
		}
	}

	for (unsigned int i = 0; i < posIndicies.size(); ++i){
		Vert temp;
		temp.Pos = tmp_Pos[posIndicies[i] - 1];
		temp.Uvs = tmp_Uvs[uvIndicies[i] - 1];
		temp.Norms = tmp_Norms[normIndicies[i] - 1];

		m_model.interleaved.push_back(temp);
		m_model.out_Indicies.push_back(i);
	}

#pragma region Create Resources
	HRESULT result;

	D3D11_BUFFER_DESC vbuffdesc;
	ZeroMemory(&vbuffdesc, sizeof(D3D11_BUFFER_DESC));
	vbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbuffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbuffdesc.ByteWidth = sizeof(Vert) * m_model.interleaved.size();

	D3D11_SUBRESOURCE_DATA vSubdata;
	ZeroMemory(&vSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
	vSubdata.pSysMem = &m_model.interleaved[0];

	result = device->CreateBuffer(&vbuffdesc, &vSubdata, &vBuffer_Model);
	if (result != S_OK)
		return false;

	D3D11_BUFFER_DESC iBuffdesc;
	ZeroMemory(&iBuffdesc, sizeof(D3D11_BUFFER_DESC));
	iBuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	iBuffdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBuffdesc.ByteWidth = sizeof(unsigned int) * m_model.out_Indicies.size();

	D3D11_SUBRESOURCE_DATA iSubdata;
	ZeroMemory(&iSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
	iSubdata.pSysMem = &m_model.out_Indicies[0];

	result = device->CreateBuffer(&iBuffdesc, &iSubdata, &iBuffer_Model);
	if (result != S_OK)
		return false;
#pragma endregion

	return true;
}

bool GraphicsProject::ShutDown() {

	swapChain->Release();
	device->Release();
	devContext->Release();
	rtView->Release();

	vertLayout->Release();

	cbPerObjectBuffer->Release();
	iBuffer_Cube->Release();
	vBuffer_Cube->Release();

	vBuffer_Model->Release();
	iBuffer_Model->Release();

	VS->Release();
	PS->Release();

	VS_Buffer->Release();
	PS_Buffer->Release();

	CubeTexture->Release();
	ssCube->Release();

	dsBuffer->Release();
	dsView->Release();

	rState_B_AA->Release();
	rState_B->Release();
	rState_F_AA->Release();
	rState_F->Release();
	rState_Wire->Release();
	
	DIKeyboard->Release();
	DIMouse->Release();

	pApp = nullptr;

	UnregisterClass(L"DirectXApplication", application);
	return true;
}



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int) {
	srand(unsigned int(time(0)));
	GraphicsProject myApp(hInstance, (WNDPROC)WndProc);
	myApp.InitScene();
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Update()) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message) {
	case (WM_DESTROY) : { PostQuitMessage(0); }
	case (WM_SIZE) : {
		if (pApp)
			pApp->ResizeWin();
		break;
	}
					 break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}