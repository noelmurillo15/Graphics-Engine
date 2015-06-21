#include "MathFunc.h"
#include "TimerClass.h"
#include "FPSClass.h"
#include "CPUClass.h"
#include "DDSTextureLoader.h"

#include <ctime>
#include <string>
#include <thread>

#include "VS.csh"
#include "PS.csh"
#include "VS_Star.csh"
#include "PS_Star.csh"
#include "PS_Skybox.csh"
#include "VS_Skybox.csh"

#include <dinput.h>
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

#define BUFFER_WIDTH	1024
#define BUFFER_HEIGHT	768


class GraphicsProject {

public:

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
	ID3D11InputLayout*		starLayout = nullptr;

	ID3D11Texture2D*		dsBuffer = nullptr;
	ID3D11DepthStencilView* dsView = nullptr;
	ID3D11DepthStencilState*dsState = nullptr;

	//	Buffers
	ID3D11Buffer*			cbPerObjectBuffer = nullptr;
	ID3D11Buffer*			cbPerFrameBuffer = nullptr;

	ID3D11Buffer*			vBuffer_Skybox = nullptr;
	ID3D11Buffer*			iBuffer_Skybox = nullptr;

	ID3D11Buffer*			iBuffer_Cube = nullptr;
	ID3D11Buffer*			vBuffer_Cube = nullptr;

	ID3D11Buffer*			iBuffer_Ground = nullptr;
	ID3D11Buffer*			vBuffer_Ground = nullptr;

	ID3D11Buffer*			iBuffer_LinkModel = nullptr;
	ID3D11Buffer*			vBuffer_LinkModel = nullptr;

	ID3D11Buffer*			iBuffer_BarrelModel = nullptr;
	ID3D11Buffer*			vBuffer_BarrelModel = nullptr;

	ID3D11Buffer*			iBuffer_Star = nullptr;
	ID3D11Buffer*			vBuffer_Star = nullptr;

	//	Shaders
	ID3D11VertexShader*		vShader = nullptr;
	ID3D11PixelShader*		pShader = nullptr;

	ID3D11VertexShader*		vShader_Star = nullptr;
	ID3D11PixelShader*		pShader_Star = nullptr;

	ID3D11VertexShader*		vShader_Skybox = nullptr;
	ID3D11PixelShader*		pShader_Skybox = nullptr;
	
	//	Raster States
	ID3D11RasterizerState*	rState_B_AA = nullptr;
	ID3D11RasterizerState*	rState_B = nullptr;
	ID3D11RasterizerState*	rState_F_AA = nullptr;
	ID3D11RasterizerState*	rState_F = nullptr;
	ID3D11RasterizerState*	rState_Wire = nullptr;

	//	Textures
	ID3D11ShaderResourceView* srvSkymap = nullptr;
	ID3D11ShaderResourceView* srvGlass = nullptr;	
	ID3D11ShaderResourceView* srvGrass = nullptr;
	ID3D11ShaderResourceView* srvGround = nullptr;
	ID3D11ShaderResourceView* srvWood = nullptr;

	//	Cube
	ID3D11SamplerState*		ssCube = nullptr;
	ID3D11BlendState*		bsTransparency = nullptr;
	
	//	Skybox
	ID3D11SamplerState*		ssSkybox = nullptr;
	ID3D11InputLayout*		skyboxLayout = nullptr;

	//	Models
	Model*					linkModel = nullptr;
	Model*					barrelModel = nullptr;
	Model*					skyboxModel = nullptr;

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

	Light			light;

	MATRIX4X4		WVP;

	MATRIX4X4		cube1World;
	MATRIX4X4		cube2World;
	MATRIX4X4		cube3World;
	MATRIX4X4		cube4World;

	MATRIX4X4		linkWorld;
	MATRIX4X4		barrelWorld;
	MATRIX4X4		skyboxWorld;
	MATRIX4X4		groundWorld;
	MATRIX4X4		starWorld;

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
	bool Render();
	bool ShutDown();

	void ResizeWin();
	void ChangeTitleBar(std::string _str);

	bool InitDirectInput(HINSTANCE hInstance);
	void DetectInput(double time, float w, float h);
	
	UINT FindNumIndicies(ID3D11Buffer*);
};

GraphicsProject* pApp = nullptr;

bool loadOBJ(const char* path, ID3D11Device* d, Model* m);

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

	result = device->CreateTexture2D(&depthDesc, NULL, &dsBuffer);
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

#pragma region Load Models
	vector<thread> threads;

	linkModel = new Model;
	barrelModel = new Model;
	skyboxModel = new Model;

	threads.push_back(thread(loadOBJ, "Link.obj", pApp->device, linkModel));
	threads.push_back(thread(loadOBJ, "Cube.obj", pApp->device, skyboxModel));
	threads.push_back(thread(loadOBJ, "Barrel.obj", pApp->device, barrelModel));
#pragma endregion

#pragma region Compile .fx Shaders
	result = device->CreateVertexShader(VS, sizeof(VS), NULL, &vShader);
	result = device->CreatePixelShader(PS, sizeof(PS), NULL, &pShader);

	result = device->CreateVertexShader(VS_Star, sizeof(VS_Star), NULL, &vShader_Star);
	result = device->CreatePixelShader(PS_Star, sizeof(PS_Star), NULL, &pShader_Star);

	result = device->CreateVertexShader(VS_Skybox, sizeof(VS_Skybox), NULL, &vShader_Skybox);
	result = device->CreatePixelShader(PS_Skybox, sizeof(PS_Skybox), NULL, &pShader_Skybox);
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

#pragma region Star Setup
	starWorld = Identity();
	starWorld = Translate(starWorld, 0.0f, 6.0f, 0.0f);

	SIMPLE_VERTEX Star[20];
	unsigned int iStar[108];

	Star[0].pos = Star[10].pos = { 0, 1, 0, 0 };
	Star[1].pos = Star[11].pos = { 0.4f, 0.2f, 0, 0 };
	Star[2].pos = Star[12].pos = { 1, 0, 0, 0 };
	Star[3].pos = Star[13].pos = { 0.5f, -0.4f, 0, 0 };
	Star[4].pos = Star[14].pos = { 0.75f, -1, 0, 0 };
	Star[5].pos = Star[15].pos = { 0, -0.65f, 0, 0 };
	Star[6].pos = Star[16].pos = { -0.75f, -1, 0, 0 };
	Star[7].pos = Star[17].pos = { -0.5f, -0.4f, 0, 0 };
	Star[8].pos = Star[18].pos = { -1, 0, 0, 0 };
	Star[9].pos = Star[19].pos = { -0.4f, 0.2f, 0, 0 };

	for (int x = 0; x < 10; ++x){
		Star[x].pos.z = -0.25f;
		Star[10 + x].pos.z = 0.25f;

		Star[x].pos.w = 1;
		Star[10 + x].pos.w = 1;

		// Tips of Star       R  G  B  A
		Star[x].color = { 0, 0, 0, 1 };
		Star[10 + x].color = { 0, 0, 0, 1 };
	}
	//	Inside Star					   R  G  B  A
	Star[11].color = Star[1].color = { 1, 0, 0, 1 };
	Star[13].color = Star[3].color = { 1, 1, 0, 1 };
	Star[15].color = Star[5].color = { 1, 0, 1, 1 };
	Star[17].color = Star[7].color = { 0, 1, 1, 1 };
	Star[19].color = Star[9].color = { 0, 1, 0, 1 };

	// front star     back star
	// clock-wise    counter clock-wise
	iStar[0] = 0;	iStar[24] = 11;
	iStar[1] = 1; 	iStar[25] = 10; //1   
	iStar[2] = 9;	iStar[26] = 19;

	iStar[3] = 1;	iStar[27] = 13;
	iStar[4] = 2;	iStar[28] = 12; //2
	iStar[5] = 3;	iStar[29] = 11;

	iStar[6] = 3;	iStar[30] = 15;
	iStar[7] = 4; 	iStar[31] = 14; //3
	iStar[8] = 5;	iStar[32] = 13;

	iStar[9] = 5;	iStar[33] = 17;
	iStar[10] = 6; 	iStar[34] = 16; //4
	iStar[11] = 7;	iStar[35] = 15;

	iStar[12] = 7;	iStar[36] = 19;
	iStar[13] = 8; 	iStar[37] = 18; //5
	iStar[14] = 9;	iStar[38] = 17;

	iStar[15] = 5;	iStar[39] = 19;
	iStar[16] = 7; 	iStar[40] = 17; //6
	iStar[17] = 9;	iStar[41] = 15;

	iStar[18] = 3;	iStar[42] = 11;
	iStar[19] = 5; 	iStar[43] = 15; //7
	iStar[20] = 1;	iStar[44] = 13;

	iStar[21] = 5;	iStar[45] = 11;
	iStar[22] = 9; 	iStar[46] = 19; //8
	iStar[23] = 1;	iStar[47] = 15;

	//			 in between
	// clockwise        counter clock-wise
	iStar[48] = 10;		iStar[51] = 10;
	iStar[49] = 1;		iStar[52] = 11;
	iStar[50] = 0;		iStar[53] = 1;

	iStar[54] = 10;		iStar[57] = 10;
	iStar[55] = 0;		iStar[58] = 9;
	iStar[56] = 9;		iStar[59] = 19;

	iStar[60] = 17;		iStar[63] = 17;
	iStar[61] = 7;		iStar[64] = 6;
	iStar[62] = 6;		iStar[65] = 16;

	iStar[66] = 18;		iStar[69] = 18;
	iStar[67] = 8;		iStar[70] = 7;
	iStar[68] = 7;		iStar[71] = 17;

	iStar[72] = 19;		iStar[75] = 19;
	iStar[73] = 9;		iStar[76] = 8;
	iStar[74] = 8;		iStar[77] = 18;

	iStar[78] = 1;		iStar[81] = 1;
	iStar[79] = 11;		iStar[82] = 12;
	iStar[80] = 12;		iStar[83] = 2;

	iStar[84] = 3;		iStar[87] = 3;
	iStar[85] = 13;		iStar[88] = 14;
	iStar[86] = 14;		iStar[89] = 4;

	iStar[90] = 2;		iStar[93] = 2;
	iStar[91] = 12;		iStar[94] = 13;
	iStar[92] = 13;		iStar[95] = 3;

	iStar[96] = 15;		iStar[99] = 15;
	iStar[97] = 5;		iStar[100] = 4;
	iStar[98] = 4;		iStar[101] = 14;

	iStar[102] = 5;		iStar[105] = 5;
	iStar[103] = 15;	iStar[106] = 16;
	iStar[104] = 16;	iStar[107] = 6;
#pragma endregion

#pragma region Cam Setup
	camView = Identity();
	skyboxWorld = Identity();

	camPosition = FLOAT4(0.0f, 0.0f, -8.0f, 0.0f);
	camTarget = FLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = FLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	camView = CreateViewMatrix(camPosition, camTarget, camUp);
	skyboxWorld = Translate(skyboxWorld, camPosition.x, camPosition.y, camPosition.z);

	unsigned int aspect = (float)BUFFER_WIDTH / BUFFER_HEIGHT;
	camProjection = CreateProjectionMatrix(100.0f, 0.1f, 72, aspect);
#pragma endregion
	
#pragma region Light Setup
	light.direction = FLOAT3(0.0f, -1.0f, 0.0f);
	light.ambientColor = FLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

	light.position = FLOAT3(0.0f, 0.0f, 0.0f);
	light.range = 50.0f;
#pragma endregion

#pragma region InputLayer
	//	VS
	D3D11_INPUT_ELEMENT_DESC layout[3];
	layout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	UINT arrSize = ARRAYSIZE(layout);
	result = device->CreateInputLayout(layout, arrSize, VS, sizeof(VS), &vertLayout);

	//	VS_Skybox
	D3D11_INPUT_ELEMENT_DESC vLayout_skybox[2];
	arrSize = ARRAYSIZE(vLayout_skybox);
	vLayout_skybox[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	vLayout_skybox[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	result = device->CreateInputLayout(vLayout_skybox, arrSize, VS_Skybox, sizeof(VS_Skybox), &skyboxLayout);

	//	VS_Star
	D3D11_INPUT_ELEMENT_DESC vLayout_Star[2];
	arrSize = ARRAYSIZE(vLayout_Star);
	vLayout_Star[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	vLayout_Star[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	result = device->CreateInputLayout(vLayout_Star, arrSize, VS_Star, sizeof(VS_Star), &starLayout);
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

	//	Star
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * 108;
	ZeroMemory(&iinitData, sizeof(D3D11_SUBRESOURCE_DATA));
	iinitData.pSysMem = iStar;

	result = device->CreateBuffer(&indexBufferDesc, &iinitData, &iBuffer_Star);
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

	//	Ground
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(VERTEX) * 4;
	ZeroMemory(&vertBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertBufferData.pSysMem = Ground;

	result = device->CreateBuffer(&vertexBufferDesc, &vertBufferData, &vBuffer_Ground);

	//	Star
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * 20;
	ZeroMemory(&vertBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertBufferData.pSysMem = Star;

	result = device->CreateBuffer(&vertexBufferDesc, &vertBufferData, &vBuffer_Star);
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
	result = CreateDDSTextureFromFile(device, L"_skymap.dds", NULL, &srvSkymap, NULL);
	result = CreateDDSTextureFromFile(device, L"_grass.dds", NULL, &srvGrass, NULL);
	result = CreateDDSTextureFromFile(device, L"_glass.dds", NULL, &srvGlass, NULL);
	result = CreateDDSTextureFromFile(device, L"_ground.dds", NULL, &srvGround, NULL);
	result = CreateDDSTextureFromFile(device, L"_wood.dds", NULL, &srvWood, NULL);
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
	sampDesc.MaxAnisotropy = 1;

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
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = TRUE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_BLEND_FACTOR;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;	
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = device->CreateBlendState(&blendDesc, &bsTransparency);
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

#pragma region Create Model Resources
		//	Wait for threads to finish and join em
	for (int i = 0; i < threads.size(); ++i)
		threads.at(i).join();

	D3D11_BUFFER_DESC vbuffdesc;
	D3D11_SUBRESOURCE_DATA vSubdata;
	D3D11_BUFFER_DESC iBuffdesc;
	D3D11_SUBRESOURCE_DATA iSubdata;

	//	Link
	ZeroMemory(&vbuffdesc, sizeof(D3D11_BUFFER_DESC));
	vbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbuffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbuffdesc.ByteWidth = sizeof(Vert) * linkModel->interleaved.size();
	ZeroMemory(&vSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
	vSubdata.pSysMem = &linkModel->interleaved[0];
	result = device->CreateBuffer(&vbuffdesc, &vSubdata, &vBuffer_LinkModel);

	ZeroMemory(&iBuffdesc, sizeof(D3D11_BUFFER_DESC));
	iBuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	iBuffdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBuffdesc.ByteWidth = sizeof(unsigned int) * linkModel->out_Indicies.size();
	ZeroMemory(&iSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
	iSubdata.pSysMem = &(linkModel->out_Indicies[0]);
	result = device->CreateBuffer(&iBuffdesc, &iSubdata, &iBuffer_LinkModel);

	//	Cube
	ZeroMemory(&vbuffdesc, sizeof(D3D11_BUFFER_DESC));
	vbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbuffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbuffdesc.ByteWidth = sizeof(Vert) * skyboxModel->interleaved.size();
	ZeroMemory(&vSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
	vSubdata.pSysMem = &skyboxModel->interleaved[0];
	result = device->CreateBuffer(&vbuffdesc, &vSubdata, &vBuffer_Skybox);

	ZeroMemory(&iBuffdesc, sizeof(D3D11_BUFFER_DESC));
	iBuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	iBuffdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBuffdesc.ByteWidth = sizeof(unsigned int) * skyboxModel->out_Indicies.size();
	ZeroMemory(&iSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
	iSubdata.pSysMem = &(skyboxModel->out_Indicies[0]);
	result = device->CreateBuffer(&iBuffdesc, &iSubdata, &iBuffer_Skybox);

	//	Barrel
	ZeroMemory(&vbuffdesc, sizeof(D3D11_BUFFER_DESC));
	vbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbuffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbuffdesc.ByteWidth = sizeof(Vert) * barrelModel->interleaved.size();
	ZeroMemory(&vSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
	vSubdata.pSysMem = &barrelModel->interleaved[0];
	result = device->CreateBuffer(&vbuffdesc, &vSubdata, &vBuffer_BarrelModel);

	ZeroMemory(&iBuffdesc, sizeof(D3D11_BUFFER_DESC));
	iBuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	iBuffdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBuffdesc.ByteWidth = sizeof(unsigned int) * barrelModel->out_Indicies.size();
	ZeroMemory(&iSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
	iSubdata.pSysMem = &(barrelModel->out_Indicies[0]);
	result = device->CreateBuffer(&iBuffdesc, &iSubdata, &iBuffer_BarrelModel);
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

	rot += timeTracker.GetTime();
	if (rot > 6.26f)
		rot = 0.0f;		
#pragma endregion

#pragma region Reset Worlds
	WVP = Identity();
	cbPerObj.World = WVP;	
	linkWorld = Identity();
	cube1World = Identity();
	cube2World = Identity();
	cube3World = Identity();
	cube4World = Identity();
	groundWorld = Identity();
	barrelWorld = Identity();
#pragma endregion

#pragma region Define Worlds
	linkWorld = RotateZ(linkWorld, rot);
	linkWorld = Translate(linkWorld, 0.0f, -0.8f, 15.0f);
	linkWorld = Scale_4x4(linkWorld, 0.25f, 0.25f, 0.25f);

	barrelWorld = RotateZ(barrelWorld, -rot);
	barrelWorld = Translate(barrelWorld, 5.0f, -0.8f, 15.0f);
	barrelWorld = Scale_4x4(barrelWorld, 0.0025f, 0.0025f, 0.0025f);

	cube1World = Translate(cube1World, 5.0f, 0.0f, 3.0f);
	cube1World = RotateZ(cube1World, rot);

	cube2World = RotateX(cube2World, rot);
	cube2World = Scale_4x4(cube2World, 0.8f, 0.8f, 0.8f);

	cube3World = RotateX(cube3World, -rot);
	cube3World = Translate(cube3World, 0.0f, 0.0f, 3.0f);
	cube3World = Scale_4x4(cube3World, 1.4f, 1.4f, 1.4f);

	cube4World = RotateX(cube4World, rot);
	cube4World = Translate(cube4World, 0.0f, 0.0f, 10.0f);
	cube4World = Scale_4x4(cube4World, 2.0f, 2.0f, 2.0f);

	groundWorld = Translate(groundWorld, 0.0f, 0.0f, 0.0f);
	groundWorld = Scale_4x4(groundWorld, 20.0f, 1.0f, 20.0f);
#pragma endregion

#pragma region Reset Light	
	DirectX::XMMATRIX temp = XMConverter(starWorld);
	light.position.x = temp.r[3].m128_f32[0];
	light.position.y = temp.r[3].m128_f32[1];
	light.position.z = temp.r[3].m128_f32[2];
#pragma endregion

	return Render();
}

bool GraphicsProject::Render(){

	UINT stride, offset;
	HRESULT result;

	//	Background Color
	FLOAT RGBA[4]; RGBA[0] = 0; RGBA[1] = 0; RGBA[2] = 1; RGBA[3] = 1;

	//	Blend Factor
	float blendFactor[] = { 0.25f, 0.25f, 0.25f, 0.25f };

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

#pragma region Draw Skybox
	stride = sizeof(Vert);
	offset = 0;

	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (skyboxWorld);
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	devContext->IASetVertexBuffers(0, 1, &vBuffer_Skybox, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Skybox, DXGI_FORMAT_R32_UINT, 0);

	devContext->IASetInputLayout(skyboxLayout);
	devContext->VSSetShader(vShader_Skybox, NULL, 0);
	devContext->PSSetShader(pShader_Skybox, NULL, 0);

	devContext->OMSetBlendState(0, 0, 0xffffffff);
	devContext->PSSetShaderResources(0, 1, &srvSkymap);
	devContext->PSSetSamplers(0, 1, &ssSkybox);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devContext->RSSetState(rState_F);
	devContext->DrawIndexed(FindNumIndicies(iBuffer_Skybox), 0, 0);

	devContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
#pragma endregion

#pragma region Draw Star
	stride = sizeof(SIMPLE_VERTEX);

	WVP = Mult_4x4(starWorld, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (starWorld);
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, NULL, NULL, &cbPerObj, NULL, NULL);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	devContext->IASetVertexBuffers(0, 1, &vBuffer_Star, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Star, DXGI_FORMAT_R32_UINT, 0);

	devContext->IASetInputLayout(starLayout);
	devContext->VSSetShader(vShader_Star, NULL, NULL);
	devContext->PSSetShader(pShader_Star, NULL, NULL);

	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devContext->RSSetState(rState_Wire);
	devContext->DrawIndexed(FindNumIndicies(iBuffer_Star), 0, 0);
#pragma endregion

#pragma region Draw Ground
	stride = sizeof(VERTEX);

	WVP = Mult_4x4(groundWorld, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (groundWorld);
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	devContext->IASetVertexBuffers(0, 1, &vBuffer_Ground, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Ground, DXGI_FORMAT_R32_UINT, 0);

	devContext->IASetInputLayout(vertLayout);
	devContext->VSSetShader(vShader, NULL, 0);
	devContext->PSSetShader(pShader, NULL, 0);

	devContext->PSSetShaderResources(0, 1, &srvGround);
	devContext->PSSetSamplers(0, 1, &ssSkybox);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devContext->RSSetState(rState_F);
	devContext->DrawIndexed(FindNumIndicies(iBuffer_Ground), 0, 0);
#pragma endregion

#pragma region Draw Link
	stride = sizeof(Vert);

	WVP = Mult_4x4(linkWorld, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (linkWorld);
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	devContext->IASetVertexBuffers(0, 1, &vBuffer_LinkModel, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_LinkModel, DXGI_FORMAT_R32_UINT, 0);

	devContext->IASetInputLayout(vertLayout);
	devContext->VSSetShader(vShader, NULL, 0);
	devContext->PSSetShader(pShader, NULL, 0);

	devContext->RSSetState(rState_B);
	devContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	devContext->DrawIndexed(FindNumIndicies(iBuffer_LinkModel), 0, 0);
#pragma endregion

#pragma region Draw Barrel
	stride = sizeof(Vert);

	WVP = Mult_4x4(barrelWorld, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (barrelWorld);
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	devContext->IASetVertexBuffers(0, 1, &vBuffer_BarrelModel, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_BarrelModel, DXGI_FORMAT_R32_UINT, 0);

	devContext->IASetInputLayout(vertLayout);
	devContext->VSSetShader(vShader, NULL, 0);
	devContext->PSSetShader(pShader, NULL, 0);

	devContext->PSSetShaderResources(0, 1, &srvWood);
	devContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devContext->RSSetState(rState_B_AA);
	devContext->DrawIndexed(FindNumIndicies(iBuffer_BarrelModel), 0, 0);
	devContext->RSSetState(rState_B_AA);
	devContext->DrawIndexed(FindNumIndicies(iBuffer_BarrelModel), 0, 0);
#pragma endregion

#pragma region Draw Cube
	stride = sizeof(VERTEX);	

	WVP = Mult_4x4(cube1World, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (cube1World);
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);	
	devContext->IASetVertexBuffers(0, 1, &vBuffer_Cube, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(vShader, NULL, 0);
	devContext->PSSetShader(pShader, NULL, 0);
	devContext->IASetInputLayout(vertLayout);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->PSSetShaderResources(0, 1, &srvGrass);
	devContext->PSSetSamplers(0, 1, &ssCube);
	devContext->DrawIndexed(FindNumIndicies(iBuffer_Cube), 0, 0);
#pragma endregion

#pragma region Draw Cube2
	WVP = Mult_4x4(cube2World, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (cube2World);
	cbPerObj.WVP = WVP;	

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	devContext->IASetVertexBuffers(0, 1, &vBuffer_Cube, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);

	devContext->IASetInputLayout(vertLayout);
	devContext->VSSetShader(vShader, NULL, 0);
	devContext->PSSetShader(pShader, NULL, 0);

	devContext->OMSetBlendState(bsTransparency, blendFactor, 0xffffffff);
	devContext->PSSetShaderResources(0, 1, &srvGlass);
	devContext->PSSetSamplers(0, 1, &ssCube);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devContext->RSSetState(rState_F_AA);
	devContext->DrawIndexed(FindNumIndicies(iBuffer_Cube), 0, 0);
#pragma endregion

#pragma region Draw Cube3
	WVP = Mult_4x4(cube3World, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (cube3World);
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	devContext->IASetVertexBuffers(0, 1, &vBuffer_Cube, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);

	devContext->IASetInputLayout(vertLayout);
	devContext->VSSetShader(vShader, NULL, 0);
	devContext->PSSetShader(pShader, NULL, 0);

	devContext->OMSetBlendState(bsTransparency, blendFactor, 0xffffffff);
	devContext->PSSetShaderResources(0, 1, &srvGlass);
	devContext->PSSetSamplers(0, 1, &ssCube);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devContext->RSSetState(rState_F_AA);
	devContext->DrawIndexed(FindNumIndicies(iBuffer_Cube), 0, 0);
#pragma endregion

#pragma region Draw Cube4
	WVP = Mult_4x4(cube4World, camView);
	WVP = Mult_4x4(WVP, camProjection);
	cbPerObj.World = (cube4World);
	cbPerObj.WVP = WVP;

	devContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	devContext->IASetVertexBuffers(0, 1, &vBuffer_Cube, &stride, &offset);
	devContext->IASetIndexBuffer(iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);

	devContext->IASetInputLayout(vertLayout);
	devContext->VSSetShader(vShader, NULL, 0);
	devContext->PSSetShader(pShader, NULL, 0);

	devContext->OMSetBlendState(bsTransparency, blendFactor, 0xffffffff);
	devContext->PSSetShaderResources(0, 1, &srvGlass);
	devContext->PSSetSamplers(0, 1, &ssCube);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devContext->RSSetState(rState_F_AA);
	devContext->DrawIndexed(FindNumIndicies(iBuffer_Cube), 0, 0);
#pragma endregion


	swapChain->Present(0, 0);
	return true;
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

	hr = device->CreateTexture2D(&depthBufferdesc, NULL, &dsBuffer);	//	Depth Stencil
	hr = device->CreateDepthStencilState(&dsDesc, &dsState);	//	Stencil state
	hr = device->CreateDepthStencilView(dsBuffer, 0, &dsView);	//	Stencil view

	devContext->OMSetRenderTargets(1, &rtView, dsView);

	// Set new proj matrix
	float ar = abs(width / height);
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

	//	Movement amount per frame
	float negMove = -(5.0f * (float)time);
	float posMove = 5.0f * (float)time;

	float negRotate = -(5.0f * (float)time);
	float posRotate = 5.0f * (float)time;

	if (keyboardState[DIK_ESCAPE] & 0x80){
		PostMessage(pApp->window, WM_DESTROY, 0, 0);
	}

#pragma region Camera Movement
	if (keyboardState[DIK_W] & 0x80)
		camView = Translate(camView, 0.0f, 0.0f, negMove);
	
	if (keyboardState[DIK_S] & 0x80)
		camView = Translate(camView, 0.0f, 0.0f, posMove);
	
	if (keyboardState[DIK_A] & 0x80)
		camView = Translate(camView, posMove, 0.0f, 0.0f);

	if (keyboardState[DIK_D] & 0x80)
		camView = Translate(camView, negMove, 0.0f, 0.0f);
	

	if (keyboardState[DIK_F] & 0x80)
		camView = Translate(camView, 0.0f, negMove, 0.0f);

	if (keyboardState[DIK_G] & 0x80)
		camView = Translate(camView, 0.0f, posMove, 0.0f);


	if (keyboardState[DIK_UPARROW] & 0x80)
		camView = RotateX(camView, negRotate);

	if (keyboardState[DIK_DOWNARROW] & 0x80)
		camView = RotateX(camView, posRotate);

	if (keyboardState[DIK_LEFTARROW] & 0x80)
		camView = RotateY(camView, negRotate);

	if (keyboardState[DIK_RIGHTARROW] & 0x80)
		camView = RotateY(camView, posRotate);
#pragma endregion

#pragma region Star Movement
	if (keyboardState[DIK_NUMPAD8] & 0x80)
		starWorld = Translate(starWorld, 0.0f, 0.0f, posMove);

	if (keyboardState[DIK_NUMPAD2] & 0x80)
		starWorld = Translate(starWorld, 0.0f, 0.0f, negMove);

	if (keyboardState[DIK_NUMPAD4] & 0x80)
		starWorld = Translate(starWorld, negMove, 0.0f, 0.0f);

	if (keyboardState[DIK_NUMPAD6] & 0x80)
		starWorld = Translate(starWorld, posMove, 0.0f, 0.0f);


	if (keyboardState[DIK_NUMPAD7] & 0x80)
		starWorld = Translate(starWorld, 0.0f, negMove, 0.0f);

	if (keyboardState[DIK_NUMPAD9] & 0x80)
		starWorld = Translate(starWorld, 0.0f, posMove, 0.0f);
#pragma endregion

	return;
}

UINT GraphicsProject::FindNumIndicies(ID3D11Buffer* b){
	D3D11_BUFFER_DESC ibufferDesc;
	b->GetDesc(&ibufferDesc);

	UINT numIndicies = ibufferDesc.ByteWidth / sizeof(UINT);

	return numIndicies;
}

bool GraphicsProject::ShutDown() {

	swapChain->Release();
	device->Release();
	devContext->Release();
	rtView->Release();

	cbPerObjectBuffer->Release();
	cbPerFrameBuffer->Release();

	iBuffer_Cube->Release();
	vBuffer_Cube->Release();
	vBuffer_LinkModel->Release();
	iBuffer_LinkModel->Release();
	vBuffer_BarrelModel->Release();
	iBuffer_BarrelModel->Release();
	iBuffer_Ground->Release();
	vBuffer_Ground->Release();
	vBuffer_Skybox->Release();
	iBuffer_Skybox->Release();
	iBuffer_Star->Release();
	vBuffer_Star->Release();

	dsBuffer->Release();
	dsView->Release();
	dsState->Release();

	vertLayout->Release();
	skyboxLayout->Release();
	starLayout->Release();

	vShader->Release();
	pShader->Release();
	vShader_Star->Release();
	pShader_Star->Release();
	vShader_Skybox->Release();
	pShader_Skybox->Release();

	ssCube->Release();
	ssSkybox->Release();
	bsTransparency->Release();

	srvGlass->Release();
	srvGrass->Release();
	srvGround->Release();
	srvSkymap->Release();
	srvWood->Release();

	rState_B_AA->Release();
	rState_B->Release();
	rState_F_AA->Release();
	rState_F->Release();
	rState_Wire->Release();
	
	DIKeyboard->Release();
	DIMouse->Release();

	delete linkModel;
	delete barrelModel;
	delete skyboxModel;

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

bool loadOBJ(const char* path, ID3D11Device* d, Model* m){

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

		m->interleaved.push_back(temp);
		m->out_Indicies.push_back(i);
	}

	return true;
}