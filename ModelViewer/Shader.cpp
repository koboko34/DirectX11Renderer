#include "Shader.h"

#include <cstdlib>
#include <cwchar>
#include <cstring>
#include <fstream>

#include "MyMacros.h"

Shader::Shader()
{
	m_VertexShader = 0;
	m_PixelShader = 0;
	m_InputLayout = 0;
	m_MatrixBuffer = 0;
}

Shader::Shader(const Shader& Other)
{
}

Shader::~Shader()
{
}

bool Shader::Initialise(ID3D11Device* Device, HWND hWnd)
{
	bool Result;
	wchar_t vsFilename[128];
	wchar_t psFilename[128];
	int Error;
	
	Error = wcscpy_s(vsFilename, 128, L"Shaders/PhongVS.hlsl");
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

void Shader::Shutdown()
{
	ShutdownShader();
}

bool Shader::Render(ID3D11DeviceContext* DeviceContext, unsigned int IndexCount, DirectX::XMMATRIX World, DirectX::XMMATRIX View, DirectX::XMMATRIX Projection,
					DirectX::XMFLOAT3 CameraPos, float Radius, DirectX::XMFLOAT3 LightPos, DirectX::XMFLOAT3 DiffuseColor, float SpecularPower)
{
	bool Result;
	FALSE_IF_FAILED(SetShaderParameters(DeviceContext, World, View, Projection, CameraPos, Radius, LightPos, DiffuseColor, SpecularPower));

	RenderShader(DeviceContext, IndexCount);

	return true;
}

bool Shader::InitialiseShader(ID3D11Device* Device, HWND hWnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT hResult;
	ID3D10Blob* ErrorMessage;
	ID3D10Blob* vsBuffer;
	ID3D10Blob* psBuffer;
	D3D11_BUFFER_DESC MatrixBufferDesc = {};
	D3D11_BUFFER_DESC LightBufferDesc = {};
	D3D11_INPUT_ELEMENT_DESC VertexLayout[3] = {};
	unsigned int NumElements;
	
	UINT CompileFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	hResult = D3DCompileFromFile(vsFilename, NULL, NULL, "main", "vs_5_0", CompileFlags, 0, &vsBuffer, &ErrorMessage);
	if (FAILED(hResult))
	{
		if (ErrorMessage)
		{
			OutputShaderErrorMessage(ErrorMessage, hWnd, vsFilename);
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
		if (ErrorMessage)
		{
			OutputShaderErrorMessage(ErrorMessage, hWnd, psFilename);
		}
		else
		{
			MessageBox(hWnd, psFilename, L"Missing shader file!", MB_OK);
		}

		return false;
	}

	HFALSE_IF_FAILED(Device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), NULL, &m_VertexShader));
	HFALSE_IF_FAILED(Device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &m_PixelShader));

	VertexLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

	VertexLayout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	VertexLayout[2].SemanticName = "NORMAL";
	VertexLayout[2].SemanticIndex = 0;
	VertexLayout[2].InputSlot = 0;
	VertexLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	VertexLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	VertexLayout[2].InstanceDataStepRate = 0;

	NumElements = sizeof(VertexLayout) / sizeof(VertexLayout[0]);

	HFALSE_IF_FAILED(Device->CreateInputLayout(VertexLayout, NumElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));

	vsBuffer->Release();
	psBuffer->Release();

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

	return true;
}

void Shader::ShutdownShader()
{
	if (m_MatrixBuffer)
	{
		m_MatrixBuffer->Release();
		m_MatrixBuffer = 0;
	}

	if (m_PixelShader)
	{
		m_PixelShader->Release();
		m_PixelShader = 0;
	}

	if (m_VertexShader)
	{
		m_VertexShader->Release();
		m_VertexShader = 0;
	}

	if (m_InputLayout)
	{
		m_InputLayout->Release();
		m_InputLayout = 0;
	}
}

void Shader::OutputShaderErrorMessage(ID3D10Blob* ErrorMessage, HWND hWnd, WCHAR* ShaderFilename)
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

bool Shader::SetShaderParameters(ID3D11DeviceContext* DeviceContext, DirectX::XMMATRIX World, DirectX::XMMATRIX View, DirectX::XMMATRIX Projection,
	DirectX::XMFLOAT3 CameraPos, float Radius, DirectX::XMFLOAT3 LightPos, DirectX::XMFLOAT3 DiffuseColor, float SpecularPower)
{
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	MatrixBuffer* MatrixDataPtr;
	LightingBuffer* LightingDataPtr;
	unsigned int vsBufferSlot = 0u;
	unsigned int psBufferSlot = 0u;
	
	// remember to transpose from row major before sending to shaders
	World = DirectX::XMMatrixTranspose(World);
	View = DirectX::XMMatrixTranspose(View);
	Projection = DirectX::XMMatrixTranspose(Projection);
	
	HFALSE_IF_FAILED(DeviceContext->Map(m_MatrixBuffer, 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	MatrixDataPtr = (MatrixBuffer*)MappedResource.pData;
	MatrixDataPtr->WorldMatrix = World;
	MatrixDataPtr->ViewMatrix = View;
	MatrixDataPtr->ProjectionMatrix = Projection;
	DeviceContext->Unmap(m_MatrixBuffer, 0u);

	DeviceContext->VSSetConstantBuffers(vsBufferSlot, 1u, &m_MatrixBuffer);
	vsBufferSlot++;

	HFALSE_IF_FAILED(DeviceContext->Map(m_LightingBuffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));
	LightingDataPtr = (LightingBuffer*)MappedResource.pData;
	LightingDataPtr->CameraPos = CameraPos;
	LightingDataPtr->Radius = Radius;
	LightingDataPtr->LightPos = LightPos;
	LightingDataPtr->SpecularPower = SpecularPower;
	LightingDataPtr->DiffuseColor = DiffuseColor;
	LightingDataPtr->Padding = 0.f;
	DeviceContext->Unmap(m_LightingBuffer, 0u);

	DeviceContext->PSSetConstantBuffers(psBufferSlot, 1u, &m_LightingBuffer);
	psBufferSlot++;

	return true;
}

void Shader::RenderShader(ID3D11DeviceContext* DeviceContext, unsigned int IndexCount)
{
	DeviceContext->IASetInputLayout(m_InputLayout);

	DeviceContext->VSSetShader(m_VertexShader, NULL, 0u);
	DeviceContext->PSSetShader(m_PixelShader, NULL, 0u);

	DeviceContext->DrawIndexed(IndexCount, 0u, 0);
}
