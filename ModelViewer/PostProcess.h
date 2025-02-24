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
		ID3D11ShaderResourceView* NullSRVs[] = { nullptr, nullptr };
		Application::GetSingletonPtr()->GetGraphics()->GetDeviceContext()->PSSetShaderResources(0u, 2u, NullSRVs);
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

class PostProcessBoxBlur : public PostProcess
{
public:
	PostProcessBoxBlur(int BlurStrength)
	{
		assert(BlurStrength > 0);
		
		struct BlurData {
			DirectX::XMFLOAT2 TexelSize;
			int BlurStrength;
			float padding;
		};
		
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(BlurData);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		BlurData Data = {};
		Data.TexelSize = DirectX::XMFLOAT2(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second);
		Data.BlurStrength = BlurStrength;

		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		SetupPixelShader(m_HorizontalPS, L"Shaders/BoxBlurPS.hlsl", "HorizontalPS");
		SetupPixelShader(m_VerticalPS, L"Shaders/BoxBlurPS.hlsl", "VerticalPS");

		D3D11_TEXTURE2D_DESC TextureDesc = {};
		TextureDesc.Width = Dimensions.first;
		TextureDesc.Height = Dimensions.second;
		TextureDesc.MipLevels = 1;
		TextureDesc.ArraySize = 1;
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.Usage = D3D11_USAGE_DEFAULT;
		TextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		ID3D11Texture2D* IntermediateTexture;
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &IntermediateTexture));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateRenderTargetView(IntermediateTexture, NULL, &m_IntermediateRTV));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateShaderResourceView(IntermediateTexture, NULL, &m_IntermediateSRV));
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		// horizontal
		DeviceContext->PSSetShader(m_HorizontalPS.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, m_IntermediateRTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);

		// vertical
		ID3D11RenderTargetView* NullRTVs[] = { nullptr };
		DeviceContext->OMSetRenderTargets(1u, NullRTVs, DSV);

		DeviceContext->PSSetShader(m_VerticalPS.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, m_IntermediateSRV.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_HorizontalPS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_VerticalPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_IntermediateRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_IntermediateSRV;
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessGaussianBlur : public PostProcess
{
public:
	PostProcessGaussianBlur(int BlurStrength, float Sigma)
	{
		assert(BlurStrength > 0);

		struct BlurData {
			DirectX::XMFLOAT2 TexelSize;
			int BlurStrength;
			float padding;
		};

		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		BlurData Data = {};
		Data.TexelSize = DirectX::XMFLOAT2(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second);
		Data.BlurStrength = BlurStrength;

		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		std::vector<float> GaussianWeights(BlurStrength + 1, 0.f);
		BufferDesc.ByteWidth = sizeof(float) * (UINT)GaussianWeights.size();

		float Sum = 0.f;
		for (int i = 0; i <= BlurStrength; i++)
		{
			GaussianWeights[i] = CalcGaussianWeight(i, Sigma);
			Sum += GaussianWeights[i];
		}

		// normalise the weights so that they sum to 1
		for (int i = 0; i < GaussianWeights.size(); i++)
		{
			GaussianWeights[i] /= Sum;
		}

		BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(float) * (UINT)GaussianWeights.size();
		BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		BufferDesc.StructureByteStride = sizeof(float);
		BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		BufferData = {};
		BufferData.pSysMem = GaussianWeights.data();

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_GaussianWeightsBuffer));

		D3D11_SHADER_RESOURCE_VIEW_DESC GaussianSRVDesc = {};
		GaussianSRVDesc.Format = DXGI_FORMAT_UNKNOWN; // set to this when using a structured buffer
		GaussianSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		GaussianSRVDesc.Buffer.NumElements = (UINT)GaussianWeights.size();

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateShaderResourceView(m_GaussianWeightsBuffer.Get(), &GaussianSRVDesc, &m_GaussianWeightsSRV));
		
		SetupPixelShader(m_HorizontalPS, L"Shaders/GaussianBlurPS.hlsl", "HorizontalPS");
		SetupPixelShader(m_VerticalPS, L"Shaders/GaussianBlurPS.hlsl", "VerticalPS");

		D3D11_TEXTURE2D_DESC TextureDesc = {};
		TextureDesc.Width = Dimensions.first;
		TextureDesc.Height = Dimensions.second;
		TextureDesc.MipLevels = 1;
		TextureDesc.ArraySize = 1;
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.Usage = D3D11_USAGE_DEFAULT;
		TextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		ID3D11Texture2D* IntermediateTexture;
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &IntermediateTexture));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateRenderTargetView(IntermediateTexture, NULL, &m_IntermediateRTV));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateShaderResourceView(IntermediateTexture, NULL, &m_IntermediateSRV));
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		// horizontal
		DeviceContext->PSSetShader(m_HorizontalPS.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());
		DeviceContext->PSSetShaderResources(1u, 1u, m_GaussianWeightsSRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, m_IntermediateRTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);

		// vertical
		ID3D11RenderTargetView* NullRTVs[] = { nullptr };
		DeviceContext->OMSetRenderTargets(1u, NullRTVs, DSV);
		
		DeviceContext->PSSetShader(m_VerticalPS.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, m_IntermediateSRV.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

	float CalcGaussianWeight(int x, float Sigma)
	{
		float AdjustedSigma = Sigma + 0.2f * abs(x);
		return expf(-0.5f * (x * x) / (AdjustedSigma * AdjustedSigma));
	}

private:
	int m_BlurStrength;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_HorizontalPS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_VerticalPS;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_GaussianWeightsBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_IntermediateRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_IntermediateSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_GaussianWeightsSRV;
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessPixelation : public PostProcess
{
public:
	PostProcessPixelation(float pixelSize)
	{
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second, pixelSize, 0.f);
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
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
};

/////////////////////////////////////////////////////////////////////////////////

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
		TextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		
		ID3D11Texture2D* LuminousTexture;
		ID3D11Texture2D* BlurredTexture;
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &LuminousTexture));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &BlurredTexture));

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateRenderTargetView(LuminousTexture, NULL, &m_LuminousRTV));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateRenderTargetView(BlurredTexture, NULL, &m_BlurredRTV));

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateShaderResourceView(LuminousTexture, NULL, &m_LuminousSRV));
		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateShaderResourceView(BlurredTexture, NULL, &m_BlurredSRV));

		m_BlurPostProcess = std::make_unique<PostProcessGaussianBlur>(25, 8.f);
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
		m_BlurPostProcess->ApplyPostProcess(DeviceContext, m_BlurredRTV, m_LuminousSRV, DSV);

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
	std::unique_ptr<PostProcess> m_BlurPostProcess;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_LuminancePS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_BloomPS;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_LuminousRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BlurredRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_LuminousSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_BlurredSRV;
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessToneMapper : public PostProcess
{
public:
	enum ToneMapperFormula {
		ReinhardBasic,
		ReinhardExtended,
		ReinhardExtendedBias
	};
	
	PostProcessToneMapper(float WhiteLevel, float Exposure, float Bias, ToneMapperFormula Formula)
	{
		assert(Formula >= 0 && Formula < 3);
		
		struct ToneMapperData {
			float WhiteLevel;
			float Exposure;
			float Bias;
			int Formula;
		};
		
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		ToneMapperData Data = { WhiteLevel, Exposure, Bias, Formula };
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		SetupPixelShader(m_PixelShader, L"Shaders/ToneMapperPS.hlsl");
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
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessGammaCorrection : public PostProcess
{
public:
	PostProcessGammaCorrection(float Gamma)
	{
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		DirectX::XMFLOAT4 Data = { Gamma, 0.f, 0.f, 0.f };
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));
		
		SetupPixelShader(m_PixelShader, L"Shaders/GammaCorrectionPS.hlsl");
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
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
};

#endif