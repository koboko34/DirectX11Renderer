#pragma once

#ifndef APPLICATION_H
#define APPLICATION_H

#include <chrono>
#include <vector>
#include <memory>

#include "Graphics.h"
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "Light.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.f;
const float SCREEN_NEAR = 0.3f;

class PostProcess;

class Application
{
private:
	static Application* m_Instance;

	Application();

public:
	static Application* GetSingletonPtr()
	{
		if (!m_Instance)
		{
			m_Instance = new Application();
		}
		return m_Instance;
	}
	
	bool Initialise(int ScreenWidth, int ScreenHeight, HWND hWnd);
	void Shutdown();
	bool Frame();

	HWND GetWindowHandle() const { return m_hWnd; }
	Graphics* GetGraphics() const { return m_Graphics; }

private:
	bool LoadModel(const char* ModelFilename);
	bool Render(double DeltaTime);
	bool RenderScene();
	bool RenderTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView);
	bool RenderPhysicalLight();
	bool RenderPlane();
	void RenderImGui();
	void ApplyPostProcesses(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> CurrentRTV, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> SecondaryRTV,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CurrentSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SecondarySRV, bool& DrawingForward);

private:	
	HWND m_hWnd;

	Graphics* m_Graphics;
	Shader* m_Shader;
	Camera* m_Camera;
	Light* m_Light;

	Model* m_SceneLight;
	Model* m_Plane;
	Model* m_Model;

	DirectX::XMFLOAT3 m_ModelPos;
	DirectX::XMFLOAT3 m_ModelRot;
	DirectX::XMFLOAT3 m_ModelScale;

	const char* m_ModelLoadSuccessMessage = "";
	bool m_ShouldRenderLight = true;
	bool m_ShouldRenderPlane = true;

	std::chrono::steady_clock::time_point m_LastUpdate;
	double m_AppTime;

	std::vector<std::unique_ptr<PostProcess>> m_PostProcesses;
	std::unique_ptr<PostProcess> m_EmptyPostProcess;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TextureResourceView;
};

#endif
