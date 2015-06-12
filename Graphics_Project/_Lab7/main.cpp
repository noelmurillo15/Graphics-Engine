#include "MathFunc.h"
#include "TimerClass.h"
#include "FPSClass.h"
#include "CPUClass.h"

#include "Cube.h"
#include "stars.h"

#include <ctime>
#include <string>

#include "PS_Cube.csh"
#include "PS_Grid.csh"
#include "PS_Star.csh"
#include "VS_Cube.csh"
#include "VS_Grid.csh"
#include "VS_Star.csh"

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

	ID3D11InputLayout*		iLayout_Grid = nullptr;
	ID3D11InputLayout*		iLayout_Skybox = nullptr;
	ID3D11InputLayout*		iLayout_Star = nullptr;
	ID3D11InputLayout*		vertLayout = nullptr;

	ID3D11Buffer*			cbuffer = nullptr;
	ID3D11Buffer*			vbuffer_Grid = nullptr;
	ID3D11Buffer*			ibuffer_Grid = nullptr;
	ID3D11Buffer*			vbuffer_Skybox = nullptr;
	ID3D11Buffer*			ibuffer_Skybox = nullptr;
	ID3D11Buffer*			vbuffer_Star = nullptr;
	ID3D11Buffer*			ibuffer_Star = nullptr;
	ID3D11Buffer*			CubeIndexBuffer = nullptr;
	ID3D11Buffer*			CubeVertexBuffer = nullptr;
	
	ID3D10Blob*				VS_Buffer = nullptr;
	ID3D10Blob*				PS_Buffer = nullptr;

	ID3D11VertexShader*		VS = nullptr;
	ID3D11VertexShader*		vShader_Grid = nullptr;
	ID3D11VertexShader*		vShader_Star = nullptr;
	ID3D11VertexShader*		vShader_Skybox = nullptr;

	ID3D11PixelShader*		PS = nullptr;
	ID3D11PixelShader*		pShader_Grid = nullptr;
	ID3D11PixelShader*		pShader_Star = nullptr;
	ID3D11PixelShader*		pShader_Skybox = nullptr;

	ID3D11RasterizerState*	rState_B_AA = nullptr;
	ID3D11RasterizerState*	rState_B = nullptr;
	ID3D11RasterizerState*	rState_F_AA = nullptr;
	ID3D11RasterizerState*	rState_F = nullptr;
	ID3D11RasterizerState*	rState_Wire = nullptr;

	ID3D11ShaderResourceView* srvSkybox = nullptr;
	ID3D11ShaderResourceView* srvCube = nullptr;

	ID3D11Texture2D*		skyboxTexture = nullptr;
	ID3D11Texture2D*		depthStencil = nullptr;

	ID3D11DepthStencilView* dsView = nullptr;
	ID3D11DepthStencilState*dsState = nullptr;

	ID3D11SamplerState*		samplerState = nullptr;
	ID3D11SamplerState*		CubeSamplerState = nullptr;
	
	ID3D11BlendState*		textureBlendState = nullptr;

	//	Input Data
	IDirectInputDevice8*	DIKeyboard;
	IDirectInputDevice8*	DIMouse;

	DIMOUSESTATE			mouseLastState;
	LPDIRECTINPUT8			DirectInput;

	//	Systems Data
	FPSClass				fpsTracker;
	TimerClass				timeTracker;
	CpuClass				cpuTracker;

	//	Camera
	OBJECT					cam_obj;

public:

	GraphicsProject();
	GraphicsProject(HINSTANCE hinst, WNDPROC proc);

	bool Update();
	bool ShutDown();

	void ResizeWin();
	void ChangeTitleBar(std::string _str);

	void DetectInput(double time, float w, float h);
	bool InitDirectInput(HINSTANCE hInstance);
};

GraphicsProject* pApp = nullptr;

GraphicsProject::GraphicsProject(HINSTANCE hinst, WNDPROC proc){

	//	Create All the Things!
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

	window = CreateWindow(L"DirectXApplication", L"Animated skybox", WS_OVERLAPPEDWINDOW,
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
	buffDesc.RefreshRate.Numerator = 0;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = buffDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Flags = 0;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
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

#pragma region Camera Projection
	cam_obj.world = Identity();
	cam_obj.view = Identity();
	cam_obj.skybox = Identity();
	cam_obj.grid = Identity();
	cam_obj.star = Identity();
	cam_obj.cube = Identity();

	float ar = abs((float)BUFFER_WIDTH / (float)BUFFER_HEIGHT);
	cam_obj.proj = CreateProjectionMatrix(100.0f, 0.1f, 72, ar);

	cam_obj.view = Translate(cam_obj.view, 0.5f, 0.5f, -0.85f);	//	back up
	cam_obj.view = FastInverse(cam_obj.view);	//	make this ho a camera

	cam_obj.skybox = Scale_4x4(cam_obj.skybox, 20.0f, 20.0f, 20.0f); // skybox EXPAND
	cam_obj.skybox = Translate(cam_obj.skybox, 0.0f, -10.0f, 0.0f);

	cam_obj.star = Scale_4x4(cam_obj.star, 0.2f, 0.2f, 0.2f); // star SHRINK
	cam_obj.star = Translate(cam_obj.star, 0.5f, 0.8f, 0.5f);

	cam_obj.cube = Scale_4x4(cam_obj.cube, 0.2f, 0.2f, 0.2f);
	cam_obj.cube = Translate(cam_obj.cube, 0.5f, 0.21f, 0.5f);
#pragma endregion

#pragma region Grid Setup
	SIMPLE_VERTEX			Grid[400];
	unsigned int			iGrid[80];

	for (int x = 0; x < 20; ++x){
		for (int y = 0; y < 20; ++y){
			int index = Convert2D_1D(y, x, 20);
			Grid[index].pos = { (x / 20.0f), 0.0f, (y / 20.0f), 1.0f };
			Grid[index].color = { 0.4f, 0.4f, 0.4f, 1.0f };
		}
	}
	//	Rows								//	Columns		         y  x 
	iGrid[0] = Convert2D_1D(0, 0, 20);      iGrid[40] = Convert2D_1D(0, 0, 20);
	iGrid[1] = Convert2D_1D(19, 0, 20);		iGrid[41] = Convert2D_1D(0, 19, 20);
	iGrid[2] = Convert2D_1D(0, 1, 20);		iGrid[42] = Convert2D_1D(1, 0, 20);
	iGrid[3] = Convert2D_1D(19, 1, 20);		iGrid[43] = Convert2D_1D(1, 19, 20);
	iGrid[4] = Convert2D_1D(0, 2, 20);		iGrid[44] = Convert2D_1D(2, 0, 20);
	iGrid[5] = Convert2D_1D(19, 2, 20);		iGrid[45] = Convert2D_1D(2, 19, 20);
	iGrid[6] = Convert2D_1D(0, 3, 20);		iGrid[46] = Convert2D_1D(3, 0, 20);
	iGrid[7] = Convert2D_1D(19, 3, 20);		iGrid[47] = Convert2D_1D(3, 19, 20);
	iGrid[8] = Convert2D_1D(0, 4, 20);		iGrid[48] = Convert2D_1D(4, 0, 20);
	iGrid[9] = Convert2D_1D(19, 4, 20);		iGrid[49] = Convert2D_1D(4, 19, 20);

	iGrid[10] = Convert2D_1D(0, 5, 20);		iGrid[50] = Convert2D_1D(5, 0, 20);
	iGrid[11] = Convert2D_1D(19, 5, 20);	iGrid[51] = Convert2D_1D(5, 19, 20);
	iGrid[12] = Convert2D_1D(0, 6, 20);		iGrid[52] = Convert2D_1D(6, 0, 20);
	iGrid[13] = Convert2D_1D(19, 6, 20);	iGrid[53] = Convert2D_1D(6, 19, 20);
	iGrid[14] = Convert2D_1D(0, 7, 20);		iGrid[54] = Convert2D_1D(7, 0, 20);
	iGrid[15] = Convert2D_1D(19, 7, 20);	iGrid[55] = Convert2D_1D(7, 19, 20);
	iGrid[16] = Convert2D_1D(0, 8, 20);		iGrid[56] = Convert2D_1D(8, 0, 20);
	iGrid[17] = Convert2D_1D(19, 8, 20);	iGrid[57] = Convert2D_1D(8, 19, 20);
	iGrid[18] = Convert2D_1D(0, 9, 20);		iGrid[58] = Convert2D_1D(9, 0, 20);
	iGrid[19] = Convert2D_1D(19, 9, 20);	iGrid[59] = Convert2D_1D(9, 19, 20);

	iGrid[20] = Convert2D_1D(0, 10, 20);	iGrid[60] = Convert2D_1D(10, 0, 20);
	iGrid[21] = Convert2D_1D(19, 10, 20);	iGrid[61] = Convert2D_1D(10, 19, 20);
	iGrid[22] = Convert2D_1D(0, 11, 20);	iGrid[62] = Convert2D_1D(11, 0, 20);
	iGrid[23] = Convert2D_1D(19, 11, 20);	iGrid[63] = Convert2D_1D(11, 19, 20);
	iGrid[24] = Convert2D_1D(0, 12, 20);	iGrid[64] = Convert2D_1D(12, 0, 20);
	iGrid[25] = Convert2D_1D(19, 12, 20);	iGrid[65] = Convert2D_1D(12, 19, 20);
	iGrid[26] = Convert2D_1D(0, 13, 20);	iGrid[66] = Convert2D_1D(13, 0, 20);
	iGrid[27] = Convert2D_1D(19, 13, 20);	iGrid[67] = Convert2D_1D(13, 19, 20);
	iGrid[28] = Convert2D_1D(0, 14, 20);	iGrid[68] = Convert2D_1D(14, 0, 20);
	iGrid[29] = Convert2D_1D(19, 14, 20);	iGrid[69] = Convert2D_1D(14, 19, 20);

	iGrid[30] = Convert2D_1D(0, 15, 20);	iGrid[70] = Convert2D_1D(15, 0, 20);
	iGrid[31] = Convert2D_1D(19, 15, 20);	iGrid[71] = Convert2D_1D(15, 19, 20);
	iGrid[32] = Convert2D_1D(0, 16, 20);	iGrid[72] = Convert2D_1D(16, 0, 20);
	iGrid[33] = Convert2D_1D(19, 16, 20);	iGrid[73] = Convert2D_1D(16, 19, 20);
	iGrid[34] = Convert2D_1D(0, 17, 20);	iGrid[74] = Convert2D_1D(17, 0, 20);
	iGrid[35] = Convert2D_1D(19, 17, 20);	iGrid[75] = Convert2D_1D(17, 19, 20);
	iGrid[36] = Convert2D_1D(0, 18, 20);	iGrid[76] = Convert2D_1D(18, 0, 20);
	iGrid[37] = Convert2D_1D(19, 18, 20);	iGrid[77] = Convert2D_1D(18, 19, 20);
	iGrid[38] = Convert2D_1D(0, 19, 20);	iGrid[78] = Convert2D_1D(19, 0, 20);
	iGrid[39] = Convert2D_1D(19, 19, 20);	iGrid[79] = Convert2D_1D(19, 19, 20);
#pragma endregion

#pragma region Star Setup
	SIMPLE_VERTEX			Star[20];
	unsigned int			iStar[108];

	Star[0].pos = Star[10].pos = { 0, 1 };
	Star[1].pos = Star[11].pos = { 0.4f, 0.2f };
	Star[2].pos = Star[12].pos = { 1, 0 };
	Star[3].pos = Star[13].pos = { 0.5f, -0.4f };
	Star[4].pos = Star[14].pos = { 0.75f, -1 };
	Star[5].pos = Star[15].pos = { 0, -0.65f };
	Star[6].pos = Star[16].pos = { -0.75f, -1 };
	Star[7].pos = Star[17].pos = { -0.5f, -0.4f };
	Star[8].pos = Star[18].pos = { -1, 0 };
	Star[9].pos = Star[19].pos = { -0.4f, 0.2f };

	for (int x = 0; x < 10; ++x){
		Star[x].pos.z = -0.25f;
		Star[10 + x].pos.z = 0.25f;

		Star[x].pos.w = 1;
		Star[10 + x].pos.w = 1;

		// Tips of Star       R  G  B  A
		Star[x].color = { 0.2f, 0.2f, 0.2f, 1 };
		Star[10 + x].color = { 0.2f, 0.2f, 0.2f, 1 };
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

#pragma region Cube Setup
	VERTEX Cube[] =
	{
		// Front Face
		VERTEX(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		VERTEX(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f),
		VERTEX(1.0f, 1.0f, -1.0f, 1.0f, 0.0f),
		VERTEX(1.0f, -1.0f, -1.0f, 1.0f, 1.0f),

		// Back Face
		VERTEX(-1.0f, -1.0f, 1.0f, 1.0f, 1.0f),
		VERTEX(1.0f, -1.0f, 1.0f, 0.0f, 1.0f),
		VERTEX(1.0f, 1.0f, 1.0f, 0.0f, 0.0f),
		VERTEX(-1.0f, 1.0f, 1.0f, 1.0f, 0.0f),

		// Top Face
		VERTEX(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f),
		VERTEX(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f),
		VERTEX(1.0f, 1.0f, 1.0f, 1.0f, 0.0f),
		VERTEX(1.0f, 1.0f, -1.0f, 1.0f, 1.0f),

		// Bottom Face
		VERTEX(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f),
		VERTEX(1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		VERTEX(1.0f, -1.0f, 1.0f, 0.0f, 0.0f),
		VERTEX(-1.0f, -1.0f, 1.0f, 1.0f, 0.0f),

		// Left Face
		VERTEX(-1.0f, -1.0f, 1.0f, 0.0f, 1.0f),
		VERTEX(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f),
		VERTEX(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f),
		VERTEX(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f),

		// Right Face
		VERTEX(1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		VERTEX(1.0f, 1.0f, -1.0f, 0.0f, 0.0f),
		VERTEX(1.0f, 1.0f, 1.0f, 1.0f, 0.0f),
		VERTEX(1.0f, -1.0f, 1.0f, 1.0f, 1.0f),
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

#pragma region InputLayer
	D3D11_INPUT_ELEMENT_DESC vLayout_skybox[2];
	vLayout_skybox[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	vLayout_skybox[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	D3D11_INPUT_ELEMENT_DESC vLayout_Grid[2];
	vLayout_Grid[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	vLayout_Grid[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	D3D11_INPUT_ELEMENT_DESC vLayout_Star[2];
	vLayout_Star[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	vLayout_Star[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	D3D11_INPUT_ELEMENT_DESC layout[2];
	layout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	layout[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 };
#pragma endregion

#pragma region VertexBuffer
		//	grid
	D3D11_BUFFER_DESC vbuffDesc_grid;
	ZeroMemory(&vbuffDesc_grid, sizeof(D3D11_BUFFER_DESC));
	vbuffDesc_grid.Usage = D3D11_USAGE_IMMUTABLE;
	vbuffDesc_grid.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbuffDesc_grid.ByteWidth = sizeof(SIMPLE_VERTEX) * 400;

	D3D11_SUBRESOURCE_DATA vsubData_grid;
	ZeroMemory(&vsubData_grid, sizeof(D3D11_SUBRESOURCE_DATA));
	vsubData_grid.pSysMem = Grid;

		//	skybox
	D3D11_BUFFER_DESC vbuffDesc_skybox;
	ZeroMemory(&vbuffDesc_skybox, sizeof(D3D11_BUFFER_DESC));
	vbuffDesc_skybox.Usage = D3D11_USAGE_IMMUTABLE;
	vbuffDesc_skybox.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbuffDesc_skybox.ByteWidth = sizeof(OBJ_VERT) * 776;

	D3D11_SUBRESOURCE_DATA vsubData_skybox;
	ZeroMemory(&vsubData_skybox, sizeof(D3D11_SUBRESOURCE_DATA));
	vsubData_skybox.pSysMem = Cube_data;

		//	star
	D3D11_BUFFER_DESC vBuffDesc_star;
	ZeroMemory(&vBuffDesc_star, sizeof(D3D11_BUFFER_DESC));
	vBuffDesc_star.Usage = D3D11_USAGE_IMMUTABLE;
	vBuffDesc_star.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBuffDesc_star.ByteWidth = sizeof(SIMPLE_VERTEX) * 20;

	D3D11_SUBRESOURCE_DATA vsubData_star;
	ZeroMemory(&vsubData_star, sizeof(D3D11_SUBRESOURCE_DATA));
	vsubData_star.pSysMem = Star;

		//	cube
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = sizeof(VERTEX) * 24;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertBufferData;
	ZeroMemory(&vertBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertBufferData.pSysMem = Cube;
#pragma endregion

#pragma region ConstBuffer
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.ByteWidth = sizeof(OBJECT);

	D3D11_SUBRESOURCE_DATA constSubData;
	ZeroMemory(&constSubData, sizeof(D3D11_SUBRESOURCE_DATA));
	constSubData.pSysMem = &cam_obj;
#pragma endregion

#pragma region IndexBuffer
		//	grid
	D3D11_BUFFER_DESC iBuffdesc_grid;
	ZeroMemory(&iBuffdesc_grid, sizeof(D3D11_BUFFER_DESC));
	iBuffdesc_grid.Usage = D3D11_USAGE_DEFAULT;
	iBuffdesc_grid.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBuffdesc_grid.ByteWidth = sizeof(iGrid);

	D3D11_SUBRESOURCE_DATA isubData_grid;
	ZeroMemory(&isubData_grid, sizeof(D3D11_SUBRESOURCE_DATA));
	isubData_grid.pSysMem = &iGrid;

		//	skybox
	D3D11_BUFFER_DESC iBuffdesc_skybox;
	ZeroMemory(&iBuffdesc_skybox, sizeof(D3D11_BUFFER_DESC));
	iBuffdesc_skybox.Usage = D3D11_USAGE_DEFAULT;
	iBuffdesc_skybox.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBuffdesc_skybox.ByteWidth = sizeof(const unsigned int) * 1692;

	D3D11_SUBRESOURCE_DATA isubData_skybox;
	ZeroMemory(&isubData_skybox, sizeof(D3D11_SUBRESOURCE_DATA));
	isubData_skybox.pSysMem = &Cube_indicies;

		//	star
	D3D11_BUFFER_DESC iBuffdesc_star;
	ZeroMemory(&iBuffdesc_star, sizeof(D3D11_BUFFER_DESC));
	iBuffdesc_star.Usage = D3D11_USAGE_DEFAULT;
	iBuffdesc_star.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBuffdesc_star.ByteWidth = sizeof(unsigned int) * 108;

	D3D11_SUBRESOURCE_DATA isubData_star;
	ZeroMemory(&isubData_star, sizeof(D3D11_SUBRESOURCE_DATA));
	isubData_star.pSysMem = &iStar;

		//	cube
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(UINT) * 12 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA iinitData;
	ZeroMemory(&iinitData, sizeof(D3D11_SUBRESOURCE_DATA));
	iinitData.pSysMem = iCube;
#pragma endregion

#pragma region DepthStencil
	D3D11_TEXTURE2D_DESC depthBufferdesc;
	ZeroMemory(&depthBufferdesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthBufferdesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferdesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthBufferdesc.Width = BUFFER_WIDTH;
	depthBufferdesc.Height = BUFFER_HEIGHT;
	depthBufferdesc.MipLevels = 1;
	depthBufferdesc.ArraySize = 1;
	depthBufferdesc.SampleDesc.Count = 1;

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

	D3D11_SUBRESOURCE_DATA stencilSubdata_skybox;
	stencilSubdata_skybox.pSysMem = Cube_indicies;
	stencilSubdata_skybox.SysMemPitch = sizeof(const unsigned int);
	stencilSubdata_skybox.SysMemSlicePitch = sizeof(Cube_indicies);

	D3D11_SUBRESOURCE_DATA stencilSubdata_star;
	stencilSubdata_star.pSysMem = iStar;
	stencilSubdata_star.SysMemPitch = sizeof(iStar[0]);
	stencilSubdata_star.SysMemSlicePitch = sizeof(iStar);
#pragma endregion

#pragma region Load Textures
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

		//	cube
	result = D3DX11CreateShaderResourceViewFromFile(device, L"grass.jpg", NULL, NULL, &srvCube, NULL);
#pragma endregion

#pragma region SamplerState
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = (-FLT_MAX);
	samplerDesc.MaxLOD = (FLT_MAX);
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
#pragma endregion

#pragma region Blendstate
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
#pragma endregion

#pragma region RasterDesc
	D3D11_RASTERIZER_DESC rasDesc_BC;
	ZeroMemory(&rasDesc_BC, sizeof(D3D11_RASTERIZER_DESC));
	rasDesc_BC.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasDesc_BC.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK; // back cull
	rasDesc_BC.DepthClipEnable = TRUE;

	D3D11_RASTERIZER_DESC rasDesc_BC_AA;
	ZeroMemory(&rasDesc_BC_AA, sizeof(D3D11_RASTERIZER_DESC));
	rasDesc_BC_AA.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasDesc_BC_AA.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;	//	back cull
	rasDesc_BC_AA.DepthClipEnable = TRUE;
	rasDesc_BC_AA.AntialiasedLineEnable = TRUE;

	D3D11_RASTERIZER_DESC rasDesc_FC;
	ZeroMemory(&rasDesc_FC, sizeof(D3D11_RASTERIZER_DESC));
	rasDesc_FC.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasDesc_FC.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;	//	front cull
	rasDesc_FC.DepthClipEnable = TRUE;

	D3D11_RASTERIZER_DESC rasDesc_FC_AA;
	ZeroMemory(&rasDesc_FC_AA, sizeof(D3D11_RASTERIZER_DESC));
	rasDesc_FC_AA.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasDesc_FC_AA.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;	//	front cull
	rasDesc_FC_AA.DepthClipEnable = TRUE;
	rasDesc_FC_AA.AntialiasedLineEnable = TRUE;


	D3D11_RASTERIZER_DESC rasDesc_Wire;
	ZeroMemory(&rasDesc_Wire, sizeof(D3D11_RASTERIZER_DESC));
	rasDesc_Wire.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	rasDesc_Wire.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;	//	front cull
#pragma endregion

#pragma region Viewport
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.Height = BUFFER_HEIGHT;
	viewport.Width = BUFFER_WIDTH;
	viewport.MaxDepth = 1.0f;
#pragma endregion

#pragma region Compile .fx Shaders
	result = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "main", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
	result = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);
#pragma endregion


#pragma region CREATE ALL THE THINGS!
	result = device->CreateBuffer(&constBufferDesc, &constSubData, &cbuffer);	//	cBuffer	grid
	result = device->CreateBuffer(&vbuffDesc_grid, &vsubData_grid, &vbuffer_Grid);	//	vBuffer grid
	result = device->CreateBuffer(&iBuffdesc_grid, &isubData_grid, &ibuffer_Grid);	//	iBuffer	grid
	result = device->CreateBuffer(&vbuffDesc_skybox, &vsubData_skybox, &vbuffer_Skybox);	//	vBuffer skybox
	result = device->CreateBuffer(&iBuffdesc_skybox, &isubData_skybox, &ibuffer_Skybox);	//	iBuffer skybox
	result = device->CreateBuffer(&vBuffDesc_star, &vsubData_star, &vbuffer_Star);	//	vBuffer star
	result = device->CreateBuffer(&iBuffdesc_star, &isubData_star, &ibuffer_Star);	//	iBuffer star
	result = device->CreateBuffer(&vertexBufferDesc, &vertBufferData, &CubeVertexBuffer);	//	vBuffer cube
	result = device->CreateBuffer(&indexBufferDesc, &iinitData, &CubeIndexBuffer);	//	iBuffer cube

	result = device->CreateVertexShader(VS_Grid, sizeof(VS_Grid), NULL, &vShader_Grid);	//	vShader grid
	result = device->CreatePixelShader(PS_Grid, sizeof(PS_Grid), NULL, &pShader_Grid);		//	pShader grid
	result = device->CreateVertexShader(VS_Cube, sizeof(VS_Cube), NULL, &vShader_Skybox);	//	vShader cube
	result = device->CreatePixelShader(PS_Cube, sizeof(PS_Cube), NULL, &pShader_Skybox);		//	pShader cube
	result = device->CreateVertexShader(VS_Star, sizeof(VS_Star), NULL, &vShader_Star);	//	vShader star
	result = device->CreatePixelShader(PS_Star, sizeof(PS_Star), NULL, &pShader_Star);		//	pShader star
	result = device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);	//	vShader cube
	result = device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);	//	pShader cube

	result = device->CreateInputLayout(vLayout_Grid, 2, VS_Grid, sizeof(VS_Grid), &iLayout_Grid);	//	Layout grid
	result = device->CreateInputLayout(vLayout_skybox, 2, VS_Cube, sizeof(VS_Cube), &iLayout_Skybox);	//	Layout skybox
	result = device->CreateInputLayout(vLayout_Star, 2, VS_Star, sizeof(VS_Star), &iLayout_Star);	//	Layout star
	result = device->CreateInputLayout(layout, 2, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);	//	Layout cube

	result = device->CreateTexture2D(&depthBufferdesc, &stencilSubdata_skybox, &depthStencil);	//	Depth Stencil
	result = device->CreateTexture2D(&depthBufferdesc, &stencilSubdata_star, &depthStencil);	//	Depth Stencil
	result = device->CreateDepthStencilState(&dsDesc, &dsState);	//	Stencil state
	result = device->CreateDepthStencilView(depthStencil, 0, &dsView);	//	Stencil view
	result = device->CreateTexture2D(&textureDesc, textureSubdata, &skyboxTexture);	//	skybox_obj Texture
	
	result = device->CreateSamplerState(&samplerDesc, &samplerState);	//	Sampler State skybox
	result = device->CreateSamplerState(&sampDesc, &CubeSamplerState);	//	Sampler State cube

	result = device->CreateRasterizerState(&rasDesc_BC, &rState_B);	//	Rasterizer State
	result = device->CreateRasterizerState(&rasDesc_BC_AA, &rState_B_AA);	//	Rasterizer State
	result = device->CreateRasterizerState(&rasDesc_FC, &rState_F);	//	Rasterizer State
	result = device->CreateRasterizerState(&rasDesc_FC_AA, &rState_F_AA);	//	Rasterizer State
	result = device->CreateRasterizerState(&rasDesc_Wire, &rState_Wire);	//	Rasterizer State

	result = device->CreateBlendState(&blendDesc, &textureBlendState);	//	Blend State
#pragma endregion

}

bool GraphicsProject::Update() {

	HRESULT result;

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

#pragma region viewports&RenderTargets
	//	Background Color
	FLOAT RGBA[4]; RGBA[0] = 0; RGBA[1] = 0; RGBA[2] = 0; RGBA[3] = 1;

	//	Clear views
	devContext->ClearRenderTargetView(rtView, RGBA);
	devContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	devContext->OMSetDepthStencilState(dsState, 1);
	devContext->OMSetRenderTargets(1, &rtView, dsView);
	devContext->RSSetViewports(1, &viewport);


	if (srvSkybox) {
		srvSkybox->Release();
		srvSkybox = nullptr;
	}

	result = device->CreateShaderResourceView(skyboxTexture, NULL, &srvSkybox);	//	Shadr res view

#pragma endregion

#pragma region Map CBuff
	D3D11_MAPPED_SUBRESOURCE mappedSubres;
	devContext->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubres);
	memcpy(mappedSubres.pData, &cam_obj, sizeof(cam_obj));
	devContext->Unmap(cbuffer, 0);
	devContext->VSSetConstantBuffers(1, 1, &cbuffer);
#pragma endregion

#pragma region Anti-Alaising
	bool anti_alaising = false;
	if (GetAsyncKeyState('Q') & 0x8000)
		anti_alaising = true;
#pragma endregion

#pragma region Draw Skybox
	UINT strideValue = sizeof(OBJ_VERT);
	UINT offset = 0;

	devContext->RSSetState(rState_F);	//	needs front culling
	devContext->IASetVertexBuffers(0, 1, &vbuffer_Skybox, &strideValue, &offset);
	devContext->IASetIndexBuffer(ibuffer_Skybox, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(vShader_Skybox, NULL, 0);
	devContext->PSSetShader(pShader_Skybox, NULL, 0);
	devContext->IASetInputLayout(iLayout_Skybox);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->PSSetShaderResources(0, 1, &srvSkybox);
	devContext->PSSetSamplers(0, 1, &samplerState);
	devContext->DrawIndexed(1692, 0, 0);
#pragma endregion

#pragma region Draw Star
	cam_obj.star = RotateZ_Local(cam_obj.star, (float)(timeTracker.GetTime() * 0.0000025f));

	strideValue = sizeof(SIMPLE_VERTEX);
	devContext->RSSetState(rState_Wire); 	//	needs back culling or wire
	devContext->IASetVertexBuffers(0, 1, &vbuffer_Star, &strideValue, &offset);
	devContext->IASetIndexBuffer(ibuffer_Star, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(vShader_Star, NULL, 0);
	devContext->PSSetShader(pShader_Star, NULL, 0);
	devContext->IASetInputLayout(iLayout_Star);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->DrawIndexed(108, 0, 0);
#pragma endregion

#pragma region Draw Grid
	if (anti_alaising)
		devContext->RSSetState(rState_F_AA);
	else
		devContext->RSSetState(rState_F);	//	Front or Back culling

	strideValue = sizeof(SIMPLE_VERTEX);
	devContext->IASetVertexBuffers(0, 1, &vbuffer_Grid, &strideValue, &offset);
	devContext->IASetIndexBuffer(ibuffer_Grid, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(vShader_Grid, NULL, 0);
	devContext->PSSetShader(pShader_Grid, NULL, 0);
	devContext->IASetInputLayout(iLayout_Grid);
	devContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);// triangle strip for land
	devContext->DrawIndexed(80, 0, 0);
#pragma endregion

#pragma region Draw Cube
	strideValue = sizeof(VERTEX);
	devContext->RSSetState(rState_B);	//	needs back culling
	devContext->IASetVertexBuffers(0, 1, &CubeVertexBuffer, &strideValue, &offset);
	devContext->IASetIndexBuffer(CubeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	devContext->VSSetShader(VS, 0, 0);
	devContext->PSSetShader(PS, 0, 0);
	devContext->IASetInputLayout(vertLayout);
	devContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->PSSetShaderResources(0, 1, &srvCube);
	devContext->PSSetSamplers(0, 1, &CubeSamplerState);
	devContext->DrawIndexed(36, 0, 0);
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
	depthStencil->Release();
	dsView->Release();
	dsView = nullptr;
	dsState->Release();

	//	resize
	hr = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	//scDesc.BufferDesc

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

	hr = device->CreateTexture2D(&depthBufferdesc, &stenilSubres, &depthStencil);	//	Depth Stencil
	hr = device->CreateDepthStencilState(&dsDesc, &dsState);	//	Stencil state
	hr = device->CreateDepthStencilView(depthStencil, 0, &dsView);	//	Stencil view

	devContext->OMSetRenderTargets(1, &rtView, dsView);

	// Set new proj matrix
	float ar = abs((float)width / (float)height);
	cam_obj.proj = CreateProjectionMatrix(100.0f, 0.1f, 72, ar);

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

	//	cam Movement
	if (keyboardState[DIK_W] & 0x80)
		cam_obj.view = Translate(cam_obj.view, 0.0f, 0.0f, negMove);

	if (keyboardState[DIK_S] & 0x80)
		cam_obj.view = Translate(cam_obj.view, 0.0f, 0.0f, posMove);

	if (keyboardState[DIK_A] & 0x80)
		cam_obj.view = Translate(cam_obj.view, posMove, 0.0f, 0.0f);

	if (keyboardState[DIK_D] & 0x80)
		cam_obj.view = Translate(cam_obj.view, negMove, 0.0f, 0.0f);

	//	cam Fly / Ground
	if (keyboardState[DIK_F] & 0x80)
		cam_obj.view = Translate(cam_obj.view, 0.0f, negMove, 0.0f);

	if (keyboardState[DIK_G] & 0x80)
		cam_obj.view = Translate(cam_obj.view, 0.0f, posMove, 0.0f);

	//	cam Rotation
	if (keyboardState[DIK_UPARROW] & 0x80){
		cam_obj.view = RotateX(cam_obj.view, negRotate);
		cam_obj.skybox = RotateX(cam_obj.skybox, negRotate);
	}

	if (keyboardState[DIK_DOWNARROW] & 0x80){
		cam_obj.view = RotateX(cam_obj.view, posRotate);
		cam_obj.skybox = RotateX(cam_obj.skybox, posRotate);
	}

	if (keyboardState[DIK_LEFTARROW] & 0x80){
		cam_obj.view = RotateY(cam_obj.view, negRotate);
		cam_obj.skybox = RotateY(cam_obj.skybox, negRotate);
	}

	if (keyboardState[DIK_RIGHTARROW] & 0x80){
		cam_obj.view = RotateY(cam_obj.view, posRotate);
		cam_obj.skybox = RotateY(cam_obj.skybox, posRotate);
	}

	//	star Movement
	if (keyboardState[DIK_NUMPAD2] & 0x80)
		cam_obj.star = Translate(cam_obj.star, 0.0f, 0.0f, negMove);

	if (keyboardState[DIK_NUMPAD8] & 0x80)
		cam_obj.star = Translate(cam_obj.star, 0.0f, 0.0f, posMove);

	if (keyboardState[DIK_NUMPAD6] & 0x80)
		cam_obj.star = Translate(cam_obj.star, posMove, 0.0f, 0.0f);

	if (keyboardState[DIK_NUMPAD4] & 0x80)
		cam_obj.star = Translate(cam_obj.star, negMove, 0.0f, 0.0f);

	//	Reset Camera
	if (keyboardState[DIK_M] & 0x80){
		cam_obj.world = Identity();
		cam_obj.view = Identity();
		cam_obj.skybox = Identity();
		cam_obj.grid = Identity();
		cam_obj.star = Identity();

		float ar = abs(w / h);
		cam_obj.proj = CreateProjectionMatrix(100.0f, 0.1f, 72, ar);

		cam_obj.view = Translate(cam_obj.view, 0.5f, 0.5f, -0.85f);	//	back up
		cam_obj.view = FastInverse(cam_obj.view);	//	make this ho a camera

		cam_obj.skybox = Scale_4x4(cam_obj.skybox, 20.0f, 20.0f, 20.0f); // skybox EXPAND
		cam_obj.skybox = Translate(cam_obj.skybox, 0.0f, -10.0f, 0.0f);

		cam_obj.star = Scale_4x4(cam_obj.star, 0.2f, 0.2f, 0.2f); // star SHRINK
		cam_obj.star = Translate(cam_obj.star, 0.5f, 0.8f, 0.5f);
	}
#pragma endregion

	return;
}

bool GraphicsProject::ShutDown() {

	swapChain->Release();
	device->Release();
	devContext->Release();
	rtView->Release();

	iLayout_Grid->Release();
	iLayout_Skybox->Release();
	iLayout_Star->Release();
	vertLayout->Release();

	cbuffer->Release();
	vbuffer_Grid->Release();
	ibuffer_Grid->Release();
	vbuffer_Skybox->Release();
	ibuffer_Skybox->Release();
	vbuffer_Star->Release();
	ibuffer_Star->Release();
	CubeIndexBuffer->Release();
	CubeVertexBuffer->Release();

	VS_Buffer->Release();
	PS_Buffer->Release();

	VS->Release();
	vShader_Grid->Release();
	vShader_Star->Release();
	vShader_Skybox->Release();

	PS->Release();
	pShader_Grid->Release();
	pShader_Star->Release();
	pShader_Skybox->Release();

	rState_B_AA->Release();
	rState_B->Release();
	rState_F_AA->Release();
	rState_F->Release();
	rState_Wire->Release();

	srvSkybox->Release();
	srvCube->Release();

	skyboxTexture->Release();
	depthStencil->Release();

	dsView->Release();
	dsState->Release();

	samplerState->Release();
	CubeSamplerState->Release();

	textureBlendState->Release();

	DIKeyboard->Release();
	DIMouse->Release();
	DirectInput->Release();

	pApp = nullptr;

	UnregisterClass(L"DirectXApplication", application);
	return true;
}



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int) {
	srand(unsigned int(time(0)));
	GraphicsProject myApp(hInstance, (WNDPROC)WndProc);
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