#pragma once

#ifndef GRASS_H
#define GRASS_H

#include "d3d11.h"
#include "DirectXMath.h"

#include "wrl.h"

#include "GameObject.h"

class Landscape;

class Grass : public GameObject
{
public:
	Grass();
	~Grass();

	bool Init(Landscape* pLandscape);

	void Shutdown();

	void Render();
	void RenderControls() override;

	bool ShouldRender() const { return m_bShouldRender; }

private:
	bool CreateBuffers();

private:
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ArgsBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_ArgsBufferUAV;

	Landscape* m_pLandscape;
	bool m_bShouldRender;

	const char* m_vsFilepath;
	const char* m_psFilepath;

};

#endif
