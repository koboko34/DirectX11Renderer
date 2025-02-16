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

#include <wrl.h>

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
	void EnableDepthWrite();
	void DisableDepthWrite();
	void ResetViewport();

private:
	bool m_VSync_Enabled;
	int m_VideoCardMemory;
	char m_VideoCardDescription[128];
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BackBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilStateWriteEnabled;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilStateWriteDisabled;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RasterState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;

	DirectX::XMMATRIX m_ProjectionMatrix;
	DirectX::XMMATRIX m_WorldMatrix;
	DirectX::XMMATRIX m_OrthoMatrix;
	D3D11_VIEWPORT m_Viewport;

public:
	ID3D11Device* GetDevice() const { return m_Device.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return m_DeviceContext.Get(); }

	ID3D11DepthStencilView* GetDepthStencilView() const { return m_DepthStencilView.Get(); }

	void GetWorldMatrix(DirectX::XMMATRIX& WorldMatrix) { WorldMatrix = m_WorldMatrix; }
	void GetProjectionMatrix(DirectX::XMMATRIX& ProjectionMatrix) { ProjectionMatrix = m_ProjectionMatrix; }
	void GetOrthoMatrix(DirectX::XMMATRIX& OrthoMatrix) { OrthoMatrix = m_OrthoMatrix; }

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_PostProcessRTVFirst;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_PostProcessRTVSecond;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_PostProcessSRVFirst;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_PostProcessSRVSecond;
};

#endif
