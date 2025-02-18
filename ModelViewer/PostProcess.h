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
		assert(m_PixelShader.Get());
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

	static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetDepthStencilState(ID3D11Device* Device)
	{
		if (ms_bInitialised)
		{
			return PostProcess::ms_DepthStencilState;
		}

		InitialiseShaderResources(Device);
		return PostProcess::ms_DepthStencilState;
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

	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;

protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
	
	virtual void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) = 0;

	bool SetupPixelShader(const wchar_t* PSFilepath, const char* entryFunc = "main")
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

		hResult = Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &m_PixelShader);
		if (FAILED(hResult))
		{
			return false;
		}

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

		hResult = Device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), NULL, &ms_QuadVertexShader);
		if (FAILED(hResult))
		{
			return;
		}

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
		hResult = Device->CreateInputLayout(VertexLayout, NumElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &ms_QuadInputLayout);
		if (FAILED(hResult))
		{
			return;
		}

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

		hResult = Device->CreateBuffer(&BufferDesc, &InitData, &ms_QuadVertexBuffer);
		if (FAILED(hResult))
		{
			return;
		}

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

		hResult = Device->CreateBuffer(&BufferDesc, &InitData, &ms_QuadIndexBuffer);
		if (FAILED(hResult))
		{
			return;
		}

		DepthStencilDesc.DepthEnable = false;
		DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		DepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		DepthStencilDesc.StencilEnable = false;

		hResult = Device->CreateDepthStencilState(&DepthStencilDesc, &ms_DepthStencilState);
		if (FAILED(hResult))
		{
			return;
		}

		ms_bInitialised = true;
	}

	static Microsoft::WRL::ComPtr<ID3D11VertexShader> ms_QuadVertexShader;
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> ms_QuadInputLayout;
	static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> ms_DepthStencilState;
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
		SetupPixelShader(L"Shaders/QuadPS.hlsl");
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
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		if (FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, nullptr, &m_ConstantBuffer)))
		{
			return;
		}

		SetupPixelShader(L"Shaders/BoxBlurPS.hlsl", "HorizontalPS");
	}

private:
	void UpdateConstantBuffer(ID3D11DeviceContext* DeviceContext)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource = {};
		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		DirectX::XMFLOAT2 TexelSize = DirectX::XMFLOAT2(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second);
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(TexelSize.x, TexelSize.y, (float)m_BlurStrength, 0.f);

		if (FAILED(DeviceContext->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource)))
		{
			return;
		}

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
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		if (FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, nullptr, &m_ConstantBuffer)))
		{
			return;
		}

		SetupPixelShader(L"Shaders/BoxBlurPS.hlsl", "VerticalPS");
	}

private:
	void UpdateConstantBuffer(ID3D11DeviceContext* DeviceContext)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource = {};
		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		DirectX::XMFLOAT2 TexelSize = DirectX::XMFLOAT2(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second);
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(TexelSize.x, TexelSize.y, (float)m_BlurStrength, 0.f);

		if (FAILED(DeviceContext->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource)))
		{
			return;
		}

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
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		std::pair<int, int> Dimensions = Application::GetSingletonPtr()->GetGraphics()->GetRenderTargetDimensions();
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second, m_PixelSize, 0.f);
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		if (FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer)))
		{
			return;
		}

		SetupPixelShader(L"Shaders/PixelationPS.hlsl");
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

#endif