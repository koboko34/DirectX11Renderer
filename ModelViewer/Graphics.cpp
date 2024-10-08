#include "Graphics.h"

#include "MyMacros.h"

Graphics::Graphics()
{
	m_SwapChain = 0;
	m_Device = 0;
	m_DeviceContext = 0;
	m_RenderTargetView = 0;
	m_DepthStencilBuffer = 0;
	m_DepthStencilState = 0;
	m_DepthStencilView = 0;
	m_RasterState = 0;
}

Graphics::Graphics(const Graphics& Other)
{
}

Graphics::~Graphics()
{
}

bool Graphics::Initialise(int ScreenWidth, int ScreenHeight, bool VSync, HWND hwnd, bool Fullscreen, float ScreenDepth, float ScreenNear)
{
	HRESULT hResult;
	IDXGIFactory* Factory;
	IDXGIAdapter* Adapter;
	IDXGIOutput* AdapterOutput;
	unsigned int NumModes, Numerator, Denominator;
	unsigned long long StringLength;
	DXGI_MODE_DESC* DisplayModeList;
	DXGI_ADAPTER_DESC AdapterDesc = {};
	int Error;
	DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
	D3D_FEATURE_LEVEL FeatureLevel;
	ID3D11Texture2D* BackBufferPtr;
	D3D11_TEXTURE2D_DESC DepthBufferDesc = {};
	D3D11_DEPTH_STENCIL_DESC DepthStencilDesc = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
	D3D11_RASTERIZER_DESC RasterDesc = {};
	float FieldOfView, ScreenAspect;

	m_VSync_Enabled = VSync;

	HFALSE_IF_FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory));
	HFALSE_IF_FAILED(Factory->EnumAdapters(0, &Adapter));
	HFALSE_IF_FAILED(Adapter->EnumOutputs(0, &AdapterOutput));

	// get the number of modes that fir the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor)
	HFALSE_IF_FAILED(AdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &NumModes, NULL));


	// create a list to hold all the possible display modes for this monitor/video card combination
	DisplayModeList = new DXGI_MODE_DESC[NumModes];
	if (!DisplayModeList)
	{
		return false;
	}

	HFALSE_IF_FAILED(AdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &NumModes, DisplayModeList));

	for (int i = 0; i < NumModes; i++)
	{
		if (DisplayModeList[i].Width == (unsigned int)ScreenWidth)
		{
			if (DisplayModeList[i].Height == (unsigned int)ScreenHeight)
			{
				Numerator = DisplayModeList[i].RefreshRate.Numerator;
				Denominator = DisplayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	HFALSE_IF_FAILED(Adapter->GetDesc(&AdapterDesc));

	m_VideoCardMemory = (int)(AdapterDesc.DedicatedVideoMemory / 1024 / 1024);

	Error = wcstombs_s(&StringLength, m_VideoCardDescription, 128, AdapterDesc.Description, 128);
	if (Error != 0)
	{
		return false;
	}

	delete[] DisplayModeList;
	DisplayModeList = 0;

	AdapterOutput->Release();
	AdapterOutput = 0;

	Adapter->Release();
	Adapter = 0;

	Factory->Release();
	Factory = 0;

	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.BufferDesc.Width = ScreenWidth;
	SwapChainDesc.BufferDesc.Height = ScreenHeight;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (m_VSync_Enabled)
	{
		SwapChainDesc.BufferDesc.RefreshRate.Numerator = Numerator;
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = Denominator;
	}
	else
	{
		SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = hwnd;

	// turn multisampling off
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;

	if (Fullscreen)
	{
		SwapChainDesc.Windowed = false;
	}
	else
	{
		SwapChainDesc.Windowed = true;
	}

	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags = 0;

	FeatureLevel = D3D_FEATURE_LEVEL_11_0;

	HFALSE_IF_FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &FeatureLevel, 1, D3D11_SDK_VERSION, &SwapChainDesc, &m_SwapChain, &m_Device, NULL, &m_DeviceContext));
	HFALSE_IF_FAILED(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBufferPtr));
	HFALSE_IF_FAILED(m_Device->CreateRenderTargetView(BackBufferPtr, NULL, &m_RenderTargetView));

	BackBufferPtr->Release();
	BackBufferPtr = 0;

	DepthBufferDesc.Width = ScreenWidth;
	DepthBufferDesc.Height = ScreenHeight;
	DepthBufferDesc.MipLevels = 1;
	DepthBufferDesc.ArraySize = 1;
	DepthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthBufferDesc.SampleDesc.Count = 1;
	DepthBufferDesc.SampleDesc.Quality = 0;
	DepthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthBufferDesc.CPUAccessFlags = 0;
	DepthBufferDesc.MiscFlags = 0;

	HFALSE_IF_FAILED(m_Device->CreateTexture2D(&DepthBufferDesc, NULL, &m_DepthStencilBuffer));

	DepthStencilDesc.DepthEnable = true;
	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	DepthStencilDesc.StencilEnable = true;
	DepthStencilDesc.StencilReadMask = 0xFF;
	DepthStencilDesc.StencilWriteMask = 0xFF;

	DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HFALSE_IF_FAILED(m_Device->CreateDepthStencilState(&DepthStencilDesc, &m_DepthStencilState));

	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState, 1);

	DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DepthStencilViewDesc.Texture2D.MipSlice = 0;

	HFALSE_IF_FAILED(m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &DepthStencilViewDesc, &m_DepthStencilView));

	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

	RasterDesc.AntialiasedLineEnable = false;
	RasterDesc.CullMode = D3D11_CULL_BACK;
	RasterDesc.DepthBias = 0;
	RasterDesc.DepthBiasClamp = 0.f;
	RasterDesc.DepthClipEnable = true;
	RasterDesc.FillMode = D3D11_FILL_SOLID;
	RasterDesc.FrontCounterClockwise = false;
	RasterDesc.MultisampleEnable = false;
	RasterDesc.ScissorEnable = false;
	RasterDesc.SlopeScaledDepthBias = 0.f;

	HFALSE_IF_FAILED(m_Device->CreateRasterizerState(&RasterDesc, &m_RasterState));

	m_DeviceContext->RSSetState(m_RasterState);

	m_Viewport.Width = (float)ScreenWidth;
	m_Viewport.Height = (float)ScreenHeight;
	m_Viewport.MinDepth = 0.f;
	m_Viewport.MaxDepth = 1.f;
	m_Viewport.TopLeftX = 0.f;
	m_Viewport.TopLeftY = 0.f;

	m_DeviceContext->RSSetViewports(1u, &m_Viewport);

	FieldOfView = 3.141592654f / 4.f;
	ScreenAspect = (float)ScreenWidth / (float)ScreenHeight;

	m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(FieldOfView, ScreenAspect, ScreenNear, ScreenDepth);
	m_WorldMatrix = DirectX::XMMatrixIdentity();
	m_OrthoMatrix = DirectX::XMMatrixOrthographicLH((float)ScreenWidth, (float)ScreenHeight, ScreenNear, ScreenDepth);

	return true;
}

void Graphics::Shutdown()
{
	if (m_SwapChain)
	{
		m_SwapChain->SetFullscreenState(false, NULL);
	}

	if (m_RasterState)
	{
		m_RasterState->Release();
		m_RasterState = 0;
	}

	if (m_DepthStencilView)
	{
		m_DepthStencilView->Release();
		m_DepthStencilView = 0;
	}

	if (m_DepthStencilState)
	{
		m_DepthStencilState->Release();
		m_DepthStencilState = 0;
	}

	if (m_DepthStencilBuffer)
	{
		m_DepthStencilBuffer->Release();
		m_DepthStencilBuffer = 0;
	}

	if (m_RenderTargetView)
	{
		m_RenderTargetView->Release();
		m_RenderTargetView = 0;
	}

	if (m_DeviceContext)
	{
		m_DeviceContext->Release();
		m_DeviceContext = 0;
	}

	if (m_Device)
	{
		m_Device->Release();
		m_Device = 0;
	}

	if (m_SwapChain)
	{
		m_SwapChain->Release();
		m_SwapChain = 0;
	}
}

void Graphics::BeginScene(float Red, float Green, float Blue, float Alpha)
{
	float Color[4];

	Color[0] = Red;
	Color[1] = Green;
	Color[2] = Blue;
	Color[3] = Alpha;

	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, Color);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.f, 0u);
}

void Graphics::EndScene()
{
	if (m_VSync_Enabled)
	{
		m_SwapChain->Present(1u, 0u);
	}
	else
	{
		m_SwapChain->Present(0u, 0u);
	}
}

void Graphics::GetVideoCardInfo(char* CardName, int& Memory)
{
	strcpy_s(CardName, 128, m_VideoCardDescription);
	Memory = m_VideoCardMemory;
}

void Graphics::SetBackBufferRenderTarget()
{
	m_DeviceContext->OMSetRenderTargets(1u, &m_RenderTargetView, m_DepthStencilView);
}

void Graphics::ResetViewport()
{
	m_DeviceContext->RSSetViewports(1u, &m_Viewport);
}
