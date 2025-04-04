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

	InstancedShader* GetInstancedShader() { return m_InstancedShader; }

private:
	std::shared_ptr<Model> LoadModel(const char* ModelFilename, const char* TexturesPath);
	bool Render(double DeltaTime);
	bool RenderScene();
	void RenderModels();
	bool RenderTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView);
	void RenderPhysicalLight();
	void RenderImGui();
	void ApplyPostProcesses(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> CurrentRTV, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> SecondaryRTV,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CurrentSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SecondarySRV, bool& DrawingForward);

	void ProcessInput();

private:	
	HWND m_hWnd;

	Graphics* m_Graphics						= nullptr;
	Shader* m_Shader							= nullptr;
	InstancedShader* m_InstancedShader			= nullptr;
	Camera* m_Camera							= nullptr;
	Light* m_PointLight							= nullptr;
	std::shared_ptr<GameObject> m_LightObject;

	Model* m_SceneLight;
	Skybox* m_Skybox;

	std::vector<std::shared_ptr<Model>> m_Models;
	std::vector<std::shared_ptr<GameObject>> m_GameObjects;

	const char* m_ModelLoadSuccessMessage = "";
	bool m_bShouldRenderLight = true;

	std::chrono::steady_clock::time_point m_LastUpdate;
	double m_AppTime;
	float m_CameraSpeed = 0.5f;

	std::vector<std::unique_ptr<PostProcess>> m_PostProcesses;
	std::unique_ptr<PostProcess> m_EmptyPostProcess;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TextureResourceView;
};

#endif
