#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

class Shader
{
private:
	struct MatrixBuffer
	{
		DirectX::XMMATRIX WorldMatrix;
		DirectX::XMMATRIX ViewMatrix;
		DirectX::XMMATRIX ProjectionMatrix;
	};

public:
	Shader();
	Shader(const Shader& Other);
	~Shader();

	bool Initialise(ID3D11Device* Device, HWND hWnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* DeviceContext, unsigned int IndexCount, DirectX::XMMATRIX World, DirectX::XMMATRIX View, DirectX::XMMATRIX Projection);

private:
	bool InitialiseShader(ID3D11Device* Device, HWND hWnd, WCHAR* vsFilename, WCHAR* psFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* ErrorMessage, HWND hWnd, WCHAR* ShaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* DeviceContext, DirectX::XMMATRIX World, DirectX::XMMATRIX View, DirectX::XMMATRIX Projection);
	void RenderShader(ID3D11DeviceContext* DeviceContext, unsigned int IndexCount);

private:
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;
	ID3D11InputLayout* m_InputLayout;
	ID3D11Buffer* m_MatrixBuffer;
};

#endif
