#include "Graphics.h"

#include "ImGui\imgui_impl_dx11.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "MyMacros.h"


Graphics::Graphics()
{
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
	Microsoft::WRL::ComPtr<ID3D11Texture2D> PostProcessRTTFirst;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> PostProcessRTTSecond;
	D3D11_TEXTURE2D_DESC PostProcessTextureDesc = {};
	D3D11_TEXTURE2D_DESC DepthBufferDesc = {};
	D3D11_DEPTH_STENCIL_DESC DepthStencilDesc = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
	D3D11_RASTERIZER_DESC RasterDesc = {};
	D3D11_SAMPLER_DESC SamplerDesc = {};
	float FieldOfView, ScreenAspect;

	m_VSync_Enabled = VSync;
	m_Dimensions = std::make_pair(ScreenWidth, ScreenHeight);

	HFALSE_IF_FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory));
	HFALSE_IF_FAILED(Factory->EnumAdapters(0, &Adapter));
	HFALSE_IF_FAILED(Adapter->EnumOutputs(0, &AdapterOutput));

	HFALSE_IF_FAILED(AdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &NumModes, NULL));

	// create a list to hold all the possible display modes for this monitor/video card combination
	DisplayModeList = new DXGI_MODE_DESC[NumModes];
	if (!DisplayModeList)
	{
		return false;
	}

	HFALSE_IF_FAILED(AdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &NumModes, DisplayModeList));

	for (unsigned int i = 0; i < NumModes; i++)
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
	SwapChainDesc.BufferCount = 1; // create a second buffer and change swapchain swap effect

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
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // swap to DXGI_SWAP_FLIP_DISCARD when added a second buffer
	SwapChainDesc.Flags = 0;

	FeatureLevel = D3D_FEATURE_LEVEL_11_0;

	HFALSE_IF_FAILED(D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		&FeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&SwapChainDesc,
		&m_SwapChain,
		&m_Device,
		NULL,
		&m_DeviceContext)
	);
	HFALSE_IF_FAILED(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBufferPtr));
	HFALSE_IF_FAILED(m_Device->CreateRenderTargetView(BackBufferPtr, NULL, &m_BackBufferRTV));

	BackBufferPtr->Release();
	BackBufferPtr = 0;

	DepthBufferDesc.Width = ScreenWidth;
	DepthBufferDesc.Height = ScreenHeight;
	DepthBufferDesc.MipLevels = 1;
	DepthBufferDesc.ArraySize = 1;
	DepthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
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

	HFALSE_IF_FAILED(m_Device->CreateDepthStencilState(&DepthStencilDesc, &m_DepthStencilStateWriteEnabled));

	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	DepthStencilDesc.StencilWriteMask = 0;

	HFALSE_IF_FAILED(m_Device->CreateDepthStencilState(&DepthStencilDesc, &m_DepthStencilStateWriteDisabled));

	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	DepthStencilDesc.StencilEnable = false;
	DepthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	DepthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	HFALSE_IF_FAILED(m_Device->CreateDepthStencilState(&DepthStencilDesc, &m_DepthStencilStateWriteDisabledAlwaysPass));

	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilStateWriteEnabled.Get(), 1);

	DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DepthStencilViewDesc.Texture2D.MipSlice = 0;

	HFALSE_IF_FAILED(m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &DepthStencilViewDesc, &m_DepthStencilView));

	m_DeviceContext->OMSetRenderTargets(1, m_BackBufferRTV.GetAddressOf(), m_DepthStencilView.Get());

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

	HFALSE_IF_FAILED(m_Device->CreateRasterizerState(&RasterDesc, &m_RasterStateBackFaceCullOn));

	m_DeviceContext->RSSetState(m_RasterStateBackFaceCullOn.Get());

	RasterDesc.CullMode = D3D11_CULL_NONE;

	HFALSE_IF_FAILED(m_Device->CreateRasterizerState(&RasterDesc, &m_RasterStateBackFaceCullOff));

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

	// setting up render target textures, views and shader resource view for post processing
	PostProcessTextureDesc.Width = ScreenWidth;
	PostProcessTextureDesc.Height = ScreenHeight;
	PostProcessTextureDesc.MipLevels = 1;
	PostProcessTextureDesc.ArraySize = 1;
	PostProcessTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	PostProcessTextureDesc.SampleDesc.Count = 1;
	PostProcessTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	PostProcessTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HFALSE_IF_FAILED(m_Device->CreateTexture2D(&PostProcessTextureDesc, NULL, &PostProcessRTTFirst));
	HFALSE_IF_FAILED(m_Device->CreateTexture2D(&PostProcessTextureDesc, NULL, &PostProcessRTTSecond));

	HFALSE_IF_FAILED(m_Device->CreateRenderTargetView(PostProcessRTTFirst.Get(), NULL, &m_PostProcessRTVFirst));
	HFALSE_IF_FAILED(m_Device->CreateRenderTargetView(PostProcessRTTSecond.Get(), NULL, &m_PostProcessRTVSecond));

	HFALSE_IF_FAILED(m_Device->CreateShaderResourceView(PostProcessRTTFirst.Get(), NULL, &m_PostProcessSRVFirst));
	HFALSE_IF_FAILED(m_Device->CreateShaderResourceView(PostProcessRTTSecond.Get(), NULL, &m_PostProcessSRVSecond));

	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HFALSE_IF_FAILED(m_Device->CreateSamplerState(&SamplerDesc, &m_SamplerState));
	m_DeviceContext->PSSetSamplers(0, 1, m_SamplerState.GetAddressOf());

	ImGui_ImplDX11_Init(m_Device.Get(), m_DeviceContext.Get());

	return true;
}

void Graphics::Shutdown()
{
	if (m_SwapChain)
	{
		m_SwapChain->SetFullscreenState(false, NULL);
	}

	ImGui_ImplDX11_Shutdown();
}

void Graphics::BeginScene(float Red, float Green, float Blue, float Alpha)
{
	float Color[4];
	Color[0] = Red;
	Color[1] = Green;
	Color[2] = Blue;
	Color[3] = Alpha;

	m_DeviceContext->ClearRenderTargetView(m_BackBufferRTV.Get(), Color);
	m_DeviceContext->ClearRenderTargetView(m_PostProcessRTVFirst.Get(), Color);
	m_DeviceContext->ClearRenderTargetView(m_PostProcessRTVSecond.Get(), Color);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
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
	m_DeviceContext->OMSetRenderTargets(1u, m_BackBufferRTV.GetAddressOf(), m_DepthStencilView.Get());
}

void Graphics::EnableDepthWrite()
{
	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilStateWriteEnabled.Get(), 1);
}

void Graphics::DisableDepthWrite()
{
	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilStateWriteDisabled.Get(), 1);
}

void Graphics::DisableDepthWriteAlwaysPass()
{
	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilStateWriteDisabledAlwaysPass.Get(), 1);
}

void Graphics::ResetViewport()
{
	m_DeviceContext->RSSetViewports(1u, &m_Viewport);
}

void Graphics::SetRasterStateBackFaceCull(bool bShouldCull)
{
	GetDeviceContext()->RSSetState(bShouldCull ? m_RasterStateBackFaceCullOn.Get() : m_RasterStateBackFaceCullOff.Get());
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Graphics::LoadTexture(const char* Filepath)
{	
	int Width, Height, Channels;
	unsigned char* ImageData = stbi_load(Filepath, &Width, &Height, &Channels, 0);
	assert(ImageData);
	
	unsigned char* ImageDataRgba = ImageData;
	bool NeedsAlpha = Channels == 3;

	ID3D11Texture2D* Texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView;
	
	D3D11_TEXTURE2D_DESC TexDesc = {};
	TexDesc.Width = Width;
	TexDesc.Height = Height;
	TexDesc.MipLevels = 1;
	TexDesc.ArraySize = 1;
	TexDesc.SampleDesc.Count = 1;
	TexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	if (Channels == 1)
	{
		TexDesc.Format = DXGI_FORMAT_R8_UNORM;
	}
	else if (Channels == 2)
	{
		TexDesc.Format = DXGI_FORMAT_R8G8_UNORM;
	}
	else if (Channels == 3)
	{
		ImageDataRgba = new unsigned char[Width * Height * 4];

		for (int i = 0; i < Width * Height; i++)
		{
			ImageDataRgba[i * 4 + 0] = ImageData[i * 3 + 0];
			ImageDataRgba[i * 4 + 1] = ImageData[i * 3 + 1];
			ImageDataRgba[i * 4 + 2] = ImageData[i * 3 + 2];
			ImageDataRgba[i * 4 + 3] = 255;
		}
		
		Channels = 4;
		TexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	else
	{
		TexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = ImageDataRgba;
	InitData.SysMemPitch = Width * Channels;

	assert(FAILED(GetDevice()->CreateTexture2D(&TexDesc, &InitData, &Texture)) == false);
	assert(FAILED(GetDevice()->CreateShaderResourceView(Texture, NULL, &TextureView)) == false);

	if (NeedsAlpha)
	{
		delete[] ImageDataRgba;
	}
	stbi_image_free(ImageData);

	return TextureView;
}
