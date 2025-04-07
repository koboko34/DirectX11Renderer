#pragma once

#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <string>

#include "wrl.h"

#include "d3d11.h"
#include "DirectXMath.h"

class Skybox
{
public:
	Skybox() {};

	bool Init();
	void Render();
	void Shutdown();

private:
	bool LoadTextures();
	bool CreateBuffers();

private:
	std::string m_TexturesDir = "Textures/skybox/";
	std::vector<std::string> m_FileNames { "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg" };
	std::vector<ID3D11Texture2D*> m_Textures;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;

};

#endif
