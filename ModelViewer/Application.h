#pragma once

#ifndef APPLICATION_H
#define APPLICATION_H

#include <chrono>
#include <vector>
#include <memory>

#include "Graphics.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.f;
const float SCREEN_NEAR = 0.1f;

class Shader;
class InstancedShader;
class Model;
class Light;
class Camera;
class PostProcess;
class GameObject;
class Skybox;
class Landscape;
class FrustumRenderer;

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

	void SetActiveCamera(int ID);

	HWND GetWindowHandle() const { return m_hWnd; }
	Graphics* GetGraphics() const { return m_Graphics; }

	std::shared_ptr<Camera> GetActiveCamera() { return m_ActiveCamera; }
	std::shared_ptr<Camera> GetMainCamera() { return m_MainCamera; }
	int GetActiveCameraID() { return m_ActiveCameraID; }

	InstancedShader* GetInstancedShader() { return m_InstancedShader.get(); }

	std::vector<std::shared_ptr<GameObject>>& GetGameObjects() { return m_GameObjects; }
	std::vector<std::shared_ptr<Camera>>& GetCameras() { return m_Cameras; }
	std::vector<std::unique_ptr<PostProcess>>& GetPostProcesses() { return m_PostProcesses; }

private:
	bool Render(double DeltaTime);
	bool RenderScene();
	void RenderModels();
	bool RenderTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView);

	void RenderImGui();

	void ApplyPostProcesses(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> CurrentRTV, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> SecondaryRTV,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CurrentSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SecondarySRV, bool& DrawingForward);

	void ProcessInput();
	void ToggleShowCursor();

private:	
	HWND m_hWnd;

	Graphics* m_Graphics;

	std::unique_ptr<Shader> m_Shader;
	std::unique_ptr<InstancedShader> m_InstancedShader;
	std::unique_ptr<FrustumRenderer> m_FrustumRenderer;
	std::unique_ptr<Skybox> m_Skybox;
	std::shared_ptr<Landscape> m_Landscape;
	std::shared_ptr<Camera> m_ActiveCamera;
	std::shared_ptr<Camera> m_MainCamera;

	std::vector<std::shared_ptr<GameObject>> m_GameObjects;
	std::vector<std::shared_ptr<Camera>> m_Cameras;
	std::vector<std::unique_ptr<PostProcess>> m_PostProcesses;

	std::chrono::steady_clock::time_point m_LastUpdate;
	double m_AppTime;
	float m_CameraSpeed = 0.5f;
	float m_CameraSpeedMin = 0.125f;
	float m_CameraSpeedMax = 2.f;
	int m_ActiveCameraID;
	bool m_bShowCursor = false;
	bool m_bCursorToggleReleased = true;

	const char* m_QuadTexturePath = "Textures/image_gamma_linear.png";
	ID3D11ShaderResourceView* m_TextureResourceView;
};

#endif
