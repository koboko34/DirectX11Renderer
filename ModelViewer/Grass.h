#pragma once

#ifndef GRASS_H
#define GRASS_H

#include "d3d11.h"
#include "DirectXMath.h"

#include "wrl.h"

#include "GameObject.h"
#include "AABB.h"

class Landscape;

class Grass : public GameObject
{
private:
	struct WindCBuffer
	{
		float Freq;
		float Amp;
		DirectX::XMFLOAT2 Direction;
		float TimeScale;
		float FreqMultiplier;
		float AmpMultiplier;
		UINT WaveCount;
		float Strength;
		float SwayExponent;
		DirectX::XMFLOAT2 Padding;
	};

public:
	Grass();
	~Grass();

	bool Init(Landscape* pLandscape, UINT GrassDimensionPerChunk);

	void Shutdown();

	void Render();
	void RenderControls() override;

	bool ShouldRender() const { return m_bShouldRender; }
	UINT GetGrassPerChunk() const { return m_GrassPerChunk; }
	const AABB& GetBoundingBox() const { return m_BBox; }

private:
	bool CreateBuffers();
	void GenerateAABB();

	void UpdateBuffers();
	void SetWindDirection(DirectX::XMFLOAT2 WindDir);

private:
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ArgsBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_GrassOffsetsBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_GrassCBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_GrassOffsetsSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_ArgsBufferUAV;

	Landscape* m_pLandscape;
	AABB m_BBox;
	UINT m_GrassPerChunk;
	UINT m_GrassInstanceCount;
	bool m_bShouldRender;
	float m_Freq;
	float m_Amp;
	DirectX::XMFLOAT2 m_WindDir;
	float m_TimeScale;
	float m_FreqMultiplier;
	float m_AmpMultiplier;
	UINT m_WaveCount;
	float m_WindStrength;
	float m_SwayExponent;

	const char* m_vsFilepath;
	const char* m_psFilepath;

};

#endif
