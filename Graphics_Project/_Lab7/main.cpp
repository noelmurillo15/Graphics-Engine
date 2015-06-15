#include "MathFunc.h"
#include "TimerClass.h"
#include "FPSClass.h"
#include "CPUClass.h"

#include "Cube.h"
#include "stars.h"

#include <ctime>
#include <string>

#include "PS_Skybox.csh"
#include "VS_Skybox.csh"

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
	ID3D11Buffer*			cbPerFrameBuffer = nullptr;
		
	ID3D11VertexShader*		VS = nullptr;
	ID3D11PixelShader*		PS = nullptr;
	ID3D11PixelShader*		PS_D2D = nullptr;

	ID3D10Blob*				VS_Buffer = nullptr;
	ID3D10Blob*				PS_Buffer = nullptr;
	ID3D10Blob*				PS_D2D_Buffer = nullptr;

	ID3D11Texture2D*		dsBuffer = nullptr;
	ID3D11DepthStencilView* dsView = nullptr;
	
	ID3D11RasterizerState*	rState_B_AA = nullptr;
	ID3D11RasterizerState*	rState_B = nullptr;
	ID3D11RasterizerState*	rState_F_AA = nullptr;
	ID3D11RasterizerState*	rState_F = nullptr;
	ID3D11RasterizerState*	rState_Wire = nullptr;

	//	Cube
	ID3D11Buffer*			iBuffer_Cube = nullptr;
	ID3D11Buffer*			vBuffer_Cube = nullptr;

	ID3D11ShaderResourceView* CubeTexture = nullptr;
	ID3D11ShaderResourceView* TransparentCubeTexture = nullptr;
	ID3D11SamplerState*		ssCube = nullptr;
	ID3D11BlendState*		textureBlendState = nullptr;

	//	Ground
	ID3D11Buffer*			iBuffer_Ground = nullptr;
	ID3D11Buffer*			vBuffer_Ground = nullptr;

	ID3D11ShaderResourceView* GroundTexture = nullptr;

	//	Model Loading
	ID3D11Buffer*			iBuffer_Model = nullptr;
	ID3D11Buffer*			vBuffer_Model = nullptr;

	//	Skybox
	ID3D11InputLayout*		skyboxLayout = nullptr;

	ID3D11Buffer*			vBuffer_Skybox = nullptr;
	ID3D11Buffer*			iBuffer_Skybox = nullptr;

	ID3D11VertexShader*		vShader_Skybox = nullptr;
	ID3D11PixelShader*		pShader_Skybox = nullptr;

	ID3D11SamplerState*		ssSkybox = nullptr;

	ID3D11Texture2D*		skyboxTexture = nullptr;
	ID3D11ShaderResourceView* srvSkybox = nullptr;

	ID3D11DepthStencilState*dsState = nullptr;

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
	cbPerObject		cbPerObj;
	cbPerFrame		constbuffPerFrame;
	Model			m_model;
	Light			light;

	MATRIX4X4		WVP;
	MATRIX4X4		cube1World;
	MATRIX4X4		cube2World;
	MATRIX4X4		modelWorld;
	MATRIX4X4		skyboxWorld;
	MATRIX4X4		groundWorld;

	MATRIX4X4		camView;
	MATRIX4X4		camProjection;
	FLOAT4			camPosition;
	FLOAT4			camTarget;
	FLOAT4			camUp;

	float			rot = 0.01f;

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
		//	Cube
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthDesc.Width = BUFFER_WIDTH;
	depthDesc.Height = BUFFER_HEIGHT;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.SampleDesc.Count = 1;

		//	Skybox
	D3D11_SUBRESOURCE_DATA stencilSubdata_skybox;
	stencilSubdata_skybox.pSysMem = Cube_indicies;
	stencilSubdata_skybox.SysMemPitch = sizeof(const unsigned int);
	stencilSubdata_skybox.SysMemSlicePitch = sizeof(Cube_indicies);

	result = device->CreateTexture2D(&depthDesc, &stencilSubdata_skybox, &dsBuffer);
	result = device->CreateDepthStencilView(dsBuffer, 0, &dsView);

		//	DS State
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = device->CreateDepthStencilState(&dsDesc, &dsState);
#pragma endregion

}

bool GraphicsProject::InitScene(){

	HRESULT result;

#pragma region Compile .fx Shaders
	result = D3DX11CompileFromFile(L"ShaderData.fx", 0, 0, "main", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
	result = D3DX11CompileFromFile(L"ShaderData.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);
	result = D3DX11CompileFromFile(L"ShaderData.fx", 0, 0, "PS_D2D", "ps_4_0", 0, 0, 0, &PS_D2D_Buffer, 0, 0);

	result = device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);	
	result = device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);	
	result = device->CreatePixelShader(PS_D2D_Buffer->GetBufferPointer(), PS_D2D_Buffer->GetBufferSize(), NULL, &PS_D2D);

	result = device->CreateVertexShader(VS_Skybox, sizeof(VS_Skybox), NULL, &vShader_Skybox);
	result = device->CreatePixelShader(PS_Skybox, sizeof(PS_Skybox), NULL, &pShader_Skybox);
#pragma endregion

#pragma region Load Model
	loadOBJ("Link_tri.obj");
#pragma endregion

#pragma region Cube Setup
	VERTEX Cube[] =
	{
		// Front Face
		VERTEX(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f),
		VERTEX(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f,  1.0f, -1.0f),
		VERTEX( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f,  1.0f,  1.0f, -1.0f),
		VERTEX( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  1.0f, -1.0f, -1.0f),

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

#pragma region Ground Setup
	VERTEX Ground[] =
	{
		VERTEX(-1.0f, -1.0f, -1.0f, 100.0f, 100.0f, 0.0f, 1.0f, 0.0f),
		VERTEX( 1.0f, -1.0f, -1.0f,   0.0f, 100.0f, 0.0f, 1.0f, 0.0f),
		VERTEX( 1.0f, -1.0f,  1.0f,   0.0f,   0.0f, 0.0f, 1.0f, 0.0f),
		VERTEX(-1.0f, -1.0f,  1.0f, 100.0f,   0.0f, 0.0f, 1.0f, 0.0f),
	};

	UINT iGround[] = {
		0, 1, 2,
		0, 2, 3,
	};
#pragma endregion

#pragma region Skybox Setup
	skyboxWorld = Identity();
	skyboxWorld = Scale_4x4(skyboxWorld, 20.0f, 20.0f, 20.0f); // skybox EXPAND
	skyboxWorld = Translate(skyboxWorld, 0.0f, -10.0f, 0.0f);
#pragma endregion

#pragma region Cam Setup
	camPosition = FLOAT4(0.0f, 3.0f, -8.0f, 0.0f);
	camTarget = FLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = FLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	camView = CreateViewMatrix(camPosition, camTarget, camUp);

	unsigned int aspect = (float)BUFFER_WIDTH / BUFFER_HEIGHT;
	camProjection = CreateProjectionMatrix(100.0f, 0.1f, 72, aspect);
#pragma endregion

#pragma region Light Setup
	light.direction = FLOAT3(0.0f, -1.0f, 0.0f);
	light.ambientColor = FLOAT4(0.4f, 0.3f, 0.4f, 1.0f);
	light.diffuse = FLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
#pragma endregion

#pragma region IndexBuffer
		//	Cube
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * 12 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA iinitData;
	ZeroMemory(&iinitData, sizeof(D3D11_SUBRESOURCE_DATA));
	iinitData.pSysMem = iCube;

	result = device->CreateBuffer(&indexBufferDesc, &iinitData, &iBuffer_Cube);

		//	Ground
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.ByteWidth = sizeof(UINT) * 6;
	ZeroMemory(&iinitData, sizeof(D3D11_SUBRESOURCE_DATA));
	iinitData.pSysMem = iGround;

	result = device->CreateBuffer(&indexBufferDesc, &iinitData, &iBuffer_Ground);
		//	Skybox
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.ByteWidth = sizeof(const unsigned int) * 1692;
	ZeroMemory(&iinitData, sizeof(D3D11_SUBRESOURCE_DATA));
	iinitData.pSysMem = Cube_indicies;

	result = device->CreateBuffer(&indexBufferDesc, &iinitData, &iBuffer_Skybox);
#pragma endregion

#pragma region VertexBuffer
	//	Cubes
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(VERTEX) * 24;

	D3D11_SUBRESOURCE_DATA vertBufferData;
	ZeroMemory(&vertBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertBufferData.pSysMem = Cube;

	result = device->CreateBuffer(&vertexBufferDesc, &vertBufferData, &vBuffer_Cube);

	//	Skybox
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(OBJ_VERT) * 776;
	ZeroMemory(&vertBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertBufferData.pSysMem = &Cube_data;

	result = device->CreateBuffer(&vertexBufferDesc, &vertBufferData, &vBuffer_Skybox);

	//	Ground
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(VERTEX) * 4;
	ZeroMemory(&vertBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertBufferData.pSysMem = Ground;

	result = device->CreateBuffer(&vertexBufferDesc, &vertBufferData, &vBuffer_Ground);
#pragma endregion

#pragma region InputLayer
		//	VS
	D3D11_INPUT_ELEMENT_DESC layout[3];
	layout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	UINT arrSize = ARRAYSIZE(layout);
	result = device->CreateInputLayout(layout, arrSize, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);

		//	VS_Skybox
	D3D11_INPUT_ELEMENT_DESC vLayout_skybox[2];
	arrSize = ARRAYSIZE(vLayout_skybox);
	vLayout_skybox[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	vLayout_skybox[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	result = device->CreateInputLayout(vLayout_skybox, arrSize, VS_Skybox, sizeof(VS_Skybox), &skyboxLayout);
#pragma endregion

#pragma region Viewport
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.Height = BUFFER_HEIGHT;
	viewport.Width = BUFFER_WIDTH;
	viewport.MaxDepth = 1.0f;
#pragma endregion

#pragma region ConstBuffer
		//	per object
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	result = device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

		//	 per frame
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.ByteWidth = sizeof(cbPerFrame);

	result = device->CreateBuffer(&cbbd, NULL, &cbPerFrameBuffer);
#pragma endregion

#pragma region Load Textures
	//	Cube
	result = D3DX11CreateShaderResourceViewFromFile(device, L"TransparentGlass.png", NULL, NULL, &TransparentCubeTexture, NULL);
	result = D3DX11CreateShaderResourceViewFromFile(device, L"grass.jpg", NULL, NULL, &CubeTexture, NULL);

	//	Ground
	result = D3DX11CreateShaderResourceViewFromFile(device, L"spaceGround.jpg", NULL, NULL, &GroundTexture, NULL);

	//	skybox
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.Width = stars_width;
	textureDesc.Height = stars_height;
	textureDesc.MipLevels = stars_numlevels;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;

	D3D11_SUBRESOURCE_DATA textureSubdata[stars_numlevels];
	ZeroMemory(&textureSubdata, sizeof(D3D11_SUBRESOURCE_DATA));

	for (int i = 0; i < stars_numlevels; i++){
		textureSubdata[i].pSysMem = &stars_pixels[stars_leveloffsets[i]];
		textureSubdata[i].SysMemPitch = sizeof(unsigned int) * (stars_width >> i);
	}
	result = device->CreateTexture2D(&textureDesc, textureSubdata, &skyboxTexture);
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

	ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MinLOD = (-FLT_MAX);
	sampDesc.MaxLOD = (FLT_MAX);
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	result = device->CreateSamplerState(&sampDesc, &ssSkybox);
#pragma endregion

#pragma region Blend State
	D3D11_BLEND_DESC blendDesc;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	result = device->CreateBlendState(&blendDesc, &textureBlendState);
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

	rot += timeTracker.GetTime();
	if (rot > 6.26f)
		rot = 0.0f;

		//	Update Skybox Texture
	if (srvSkybox) {
		srvSkybox->Release();
		srvSkybox = nullptr;
	}
	HRESULT result = device->CreateShaderResourceView(skyboxTexture, NULL, &srvSkybox);	//	Shadr res view

		//	Reset 
	WVP = Identity();
	cbPerObj.World = WVP;
	cube1World = Identity();
	cube2World = Identity();
	modelWorld = Identity();
	groundWorld = Identity();

		//	Define
	modelWorld = RotateZ(modelWorld, rot);
	modelWorld = Translate(modelWorld, 0.0f, -0.8f, 6.0f);
	modelWorld = Scale_4x4(modelWorld, 0.4f, 0.4f, 0.4f);

	cube1World = Translate(cube1World, 5.0f, 0.0f, 3.0f);
	cube1World = RotateZ(cube1World, rot);

	cube2World = RotateX(cube2World, rot);
	cube2World = Scale_4x4(cube2World, 2.0f, 2.0f, 2.0f);

	groundWorld = Translate(groundWorld, 0.0f, 0.0f, 0.0f);
	groundWorld = Scale_4x4(groundWorld, 20.0f, 1.0f, 20.0f);

	Render();

	return true;
}

void GraphicsProject::Render(){

	//	Background Color
	FLOAT RGBA[4]; RGBA[0] = 0; RGBA[1] = 0; RGBA[2] = 0; RGBA[3] = 1;
	UINT stride, offset;

	//	Clear views
	devContext->ClearRenderTargetView(rtView, RGBA);
	devContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	devContext->OMSetDepthStencilState(dsState, 1);
	devContext->OMSetRenderTargets(1, &rtView, dsView);
	devContext->RSSetViewports(1, &viewport);

	//	Update Lights
	constbuffPerFrame.light = light;
	devContext->UpdateSubresource(cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0);
	devContext->PSSetConstantBuffers(0, 1, &cbPerFrameBuffer);

	//	Infinite Skybox
	stride = sizeof(OBJ_VERT);
	offset = 0;

	WVP = Mult_4x4(skyboxWorld, camProjection);	//	dont mult with camView for infinite skybox
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	devContext->RSSetState(rState_F);
	devContext->IASetVertexBuffers(0, 1, &vBuffer_Skybox, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Skybox, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(vShader_Skybox, NULL, 0);
	devContext->PSSetShader(pShader_Skybox, NULL, 0);
	devContext->IASetInputLayout(skyboxLayout);
	devContext->PSSetShaderResources(0, 1, &srvSkybox);
	devContext->PSSetSamplers(0, 1, &ssSkybox);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->DrawIndexed(1692, 0, 0);

	devContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	//	Ground
	WVP = Mult_4x4(groundWorld, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (Transpose_4x4(groundWorld));
	cbPerObj.WVP = WVP;

	stride = sizeof(VERTEX);
	devContext->UpdateSubresource(cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0);
	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	devContext->IASetVertexBuffers(0, 1, &vBuffer_Ground, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Ground, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(VS, NULL, 0);
	devContext->PSSetShader(PS, NULL, 0);
	devContext->IASetInputLayout(vertLayout);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->PSSetShaderResources(0, 1, &GroundTexture);
	devContext->PSSetSamplers(0, 1, &ssCube);
	devContext->DrawIndexed(6, 0, 0);

	//	Link Model
	WVP = Mult_4x4(modelWorld, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (Transpose_4x4(modelWorld));
	cbPerObj.WVP = WVP;

	stride = sizeof(Vert);
	offset = 0;

	devContext->UpdateSubresource(cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0);
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
	cbPerObj.World = (Transpose_4x4(cube1World));
	cbPerObj.WVP = WVP;

	stride = sizeof(VERTEX);	
	devContext->UpdateSubresource(cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0);
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
	cbPerObj.World = (Transpose_4x4(cube2World));
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0);
	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	devContext->RSSetState(rState_F);
	devContext->OMSetBlendState(textureBlendState, NULL, 0xffffffff);
	devContext->IASetVertexBuffers(0, 1, &vBuffer_Cube, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(VS, NULL, 0);
	devContext->PSSetShader(PS, NULL, 0);
	devContext->IASetInputLayout(vertLayout);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->PSSetShaderResources(0, 1, &TransparentCubeTexture);
	devContext->PSSetSamplers(0, 1, &ssCube);
	devContext->DrawIndexed(36, 0, 0);

	devContext->RSSetState(rState_B);
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
	if (pApp->swapChain == NULL || pApp->devContext == NULL || pApp->device == NULL)
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
	dsBuffer = nullptr;
	dsView->Release();
	dsView = nullptr;
	dsState->Release();

	//	resize
	hr = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

	//	Create new rtv
	ID3D11Resource* pBB;
	swapChain->GetBuffer(0, __uuidof(pBB),
		reinterpret_cast<void**>(&pBB));

	hr = device->CreateRenderTargetView(pBB, NULL, &rtView);
	pBB->Release();

	//	Create new depth stencil
	D3D11_TEXTURE2D_DESC depthBufferdesc;
	depthBufferdesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferdesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthBufferdesc.Width = BUFFER_WIDTH;
	depthBufferdesc.Height = BUFFER_HEIGHT;
	depthBufferdesc.MipLevels = 1;
	depthBufferdesc.ArraySize = 1;
	depthBufferdesc.SampleDesc.Count = 1;
	depthBufferdesc.SampleDesc.Quality = 0;
	depthBufferdesc.CPUAccessFlags = 0;
	depthBufferdesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA stenilSubres;
	stenilSubres.pSysMem = Cube_indicies;
	stenilSubres.SysMemPitch = sizeof(const unsigned int);
	stenilSubres.SysMemSlicePitch = sizeof(Cube_indicies);

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateTexture2D(&depthBufferdesc, &stenilSubres, &dsBuffer);	//	Depth Stencil
	hr = device->CreateDepthStencilState(&dsDesc, &dsState);	//	Stencil state
	hr = device->CreateDepthStencilView(dsBuffer, 0, &dsView);	//	Stencil view

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

	if (keyboardState[DIK_ESCAPE] & 0x80){
		int x = 0;
		PostMessage(pApp->window, WM_DESTROY, 0, 0);
	}

#pragma region Camera&Object Movement
	//	Movement amount per frame
	float negMove = -(5.0f * (float)time);
	float posMove = 5.0f * (float)time;

	float negRotate = -(5.0f * (float)time);
	float posRotate = 5.0f * (float)time;

		//	DIK_ & 0x80	
	//	cam Movement
	if (keyboardState[DIK_W] & 0x80)
		camView = Translate(camView, 0.0f, 0.0f, negMove);

	if (keyboardState[DIK_S] & 0x80)
		camView = Translate(camView, 0.0f, 0.0f, posMove);

	if (keyboardState[DIK_A] & 0x80)
		camView = Translate(camView, posMove, 0.0f, 0.0f);

	if (keyboardState[DIK_D] & 0x80)
		camView = Translate(camView, negMove, 0.0f, 0.0f);

	//	cam Fly / Ground
	if (keyboardState[DIK_F] & 0x80)
		camView = Translate(camView, 0.0f, negMove, 0.0f);

	if (keyboardState[DIK_G] & 0x80)
		camView = Translate(camView, 0.0f, posMove, 0.0f);

	//	cam Rotation
	if (keyboardState[DIK_UPARROW] & 0x80){
		camView = RotateX(camView, negRotate);
		skyboxWorld = RotateX(skyboxWorld, negRotate);
	}

	if (keyboardState[DIK_DOWNARROW] & 0x80){
		camView = RotateX(camView, posRotate);
		skyboxWorld = RotateX(skyboxWorld, posRotate);
	}

	if (keyboardState[DIK_LEFTARROW] & 0x80){
		camView = RotateY(camView, negRotate);
		skyboxWorld = RotateY(skyboxWorld, negRotate);
	}

	if (keyboardState[DIK_RIGHTARROW] & 0x80){
		camView = RotateY(camView, posRotate);
		skyboxWorld = RotateY(skyboxWorld, posRotate);
	}

	if (keyboardState[DIK_M] & 0x80){
		skyboxWorld = Identity();
		skyboxWorld = Scale_4x4(skyboxWorld, 20.0f, 20.0f, 20.0f); // skybox EXPAND
		skyboxWorld = Translate(skyboxWorld, 0.0f, -10.0f, 0.0f);

		camPosition = FLOAT4(0.0f, 3.0f, -8.0f, 0.0f);
		camTarget = FLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		camUp = FLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

		camView = CreateViewMatrix(camPosition, camTarget, camUp);

		float ar = abs(w / h);
		camProjection = CreateProjectionMatrix(100.0f, 0.1f, 72, ar);
	}
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
	cbPerFrameBuffer->Release();

	iBuffer_Cube->Release();
	vBuffer_Cube->Release();

	vBuffer_Model->Release();
	iBuffer_Model->Release();

	iBuffer_Ground->Release();
	vBuffer_Ground->Release();

	GroundTexture->Release();

	VS->Release();
	PS->Release();
	PS_D2D->Release();

	VS_Buffer->Release();
	PS_Buffer->Release();
	PS_D2D_Buffer->Release();

	CubeTexture->Release();
	TransparentCubeTexture->Release();
	ssCube->Release();

	textureBlendState->Release();

	dsBuffer->Release();
	dsView->Release();
	dsState->Release();

	skyboxLayout->Release();

	vBuffer_Skybox->Release();
	iBuffer_Skybox->Release();

	vShader_Skybox->Release();
	pShader_Skybox->Release();

	ssSkybox->Release();

	srvSkybox->Release();
	skyboxTexture->Release();

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