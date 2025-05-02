#include "InstancedShader.h"
#include "Light.h"
#include "MyMacros.h"
#include "Common.h"
#include "ResourceManager.h"

InstancedShader::~InstancedShader()
{
	Shutdown();
}

bool InstancedShader::Initialise(ID3D11Device* Device)
{
	bool Result;
	m_vsFilename = "Shaders/InstancedPhongVS.hlsl";
	m_psFilename = "Shaders/PhongPS.hlsl";

	FALSE_IF_FAILED(InitialiseShader(Device));

	return true;
}

void InstancedShader::Shutdown()
{
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11VertexShader>(m_vsFilename, "main");
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11PixelShader>(m_psFilename, "main");
}

bool InstancedShader::InitialiseShader(ID3D11Device* Device)
{
	HRESULT hResult;
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;
	D3D11_BUFFER_DESC MatrixBufferDesc = {};
	D3D11_BUFFER_DESC LightBufferDesc = {};
	D3D11_BUFFER_DESC InstanceBufferDesc = {};
	D3D11_INPUT_ELEMENT_DESC VertexLayout[7] = {};
	unsigned int NumElements;

	m_VertexShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11VertexShader>(m_vsFilename, "main", vsBuffer);
	m_PixelShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11PixelShader>(m_psFilename, "main");

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
	NAME_D3D_RESOURCE(m_InputLayout, "Instanced shader input layout");

	MatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	MatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HFALSE_IF_FAILED(Device->CreateBuffer(&MatrixBufferDesc, NULL, &m_MatrixBuffer));
	NAME_D3D_RESOURCE(m_MatrixBuffer, "Instanced shader matrix buffer");

	LightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	LightBufferDesc.ByteWidth = sizeof(LightingBuffer);
	LightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	LightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HFALSE_IF_FAILED(Device->CreateBuffer(&LightBufferDesc, NULL, &m_LightingBuffer));
	NAME_D3D_RESOURCE(m_LightingBuffer, "Instanced shader lighting buffer");

	InstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	InstanceBufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * MAX_INSTANCE_COUNT;
	InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HFALSE_IF_FAILED(Device->CreateBuffer(&InstanceBufferDesc, NULL, &m_InstanceBuffer));
	NAME_D3D_RESOURCE(m_InstanceBuffer, "Instanced shader instance buffer");

	return true;
}

bool InstancedShader::SetShaderParameters(ID3D11DeviceContext* DeviceContext, const DirectX::XMMATRIX& View, const DirectX::XMMATRIX& Projection, const DirectX::XMFLOAT3& CameraPos,
	const std::vector<PointLight*>& PointLights, const std::vector<DirectionalLight*>& DirLights, const DirectX::XMFLOAT3& SkylightColor)
{
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	MatrixBuffer* MatrixDataPtr;
	LightingBuffer* LightingDataPtr;
	unsigned int vsBufferSlot = 0u;
	unsigned int psBufferSlot = 0u;

	// remember to transpose from row major before sending to shaders
	ASSERT_NOT_FAILED(DeviceContext->Map(m_MatrixBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));
	MatrixDataPtr = (MatrixBuffer*)MappedResource.pData;
	MatrixDataPtr->ViewMatrix = DirectX::XMMatrixTranspose(View);
	MatrixDataPtr->ProjectionMatrix = DirectX::XMMatrixTranspose(Projection);
	DeviceContext->Unmap(m_MatrixBuffer.Get(), 0u);

	DeviceContext->VSSetConstantBuffers(vsBufferSlot, 1u, m_MatrixBuffer.GetAddressOf());
	vsBufferSlot++;

	ASSERT_NOT_FAILED(DeviceContext->Map(m_LightingBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));
	LightingDataPtr = (LightingBuffer*)MappedResource.pData;
	LightingDataPtr->CameraPos = CameraPos;
	LightingDataPtr->SkylightColor = SkylightColor;

	int NumDirLights = 0;
	for (int i = 0; i < DirLights.size(); i++)
	{
		assert(NumDirLights < MAX_POINT_LIGHTS);
		LightingDataPtr->DirLights[NumDirLights].LightColor = DirLights[NumDirLights]->GetDiffuseColor();
		LightingDataPtr->DirLights[NumDirLights].LightDir = DirLights[NumDirLights]->GetDirection();
		LightingDataPtr->DirLights[NumDirLights].SpecularPower = DirLights[NumDirLights]->GetSpecularPower();

		NumDirLights++;
		continue;
	}

	LightingDataPtr->DirLightCount = NumDirLights;

	int NumPointLights = 0;
	for (int i = 0; i < PointLights.size(); i++)
	{
		assert(NumPointLights < MAX_POINT_LIGHTS);
		LightingDataPtr->PointLights[NumPointLights].LightColor = PointLights[NumPointLights]->GetDiffuseColor();
		LightingDataPtr->PointLights[NumPointLights].LightPos = PointLights[NumPointLights]->GetPosition();
		LightingDataPtr->PointLights[NumPointLights].Radius = PointLights[NumPointLights]->GetRadius();
		LightingDataPtr->PointLights[NumPointLights].SpecularPower = PointLights[NumPointLights]->GetSpecularPower();

		NumPointLights++;
		continue;
	}

	LightingDataPtr->PointLightCount = NumPointLights;
	DeviceContext->Unmap(m_LightingBuffer.Get(), 0u);

	DeviceContext->PSSetConstantBuffers(psBufferSlot, 1u, m_LightingBuffer.GetAddressOf());
	psBufferSlot++;

	/*assert(Transforms.size() <= MAX_INSTANCE_COUNT);
	ASSERT_NOT_FAILED(DeviceContext->Map(m_InstanceBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));
	memcpy(MappedResource.pData, Transforms.data(), sizeof(InstanceData) * Transforms.size());
	DeviceContext->Unmap(m_InstanceBuffer.Get(), 0u);*/

	return true;
}

void InstancedShader::ActivateShader(ID3D11DeviceContext* DeviceContext)
{
	DeviceContext->IASetInputLayout(m_InputLayout.Get());

	DeviceContext->VSSetShader(m_VertexShader, NULL, 0u);
	DeviceContext->PSSetShader(m_PixelShader, NULL, 0u);
}
