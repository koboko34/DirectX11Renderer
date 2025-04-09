#include <cstdlib>
#include <cwchar>
#include <cstring>
#include <fstream>

#include "InstancedShader.h"
#include "Light.h"
#include "MyMacros.h"

bool InstancedShader::Initialise(ID3D11Device* Device, HWND hWnd)
{
	bool Result;
	wchar_t vsFilename[128];
	wchar_t psFilename[128];
	int Error;

	Error = wcscpy_s(vsFilename, 128, L"Shaders/InstancedPhongVS.hlsl");
	if (Error != 0)
	{
		return false;
	}

	Error = wcscpy_s(psFilename, 128, L"Shaders/PhongPS.hlsl");
	if (Error != 0)
	{
		return false;
	}

	FALSE_IF_FAILED(InitialiseShader(Device, hWnd, vsFilename, psFilename));

	return true;
}

void InstancedShader::Shutdown()
{
	ShutdownShader();
}

bool InstancedShader::InitialiseShader(ID3D11Device* Device, HWND hWnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT hResult;
	Microsoft::WRL::ComPtr<ID3D10Blob> ErrorMessage;
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> psBuffer;
	D3D11_BUFFER_DESC MatrixBufferDesc = {};
	D3D11_BUFFER_DESC LightBufferDesc = {};
	D3D11_BUFFER_DESC InstanceBufferDesc = {};
	D3D11_INPUT_ELEMENT_DESC VertexLayout[7] = {};
	unsigned int NumElements;

	UINT CompileFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	hResult = D3DCompileFromFile(vsFilename, NULL, NULL, "main", "vs_5_0", CompileFlags, 0, &vsBuffer, &ErrorMessage);
	if (FAILED(hResult))
	{
		if (ErrorMessage.Get())
		{
			OutputShaderErrorMessage(ErrorMessage.Get(), hWnd, vsFilename);
		}
		else
		{
			MessageBox(hWnd, vsFilename, L"Missing shader file!", MB_OK);
		}

		return false;
	}

	hResult = D3DCompileFromFile(psFilename, NULL, NULL, "main", "ps_5_0", CompileFlags, 0, &psBuffer, &ErrorMessage);
	if (FAILED(hResult))
	{
		if (ErrorMessage.Get())
		{
			OutputShaderErrorMessage(ErrorMessage.Get(), hWnd, psFilename);
		}
		else
		{
			MessageBox(hWnd, psFilename, L"Missing shader file!", MB_OK);
		}

		return false;
	}

	HFALSE_IF_FAILED(Device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), NULL, &m_VertexShader));
	HFALSE_IF_FAILED(Device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &m_PixelShader));

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

	VertexLayout[3].SemanticName = "INSTANCE_TRANSFORM";
	VertexLayout[3].SemanticIndex = 0;
	VertexLayout[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	VertexLayout[3].InputSlot = 1;
	VertexLayout[3].AlignedByteOffset = 0;
	VertexLayout[3].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	VertexLayout[3].InstanceDataStepRate = 1;

	VertexLayout[4].SemanticName = "INSTANCE_TRANSFORM";
	VertexLayout[4].SemanticIndex = 1;
	VertexLayout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	VertexLayout[4].InputSlot = 1;
	VertexLayout[4].AlignedByteOffset = 16;
	VertexLayout[4].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	VertexLayout[4].InstanceDataStepRate = 1;

	VertexLayout[5].SemanticName = "INSTANCE_TRANSFORM";
	VertexLayout[5].SemanticIndex = 2;
	VertexLayout[5].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	VertexLayout[5].InputSlot = 1;
	VertexLayout[5].AlignedByteOffset = 32;
	VertexLayout[5].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	VertexLayout[5].InstanceDataStepRate = 1;

	VertexLayout[6].SemanticName = "INSTANCE_TRANSFORM";
	VertexLayout[6].SemanticIndex = 3;
	VertexLayout[6].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	VertexLayout[6].InputSlot = 1;
	VertexLayout[6].AlignedByteOffset = 48;
	VertexLayout[6].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	VertexLayout[6].InstanceDataStepRate = 1;

	NumElements = _countof(VertexLayout);

	HFALSE_IF_FAILED(Device->CreateInputLayout(VertexLayout, NumElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));

	MatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	MatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HFALSE_IF_FAILED(Device->CreateBuffer(&MatrixBufferDesc, NULL, &m_MatrixBuffer));

	LightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	LightBufferDesc.ByteWidth = sizeof(LightingBuffer);
	LightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	LightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HFALSE_IF_FAILED(Device->CreateBuffer(&LightBufferDesc, NULL, &m_LightingBuffer));

	InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	InstanceBufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * 10; // hard coded value for now to test
	InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HFALSE_IF_FAILED(Device->CreateBuffer(&InstanceBufferDesc, NULL, &m_InstanceBuffer));

	return true;
}

void InstancedShader::ShutdownShader()
{

}

void InstancedShader::OutputShaderErrorMessage(ID3D10Blob* ErrorMessage, HWND hWnd, WCHAR* ShaderFilename)
{
	char* CompileErrors;
	unsigned long long BufferSize, i;
	std::ofstream fout;

	CompileErrors = (char*)(ErrorMessage->GetBufferPointer());
	BufferSize = ErrorMessage->GetBufferSize();
	fout.open("shader-error.txt");

	const char* str1 = "Error compiling shader!\n\n";

	size_t len1 = std::strlen(str1) + 1;
	size_t len2 = std::strlen(CompileErrors) + 1;

	wchar_t* wstr1 = new wchar_t[len1];
	wchar_t* wstr2 = new wchar_t[len2];

	size_t Converted1, Converted2;
	mbstowcs_s(&Converted1, wstr1, len1, str1, len1 - 1);
	mbstowcs_s(&Converted2, wstr2, len2, CompileErrors, len2 - 1);

	size_t CombinedLen = len1 + len2 - 1;
	wchar_t* CombinedWstr = new wchar_t[CombinedLen];

	wcscpy_s(CombinedWstr, CombinedLen, wstr1);
	wcscat_s(CombinedWstr, CombinedLen, wstr2);

	for (i = 0; i < BufferSize; i++)
	{
		fout << CompileErrors[i];
	}

	fout.close();

	ErrorMessage->Release();
	ErrorMessage = 0;

	MessageBox(hWnd, CombinedWstr, ShaderFilename, MB_OK);

	delete[] wstr1;
	delete[] wstr2;
	delete[] CombinedWstr;
}

bool InstancedShader::SetShaderParameters(ID3D11DeviceContext* DeviceContext, const std::vector<DirectX::XMMATRIX>& Transforms, DirectX::XMMATRIX View, DirectX::XMMATRIX Projection,
	DirectX::XMFLOAT3 CameraPos, const std::vector<Light*>& Lights)
{
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	MatrixBuffer* MatrixDataPtr;
	LightingBuffer* LightingDataPtr;
	unsigned int vsBufferSlot = 0u;
	unsigned int psBufferSlot = 0u;

	// remember to transpose from row major before sending to shaders
	View = DirectX::XMMatrixTranspose(View);
	Projection = DirectX::XMMatrixTranspose(Projection);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_MatrixBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	MatrixDataPtr = (MatrixBuffer*)MappedResource.pData;
	MatrixDataPtr->ViewMatrix = View;
	MatrixDataPtr->ProjectionMatrix = Projection;
	DeviceContext->Unmap(m_MatrixBuffer.Get(), 0u);

	DeviceContext->VSSetConstantBuffers(vsBufferSlot, 1u, m_MatrixBuffer.GetAddressOf());
	vsBufferSlot++;

	ASSERT_NOT_FAILED(DeviceContext->Map(m_LightingBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));
	LightingDataPtr = (LightingBuffer*)MappedResource.pData;
	LightingDataPtr->CameraPos = CameraPos;

	int NumPointLights = 0;
	for (int i = 0; i < Lights.size(); i++)
	{
		PointLight* pLight = dynamic_cast<PointLight*>(Lights[i]);
		if (pLight)
		{
			assert(NumPointLights < MAX_POINT_LIGHTS);
			LightingDataPtr->PointLights[NumPointLights].LightColor = pLight->GetDiffuseColor();
			LightingDataPtr->PointLights[NumPointLights].LightPos = pLight->GetPosition();
			LightingDataPtr->PointLights[NumPointLights].Radius = pLight->GetRadius();
			LightingDataPtr->PointLights[NumPointLights].SpecularPower = pLight->GetSpecularPower();

			NumPointLights++;
			continue;
		}
	}

	LightingDataPtr->PointLightCount = NumPointLights;
	DeviceContext->Unmap(m_LightingBuffer.Get(), 0u);

	DeviceContext->PSSetConstantBuffers(psBufferSlot, 1u, m_LightingBuffer.GetAddressOf());
	psBufferSlot++;

	ASSERT_NOT_FAILED(DeviceContext->Map(m_InstanceBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));
	memcpy(MappedResource.pData, Transforms.data(), sizeof(InstanceData) * Transforms.size());
	DeviceContext->Unmap(m_InstanceBuffer.Get(), 0u);

	return true;
}

void InstancedShader::ActivateShader(ID3D11DeviceContext* DeviceContext)
{
	DeviceContext->IASetInputLayout(m_InputLayout.Get());

	DeviceContext->VSSetShader(m_VertexShader.Get(), NULL, 0u);
	DeviceContext->PSSetShader(m_PixelShader.Get(), NULL, 0u);
}
