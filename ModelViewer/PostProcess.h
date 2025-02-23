#pragma once

#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <iostream>
#include <cassert>

#include "Shader.h"
#include "MyMacros.h"
#include "Application.h"

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 TexCoord;
	DirectX::XMFLOAT3 Normal;
};

class PostProcess
{
public:
	void ApplyPostProcess(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV, ID3D11DepthStencilView* DSV)
	{
		ID3D11RenderTargetView* NullRTVs[] = { nullptr };
		Application::GetSingletonPtr()->GetGraphics()->GetDeviceContext()->OMSetRenderTargets(1u, NullRTVs, DSV);
		ApplyPostProcessImpl(DeviceContext, RTV, SRV, DSV);
	}

	virtual ~PostProcess() {}

	static Microsoft::WRL::ComPtr<ID3D11VertexShader> GetQuadVertexShader(ID3D11Device* Device)
	{
		if (ms_bInitialised)
		{
			return PostProcess::ms_QuadVertexShader;
		}

		InitialiseShaderResources(Device);
		return PostProcess::ms_QuadVertexShader;
	}

	static Microsoft::WRL::ComPtr<ID3D11Buffer> GetQuadVertexBuffer(ID3D11Device* Device)
	{
		if (ms_bInitialised)
		{
			return PostProcess::ms_QuadVertexBuffer;
		}

		InitialiseShaderResources(Device);
		return PostProcess::ms_QuadVertexBuffer;
	}

	static Microsoft::WRL::ComPtr<ID3D11Buffer> GetQuadIndexBuffer(ID3D11Device* Device)
	{
		if (ms_bInitialised)
		{
			return PostProcess::ms_QuadIndexBuffer;
		}

		InitialiseShaderResources(Device);
		return PostProcess::ms_QuadIndexBuffer;
	}

	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader() const { return m_PixelShader; };

protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	
	virtual void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) = 0;

	bool SetupPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader>& PixelShader, const wchar_t* PSFilepath, const char* entryFunc = "main")
	{		
		wchar_t psFilename[128];
		int Error;

		Error = wcscpy_s(psFilename, 128, PSFilepath);
		if (Error != 0)
		{
			return false;
		}

		HRESULT hResult;
		Microsoft::WRL::ComPtr<ID3D10Blob> ErrorMessage;
		Microsoft::WRL::ComPtr<ID3D10Blob> psBuffer;

		UINT CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		hResult = D3DCompileFromFile(psFilename, NULL, NULL, entryFunc, "ps_5_0", CompileFlags, 0, &psBuffer, &ErrorMessage);
		if (FAILED(hResult))
		{
			if (ErrorMessage.Get())
			{
				Shader::OutputShaderErrorMessage(ErrorMessage.Get(), Application::GetSingletonPtr()->GetWindowHandle(), psFilename);
			}
			else
			{
				MessageBox(Application::GetSingletonPtr()->GetWindowHandle(), psFilename, L"Missing shader file!", MB_OK);
			}
			return false;
		}

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &PixelShader));

		return true;
	}

private:
	static void InitialiseShaderResources(ID3D11Device* Device)
	{
		wchar_t vsFilename[128];
		int Error;

		Error = wcscpy_s(vsFilename, 128, L"Shaders/QuadVS.hlsl");
		if (Error != 0)
		{
			return;
		}

		HRESULT hResult;
		Microsoft::WRL::ComPtr<ID3D10Blob> ErrorMessage;
		Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;
		D3D11_INPUT_ELEMENT_DESC VertexLayout[3] = {};
		D3D11_DEPTH_STENCIL_DESC DepthStencilDesc = {};
		unsigned int NumElements;

		UINT CompileFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		hResult = D3DCompileFromFile(vsFilename, NULL, NULL, "main", "vs_5_0", CompileFlags, 0, &vsBuffer, &ErrorMessage);
		if (FAILED(hResult))
		{
			if (ErrorMessage.Get())
			{
				Shader::OutputShaderErrorMessage(ErrorMessage.Get(), Application::GetSingletonPtr()->GetWindowHandle(), vsFilename);
			}
			else
			{
				MessageBox(Application::GetSingletonPtr()->GetWindowHandle(), vsFilename, L"Missing shader file!", MB_OK);
			}
			return;
		}

		ASSERT_NOT_FAILED(Device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), NULL, &ms_QuadVertexShader));

		VertexLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		VertexLayout[0].SemanticName = "POSITION";
		VertexLayout[0].SemanticIndex = 0;
		VertexLayout[0].InputSlot = 0;
		VertexLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		VertexLayout[0].AlignedByteOffset = 0;
		VertexLayout[0].InstanceDataStepRate = 0;

		VertexLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		VertexLayout[1].SemanticName = "TEXCOORD";
		VertexLayout[1].SemanticIndex = 0;
		VertexLayout[1].InputSlot = 0;
		VertexLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		VertexLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		VertexLayout[1].InstanceDataStepRate = 0;

		VertexLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		VertexLayout[2].SemanticName = "NORMAL";
		VertexLayout[2].SemanticIndex = 0;
		VertexLayout[2].InputSlot = 0;
		VertexLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		VertexLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		VertexLayout[2].InstanceDataStepRate = 0;

		NumElements = sizeof(VertexLayout) / sizeof(VertexLayout[0]);
		ASSERT_NOT_FAILED(Device->CreateInputLayout(VertexLayout, NumElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &ms_QuadInputLayout));

		Vertex QuadVertices[] = {
			{ DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), },
			{ DirectX::XMFLOAT3( 1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), },
			{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), },
			{ DirectX::XMFLOAT3( 1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f)  },
		};

		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(QuadVertices);
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = QuadVertices;

		ASSERT_NOT_FAILED(Device->CreateBuffer(&BufferDesc, &InitData, &ms_QuadVertexBuffer));

		unsigned int QuadIndices[] = {
			1, 2, 0,
			3, 2, 1
		};

		BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(QuadIndices);
		BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		BufferDesc.CPUAccessFlags = 0;

		InitData = {};
		InitData.pSysMem = QuadIndices;

		ASSERT_NOT_FAILED(Device->CreateBuffer(&BufferDesc, &InitData, &ms_QuadIndexBuffer));

		ms_bInitialised = true;
	}

	static Microsoft::WRL::ComPtr<ID3D11VertexShader> ms_QuadVertexShader;
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> ms_QuadInputLayout;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> ms_QuadVertexBuffer;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> ms_QuadIndexBuffer;
	static bool ms_bInitialised;
};


/////////////////////////////////////////////////////////////////////////////////

class PostProcessEmpty : public PostProcess
{
public:
	PostProcessEmpty()
	{
		SetupPixelShader(m_PixelShader, L"Shaders/QuadPS.hlsl");
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessFog : public PostProcess
{
public:
	PostProcessFog() {} // pass whatever we need into here
	
private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
							ID3D11DepthStencilView* DSV) override
	{
		std::cout << "Applying fog..." << std::endl;
	}
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessBlurHorizontal : public PostProcess
{
public:
	PostProcessBlurHorizontal(int BlurStrength) : m_BlurStrength(BlurStrength)
	{
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, nullptr, &m_ConstantBuffer));

		SetupPixelShader(m_PixelShader, L"Shaders/BoxBlurPS.hlsl", "HorizontalPS");
	}

private:
	void UpdateConstantBuffer(ID3D11DeviceContext* DeviceContext)
	{
		HRESULT hResult;
		D3D11_MAPPED_SUBRESOURCE MappedResource = {};
		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		DirectX::XMFLOAT2 TexelSize = DirectX::XMFLOAT2(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second);
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(TexelSize.x, TexelSize.y, (float)m_BlurStrength, 0.f);

		ASSERT_NOT_FAILED(DeviceContext->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));

		memcpy(MappedResource.pData, &Data, sizeof(Data));
		DeviceContext->Unmap(m_ConstantBuffer.Get(), 0u);
	}

	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
							ID3D11DepthStencilView* DSV) override
	{
		DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		UpdateConstantBuffer(DeviceContext);
		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

private:
	int m_BlurStrength;

};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessBlurVertical : public PostProcess
{
public:
	PostProcessBlurVertical(int BlurStrength) : m_BlurStrength(BlurStrength)
	{
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, nullptr, &m_ConstantBuffer));

		SetupPixelShader(m_PixelShader, L"Shaders/BoxBlurPS.hlsl", "VerticalPS");
	}

private:
	void UpdateConstantBuffer(ID3D11DeviceContext* DeviceContext)
	{
		HRESULT hResult;
		D3D11_MAPPED_SUBRESOURCE MappedResource = {};
		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		DirectX::XMFLOAT2 TexelSize = DirectX::XMFLOAT2(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second);
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(TexelSize.x, TexelSize.y, (float)m_BlurStrength, 0.f);

		ASSERT_NOT_FAILED(DeviceContext->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));

		memcpy(MappedResource.pData, &Data, sizeof(Data));
		DeviceContext->Unmap(m_ConstantBuffer.Get(), 0u);
	}

	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		UpdateConstantBuffer(DeviceContext);
		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

private:
	int m_BlurStrength;
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessPixelation : public PostProcess
{
public:
	PostProcessPixelation(float pixelSize) : m_PixelSize(pixelSize)
	{
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second, m_PixelSize, 0.f);
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		SetupPixelShader(m_PixelShader, L"Shaders/PixelationPS.hlsl");
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

private:
	float m_PixelSize;
};

class PostProcessBloom : public PostProcess
{
public:
	PostProcessBloom(float LuminanceThreshold) : m_LuminanceThreshold(LuminanceThreshold)
	{
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(m_LuminanceThreshold, 0.f, 0.f, 0.f);
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		SetupPixelShader(m_LuminancePS, L"Shaders/BloomPS.hlsl", "LuminancePS");
		SetupPixelShader(m_BloomPS, L"Shaders/BloomPS.hlsl", "BloomPS");

		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		D3D11_TEXTURE2D_DESC TextureDesc = {};
		TextureDesc.Width = Dimensions.first;
		TextureDesc.Height = Dimensions.second;
		TextureDesc.MipLevels = 1;
		TextureDesc.ArraySize = 1;
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.Usage = D3D11_USAGE_DEFAULT;
		TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		
		ID3D11Texture2D* LuminousTexture;
		ID3D11Texture2D* BlurIntermediateTexture;
		ID3D11Texture2D* BlurredTexture;
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &LuminousTexture));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &BlurIntermediateTexture));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &BlurredTexture));

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateRenderTargetView(LuminousTexture, NULL, &m_LuminousRTV));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateRenderTargetView(BlurIntermediateTexture, NULL, &m_BlurIntermediateRTV));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateRenderTargetView(BlurredTexture, NULL, &m_BlurredRTV));

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateShaderResourceView(LuminousTexture, NULL, &m_LuminousSRV));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateShaderResourceView(BlurIntermediateTexture, NULL, &m_BlurIntermediateSRV));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateShaderResourceView(BlurredTexture, NULL, &m_BlurredSRV));

		m_BlurPostProcesses.emplace_back(std::make_unique<PostProcessBlurHorizontal>(5));
		m_BlurPostProcesses.emplace_back(std::make_unique<PostProcessBlurVertical>(5));
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{		
		// render luminous pixels
		DeviceContext->PSSetShader(m_LuminancePS.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, m_LuminousRTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);

		// blur luminous pixels
		m_BlurPostProcesses[0]->ApplyPostProcess(DeviceContext, m_BlurIntermediateRTV, m_LuminousSRV, DSV);
		ID3D11ShaderResourceView* NullSRVs[] = { nullptr };
		DeviceContext->PSSetShaderResources(1u, 1u, NullSRVs);
		m_BlurPostProcesses[1]->ApplyPostProcess(DeviceContext, m_BlurredRTV, m_BlurIntermediateSRV, DSV);

		// add bloom to original
		ID3D11RenderTargetView* NullRTVs[] = { nullptr };
		Application::GetSingletonPtr()->GetGraphics()->GetDeviceContext()->OMSetRenderTargets(1u, NullRTVs, DSV);

		DeviceContext->PSSetShader(m_BloomPS.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());
		DeviceContext->PSSetShaderResources(1u, 1u, m_BlurredSRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

private:
	float m_LuminanceThreshold;
	std::vector<std::unique_ptr<PostProcess>> m_BlurPostProcesses;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_LuminancePS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_BloomPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_LuminousRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BlurIntermediateRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BlurredRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_LuminousSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_BlurIntermediateSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_BlurredSRV;
};

#endif