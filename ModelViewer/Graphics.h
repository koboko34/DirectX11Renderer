#pragma once

#ifndef GRAPHICS_H
#define GRAPHICS_H

// LINKING //
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "assimp-vc143-mt.lib")

#include <d3d11.h>
#include <DirectXMath.h>

class Graphics
{
public:
	Graphics();
	Graphics(const Graphics&);
	~Graphics();

	bool Initialise(int ScreenWidth, int ScreenHeight, bool VSync, HWND hwnd, bool Fullscreen, float ScreenDepth, float ScreenNear);
	void Shutdown();

	void BeginScene(float Red, float Green, float Blue, float Alpha);
	void EndScene();

	void GetVideoCardInfo(char* CardName, int& Memory);

	void SetBackBufferRenderTarget();
	void ResetViewport();

private:
	bool m_VSync_Enabled;
	int m_VideoCardMemory;
	char m_VideoCardDescription[128];
	IDXGISwapChain* m_SwapChain;
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;
	ID3D11RenderTargetView* m_RenderTargetView;
	ID3D11Texture2D* m_DepthStencilBuffer;
	ID3D11DepthStencilState* m_DepthStencilState;
	ID3D11DepthStencilView* m_DepthStencilView;
	ID3D11RasterizerState* m_RasterState;
	DirectX::XMMATRIX m_ProjectionMatrix;
	DirectX::XMMATRIX m_WorldMatrix;
	DirectX::XMMATRIX m_OrthoMatrix;
	D3D11_VIEWPORT m_Viewport;

public:
	ID3D11Device* GetDevice() const { return m_Device; }
	ID3D11DeviceContext* GetDeviceContext() const { return m_DeviceContext; }

	void GetWorldMatrix(DirectX::XMMATRIX& WorldMatrix) { WorldMatrix = m_WorldMatrix; }
	void GetProjectionMatrix(DirectX::XMMATRIX& ProjectionMatrix) { ProjectionMatrix = m_ProjectionMatrix; }
	void GetOrthoMatrix(DirectX::XMMATRIX& OrthoMatrix) { OrthoMatrix = m_OrthoMatrix; }
};

#endif
