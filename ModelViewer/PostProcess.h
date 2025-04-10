#pragma once

#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <iostream>
#include <cassert>

#include "ImGui/imgui.h"

#include "Shader.h"
#include "MyMacros.h"
#include "Graphics.h"
#include "Application.h"

class PostProcessEmpty;

class PostProcess
{
public:
	void ApplyPostProcess(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV, ID3D11DepthStencilView* DSV)
	{
		ID3D11RenderTargetView* NullRTVs[] = { nullptr };
		Graphics::GetSingletonPtr()->GetDeviceContext()->OMSetRenderTargets(1u, NullRTVs, DSV);
		ID3D11ShaderResourceView* NullSRVs[] = { nullptr, nullptr };
		Graphics::GetSingletonPtr()->GetDeviceContext()->PSSetShaderResources(0u, 2u, NullSRVs);
		ApplyPostProcessImpl(DeviceContext, RTV, SRV, DSV);
	}

	virtual ~PostProcess() {}

	virtual void RenderControls()
	{
		ImGui::Text("Controls not set up for this post process!");
	}

	static void ShutdownStatics()
	{
		ms_QuadVertexShader.Reset();
		ms_QuadInputLayout.Reset();
		ms_QuadVertexBuffer.Reset();
		ms_QuadIndexBuffer.Reset();
		ms_EmptyPostProcess.reset();
		ms_bInitialised = false;
	}

	static Microsoft::WRL::ComPtr<ID3D11VertexShader> GetQuadVertexShader()
	{
		if (ms_bInitialised)
		{
			return PostProcess::ms_QuadVertexShader;
		}

		InitialiseShaderResources();
		return PostProcess::ms_QuadVertexShader;
	}

	static Microsoft::WRL::ComPtr<ID3D11Buffer> GetQuadVertexBuffer()
	{
		if (ms_bInitialised)
		{
			return PostProcess::ms_QuadVertexBuffer;
		}

		InitialiseShaderResources();
		return PostProcess::ms_QuadVertexBuffer;
	}

	static Microsoft::WRL::ComPtr<ID3D11Buffer> GetQuadIndexBuffer()
	{
		if (ms_bInitialised)
		{
			return PostProcess::ms_QuadIndexBuffer;
		}

		InitialiseShaderResources();
		return PostProcess::ms_QuadIndexBuffer;
	}

	static Microsoft::WRL::ComPtr<ID3D11InputLayout> GetQuadInputLayout()
	{
		if (ms_bInitialised)
		{
			return PostProcess::ms_QuadInputLayout;
		}

		InitialiseShaderResources();
		return PostProcess::ms_QuadInputLayout;
	}

	static std::shared_ptr<PostProcessEmpty> GetEmptyPostProcess()
	{
		if (ms_EmptyPostProcess.get())
		{
			return PostProcess::ms_EmptyPostProcess;
		}

		ms_EmptyPostProcess = std::make_shared<PostProcessEmpty>();
		return PostProcess::ms_EmptyPostProcess;
	}

	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader() const { return m_PixelShader; };

	void Activate() { m_bActive = true; }
	void Deactivate() { m_bActive = false; }
	bool& GetIsActive() { return m_bActive; }
	const std::string& GetName() const { return m_Name; }

protected:
	struct Vertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexCoord;
	};
	
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	bool m_bActive = true;
	std::string m_Name = "";
	
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

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &PixelShader));
		PixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)wcslen(PSFilepath), PSFilepath);

		return true;
	}

private:
	static void InitialiseShaderResources()
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

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), NULL, &ms_QuadVertexShader));

		VertexLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		VertexLayout[0].SemanticName = "POSITION";
		VertexLayout[0].SemanticIndex = 0;
		VertexLayout[0].InputSlot = 0;
		VertexLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		VertexLayout[0].AlignedByteOffset = 0;
		VertexLayout[0].InstanceDataStepRate = 0;

		VertexLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		VertexLayout[1].SemanticName = "NORMAL";
		VertexLayout[1].SemanticIndex = 0;
		VertexLayout[1].InputSlot = 0;
		VertexLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		VertexLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		VertexLayout[1].InstanceDataStepRate = 0;

		VertexLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		VertexLayout[2].SemanticName = "TEXCOORD";
		VertexLayout[2].SemanticIndex = 0;
		VertexLayout[2].InputSlot = 0;
		VertexLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		VertexLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		VertexLayout[2].InstanceDataStepRate = 0;

		NumElements = sizeof(VertexLayout) / sizeof(VertexLayout[0]);
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateInputLayout(VertexLayout, NumElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &ms_QuadInputLayout));

		Vertex QuadVertices[] = {
			{ DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f), },
			{ DirectX::XMFLOAT3( 1.0f,  1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 0.0f), },
			{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f), },
			{ DirectX::XMFLOAT3( 1.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f), },
		};

		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.ByteWidth = sizeof(QuadVertices);
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = QuadVertices;

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &InitData, &ms_QuadVertexBuffer));

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

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &InitData, &ms_QuadIndexBuffer));

		ms_bInitialised = true;
	}

	static Microsoft::WRL::ComPtr<ID3D11VertexShader> ms_QuadVertexShader;
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> ms_QuadInputLayout;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> ms_QuadVertexBuffer;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> ms_QuadIndexBuffer;
	static std::shared_ptr<PostProcessEmpty> ms_EmptyPostProcess;
	static bool ms_bInitialised;
};


/////////////////////////////////////////////////////////////////////////////////

class PostProcessEmpty : public PostProcess
{
public:
	PostProcessEmpty()
	{
		m_Name = "Empty";
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
	PostProcessFog()
	{
		m_Name = "Fog";
		// pass whatever we need into here
	} 

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
private:
	struct BlurData
	{
		DirectX::XMFLOAT2 TexelSize;
		int BlurStrength;
		float Padding;
	};

public:
	PostProcessBoxBlur(int BlurStrength)
	{
		assert(BlurStrength > 0);
		m_Name = "Box Blur";
		
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(BlurData);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		std::pair<int, int> Dimensions = Graphics::GetSingletonPtr()->GetRenderTargetDimensions();
		m_BlurData = {};
		m_BlurData.TexelSize = DirectX::XMFLOAT2(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second);
		m_BlurData.BlurStrength = BlurStrength;
		m_LastBlurData = m_BlurData;

		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &m_BlurData;

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

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
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &IntermediateTexture));
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateRenderTargetView(IntermediateTexture, NULL, &m_IntermediateRTV));
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateShaderResourceView(IntermediateTexture, NULL, &m_IntermediateSRV));

		IntermediateTexture->Release();
	}

	void RenderControls() override
	{
		ImGui::Text("Blur Strength is currently hard coded in the shader. Fix it eventually.");
		ImGui::SliderInt("Blur Strength", &m_BlurData.BlurStrength, 1, 32, "%.d", ImGuiSliderFlags_AlwaysClamp);
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		// horizontal
		DeviceContext->PSSetShader(m_HorizontalPS.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		if (memcmp(&m_LastBlurData, &m_BlurData, sizeof(BlurData)) != 0)
		{
			UpdateBuffer();
			m_LastBlurData = m_BlurData;
		}

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

	void UpdateBuffer()
	{
		HRESULT hResult;
		D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDeviceContext()->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedSubresource));
		memcpy(MappedSubresource.pData, &m_BlurData, sizeof(BlurData));
		Graphics::GetSingletonPtr()->GetDeviceContext()->Unmap(m_ConstantBuffer.Get(), 0u);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_HorizontalPS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_VerticalPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_IntermediateRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_IntermediateSRV;

	BlurData m_BlurData;
	BlurData m_LastBlurData;
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessGaussianBlur : public PostProcess
{
private:
	struct BlurData
	{
		DirectX::XMFLOAT2 TexelSize;
		int BlurStrength;
		float Sigma;
	};

public:
	PostProcessGaussianBlur(int BlurStrength, float Sigma)
	{
		assert(BlurStrength > 0 && (UINT)BlurStrength <= m_MaxBlurStrength);
		m_Name = "Gaussian Blur";

		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		std::pair<int, int> Dimensions = Graphics::GetSingletonPtr()->GetRenderTargetDimensions();
		m_BlurData = {};
		m_BlurData.TexelSize = DirectX::XMFLOAT2(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second);
		m_BlurData.BlurStrength = BlurStrength;
		m_BlurData.Sigma = Sigma;
		m_LastBlurData = m_BlurData;

		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &m_BlurData;

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		std::vector<float> GaussianWeights(m_MaxBlurStrength + 1, 0.f);
		FillGaussianWeights(GaussianWeights);

		BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(float) * ((UINT)m_MaxBlurStrength + 1);
		BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		BufferDesc.StructureByteStride = sizeof(float);
		BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		BufferData = {};
		BufferData.pSysMem = GaussianWeights.data();

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_GaussianWeightsBuffer));

		D3D11_SHADER_RESOURCE_VIEW_DESC GaussianSRVDesc = {};
		GaussianSRVDesc.Format = DXGI_FORMAT_UNKNOWN; // set to this when using a structured buffer
		GaussianSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		GaussianSRVDesc.Buffer.NumElements = m_MaxBlurStrength + 1;

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateShaderResourceView(m_GaussianWeightsBuffer.Get(), &GaussianSRVDesc, &m_GaussianWeightsSRV));
		
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
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &IntermediateTexture));
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateRenderTargetView(IntermediateTexture, NULL, &m_IntermediateRTV));
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateShaderResourceView(IntermediateTexture, NULL, &m_IntermediateSRV));

		IntermediateTexture->Release();
	}

	void RenderControls() override
	{
		ImGui::SliderInt("Blur Strength", &m_BlurData.BlurStrength, 0, m_MaxBlurStrength, "%.d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Sigma", &m_BlurData.Sigma, 1.f, 8.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
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

		if (memcmp(&m_LastBlurData, &m_BlurData, sizeof(BlurData)) != 0)
		{
			UpdateBuffers();
			m_LastBlurData = m_BlurData;
		}

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

	void UpdateBuffers()
	{
		HRESULT hResult;
		D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDeviceContext()->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedSubresource));
		memcpy(MappedSubresource.pData, &m_BlurData, sizeof(BlurData));
		Graphics::GetSingletonPtr()->GetDeviceContext()->Unmap(m_ConstantBuffer.Get(), 0u);

		MappedSubresource = {};
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDeviceContext()->Map(m_GaussianWeightsBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedSubresource));
		std::vector<float> GaussianWeights(m_BlurData.BlurStrength + 1, 0.f);
		FillGaussianWeights(GaussianWeights);
		memcpy(MappedSubresource.pData, GaussianWeights.data(), (m_BlurData.BlurStrength + 1) * sizeof(float));
		Graphics::GetSingletonPtr()->GetDeviceContext()->Unmap(m_GaussianWeightsBuffer.Get(), 0u);
	}

	float CalcGaussianWeight(int x)
	{
		float AdjustedSigma = m_BlurData.Sigma + 0.2f * abs(x);
		return expf(-0.5f * (x * x) / (AdjustedSigma * AdjustedSigma));
	}

	void FillGaussianWeights(std::vector<float>& GaussianWeights)
	{
		float Sum = 0.f;
		for (int i = 0; i <= m_BlurData.BlurStrength; i++)
		{
			GaussianWeights[i] = CalcGaussianWeight(i);
			Sum += GaussianWeights[i];
		}

		// normalise the weights so that they sum to 1
		for (int i = 0; i <= m_BlurData.BlurStrength; i++)
		{
			GaussianWeights[i] /= Sum;
		}
	}

private:
	BlurData m_BlurData;
	BlurData m_LastBlurData;
	const UINT m_MaxBlurStrength = 100;

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
	PostProcessPixelation(float pixelSize) : m_PixelSize(pixelSize), m_LastPixelSize(pixelSize)
	{
		m_Name = "Pixelation";
		
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		std::pair<int, int> Dimensions = Graphics::GetSingletonPtr()->GetRenderTargetDimensions();
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second, m_PixelSize, 0.f);
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		SetupPixelShader(m_PixelShader, L"Shaders/PixelationPS.hlsl");
	}

	void RenderControls() override
	{
		ImGui::SliderFloat("Pixel Size", &m_PixelSize, 1.f, 32.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		if (m_LastPixelSize != m_PixelSize)
		{
			UpdateBuffer();
			m_LastPixelSize = m_PixelSize;
		}

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

	void UpdateBuffer()
	{
		HRESULT hResult;
		std::pair<int, int> Dimensions = Graphics::GetSingletonPtr()->GetRenderTargetDimensions();
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(1.f / (float)Dimensions.first, 1.f / (float)Dimensions.second, m_PixelSize, 0.f);
		D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDeviceContext()->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedSubresource));
		memcpy(MappedSubresource.pData, &Data, sizeof(DirectX::XMFLOAT4));
		Graphics::GetSingletonPtr()->GetDeviceContext()->Unmap(m_ConstantBuffer.Get(), 0u);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;

	float m_PixelSize;
	float m_LastPixelSize;
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessBloom : public PostProcess
{
public:
	PostProcessBloom(float LuminanceThreshold, int BlurStrength, float Sigma) : m_LuminanceThreshold(LuminanceThreshold), m_LastLuminanceThreshold(LuminanceThreshold)
	{
		m_Name = "Bloom";
		
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(m_LuminanceThreshold, 0.f, 0.f, 0.f);
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		SetupPixelShader(m_LuminancePS, L"Shaders/BloomPS.hlsl", "LuminancePS");
		SetupPixelShader(m_BloomPS, L"Shaders/BloomPS.hlsl", "BloomPS");

		std::pair<int, int> Dimensions = Graphics::GetSingletonPtr()->GetRenderTargetDimensions();
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
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &LuminousTexture));
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &BlurredTexture));

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateRenderTargetView(LuminousTexture, NULL, &m_LuminousRTV));
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateRenderTargetView(BlurredTexture, NULL, &m_BlurredRTV));

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateShaderResourceView(LuminousTexture, NULL, &m_LuminousSRV));
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateShaderResourceView(BlurredTexture, NULL, &m_BlurredSRV));

		LuminousTexture->Release();
		BlurredTexture->Release();

		m_BlurPostProcess = std::make_unique<PostProcessGaussianBlur>(BlurStrength, Sigma);
	}

	void RenderControls() override
	{
		ImGui::SliderFloat("Luminance Threshold", &m_LuminanceThreshold, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		m_BlurPostProcess->RenderControls();
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{		
		// render luminous pixels
		DeviceContext->PSSetShader(m_LuminancePS.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		if (m_LastLuminanceThreshold != m_LuminanceThreshold)
		{
			UpdateBuffer();
			m_LastLuminanceThreshold = m_LuminanceThreshold;
		}

		DeviceContext->OMSetRenderTargets(1u, m_LuminousRTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);

		// blur luminous pixels
		m_BlurPostProcess->ApplyPostProcess(DeviceContext, m_BlurredRTV, m_LuminousSRV, DSV);

		// add bloom to original
		ID3D11RenderTargetView* NullRTVs[] = { nullptr };
		Graphics::GetSingletonPtr()->GetDeviceContext()->OMSetRenderTargets(1u, NullRTVs, DSV);

		DeviceContext->PSSetShader(m_BloomPS.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());
		DeviceContext->PSSetShaderResources(1u, 1u, m_BlurredSRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

	void UpdateBuffer()
	{
		HRESULT hResult;
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(m_LuminanceThreshold, 0.f, 0.f, 0.f);
		D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDeviceContext()->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedSubresource));
		memcpy(MappedSubresource.pData, &Data, sizeof(DirectX::XMFLOAT4));
		Graphics::GetSingletonPtr()->GetDeviceContext()->Unmap(m_ConstantBuffer.Get(), 0u);
	}

private:
	float m_LuminanceThreshold;
	float m_LastLuminanceThreshold;
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
private:
	struct ToneMapperData {
		float WhiteLevel;
		float Exposure;
		float Bias;
		int Formula;
	};

public:
	enum ToneMapperFormula {
		ReinhardBasic,
		ReinhardExtended,
		ReinhardExtendedBias,
		NarkowiczACES,
		HillACES,
		None
	};
	
	PostProcessToneMapper(float WhiteLevel, float Exposure, float Bias, ToneMapperFormula Formula)
	{
		assert(Formula >= 0 && Formula < ToneMapperFormula::None);
		m_Name = "Tone Mapper";
		
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		m_ToneMapperData = { WhiteLevel, Exposure, Bias, Formula };
		m_LastToneMapperData = m_ToneMapperData;
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &m_ToneMapperData;

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		SetupPixelShader(m_PixelShader, L"Shaders/ToneMapperPS.hlsl");
	}

	void RenderControls() override
	{
		const char* Formulas[] = { "Reinhard Basic", "Reinhard Extended", "Reinhard Extended Bias", "Narkowicz ACES", "Hill ACES" };
		
		ImGui::Combo("Formula", &m_ToneMapperData.Formula, Formulas, IM_ARRAYSIZE(Formulas));
		switch (m_ToneMapperData.Formula)
		{
		case ReinhardBasic:
			break;
		case ReinhardExtended:
			ImGui::SliderFloat("White Level", &m_ToneMapperData.WhiteLevel, 0.f, 10.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
			break;
		case ReinhardExtendedBias:
			ImGui::SliderFloat("Bias", &m_ToneMapperData.Bias, 0.f, 10.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
			break;
		case NarkowiczACES:
			break;
		case HillACES:
			break;
		default:
			break;
		}
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		if (memcmp(&m_LastToneMapperData, &m_ToneMapperData, sizeof(ToneMapperData)) != 0)
		{
			UpdateBuffer();
			m_LastToneMapperData = m_ToneMapperData;
		}

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

	void UpdateBuffer()
	{
		HRESULT hResult;
		D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDeviceContext()->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedSubresource));
		memcpy(MappedSubresource.pData, &m_ToneMapperData, sizeof(ToneMapperData));
		Graphics::GetSingletonPtr()->GetDeviceContext()->Unmap(m_ConstantBuffer.Get(), 0u);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;

	ToneMapperData m_ToneMapperData;
	ToneMapperData m_LastToneMapperData;
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessGammaCorrection : public PostProcess
{
public:
	PostProcessGammaCorrection(float Gamma) : m_Gamma(Gamma), m_LastGamma(Gamma)
	{
		m_Name = "Gamma Correction";
		
		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		DirectX::XMFLOAT4 Data = { m_Gamma, 0.f, 0.f, 0.f };
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &Data;

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));
		
		SetupPixelShader(m_PixelShader, L"Shaders/GammaCorrectionPS.hlsl");
	}

	void RenderControls() override
	{
		ImGui::SliderFloat("Gamma", &m_Gamma, 1.f, 3.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		if (m_LastGamma != m_Gamma)
		{
			UpdateBuffer();
			m_LastGamma = m_Gamma;
		}

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

	void UpdateBuffer()
	{
		HRESULT hResult;
		DirectX::XMFLOAT4 Data = DirectX::XMFLOAT4(m_Gamma, 0.f, 0.f, 0.f);
		D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDeviceContext()->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedSubresource));
		memcpy(MappedSubresource.pData, &Data, sizeof(DirectX::XMFLOAT4));
		Graphics::GetSingletonPtr()->GetDeviceContext()->Unmap(m_ConstantBuffer.Get(), 0u);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;

	float m_Gamma;
	float m_LastGamma;
};

/////////////////////////////////////////////////////////////////////////////////

class PostProcessColorCorrection : public PostProcess
{
private:
	struct ColorData
	{
		float Contrast;
		float Brightness;
		float Saturation;
		float Padding;
	};

public:
	PostProcessColorCorrection(float Contrast, float Brightness, float Saturation)
	{
		m_Name = "Color Correction";

		HRESULT hResult;
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.ByteWidth = sizeof(ColorData);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		std::pair<int, int> Dimensions = Graphics::GetSingletonPtr()->GetRenderTargetDimensions();
		m_ColorData = { Contrast, Brightness, Saturation, 0.f };
		m_LastColorData = m_ColorData;
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &m_ColorData;

		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));

		SetupPixelShader(m_PixelShader, L"Shaders/ColorCorrectionPS.hlsl");
	}

	void RenderControls() override
	{
		ImGui::SliderFloat("Contrast", &m_ColorData.Contrast, 0.f, 2.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Brightness", &m_ColorData.Brightness, -1.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Saturation", &m_ColorData.Saturation, 0.f, 5.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	}

private:
	void ApplyPostProcessImpl(ID3D11DeviceContext* DeviceContext, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV,
		ID3D11DepthStencilView* DSV) override
	{
		DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		DeviceContext->PSSetShaderResources(0u, 1u, SRV.GetAddressOf());

		DeviceContext->PSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		if (memcmp(&m_LastColorData, &m_ColorData, sizeof(ColorData)) != 0)
		{
			UpdateBuffer();
			m_LastColorData = m_ColorData;
		}

		DeviceContext->OMSetRenderTargets(1u, RTV.GetAddressOf(), DSV);

		DeviceContext->DrawIndexed(6u, 0u, 0);
	}

	void UpdateBuffer()
	{
		HRESULT hResult;
		D3D11_MAPPED_SUBRESOURCE MappedSubresource = {};
		ASSERT_NOT_FAILED(Graphics::GetSingletonPtr()->GetDeviceContext()->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedSubresource));
		memcpy(MappedSubresource.pData, &m_ColorData, sizeof(ColorData));
		Graphics::GetSingletonPtr()->GetDeviceContext()->Unmap(m_ConstantBuffer.Get(), 0u);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;

	ColorData m_ColorData;
	ColorData m_LastColorData;
};

#endif