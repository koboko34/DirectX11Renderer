#pragma once

#ifndef INSTANCED_SHADER_H
#define INSTANCED_SHADER_H

#include <vector>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <wrl.h>

#include "Common.h"

class PointLight;
class DirectionalLight;

struct InstanceData
{
	DirectX::XMMATRIX Transform;
};

class InstancedShader
{
private:
	struct MatrixBuffer
	{
		DirectX::XMMATRIX ViewMatrix;
		DirectX::XMMATRIX ProjectionMatrix;
	};

	struct PointLightData
	{
		float Radius;
		DirectX::XMFLOAT3 LightPos;
		float SpecularPower;
		DirectX::XMFLOAT3 LightColor;
	};

	struct DirectionalLightData
	{
		DirectX::XMFLOAT3 LightDir;
		float SpecularPower;
		DirectX::XMFLOAT3 LightColor;
		float Padding = 0.f;
	};

	struct LightingBuffer
	{
		PointLightData PointLights[MAX_POINT_LIGHTS];
		DirectionalLightData DirLight;
		DirectX::XMFLOAT3 CameraPos;
		int PointLightCount = 0;
	};

public:
	InstancedShader() {};

	bool Initialise(ID3D11Device* Device, HWND hWnd);
	void Shutdown();
	void ActivateShader(ID3D11DeviceContext* DeviceContext);
	bool SetShaderParameters(ID3D11DeviceContext* DeviceContext, const std::vector<DirectX::XMMATRIX>& Transforms, const DirectX::XMMATRIX& View, const DirectX::XMMATRIX& Projection,
		const DirectX::XMFLOAT3& CameraPos, const std::vector<PointLight*>& PointLights, DirectionalLight* pDirLight);

	static void OutputShaderErrorMessage(ID3D10Blob* ErrorMessage, HWND hWnd, WCHAR* ShaderFilename);

	Microsoft::WRL::ComPtr<ID3D11InputLayout> GetInputLayout() const { return m_InputLayout; }
	Microsoft::WRL::ComPtr<ID3D11Buffer>& GetInstanceBuffer() { return m_InstanceBuffer; }

private:
	bool InitialiseShader(ID3D11Device* Device, HWND hWnd, WCHAR* vsFilename, WCHAR* psFilename);
	void ShutdownShader();

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_MatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_LightingBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_InstanceBuffer;
};

#endif
