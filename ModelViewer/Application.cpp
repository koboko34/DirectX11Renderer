#include "Application.h"

#include <iostream>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

#include "MyMacros.h"

Application::Application()
{
	m_Graphics = 0;
	m_Shader = 0;
	m_Camera = 0;
	m_Light = 0;
	m_Model = 0;

	m_LastUpdate = std::chrono::steady_clock::now();
	m_AppTime = 0.0;
}

Application::Application(const Application& Other)
{
}

Application::~Application()
{
}

bool Application::Initialise(int ScreenWidth, int ScreenHeight, HWND hWnd)
{
	m_hWnd = hWnd;
	
	m_Graphics = new Graphics();
	bool Result = m_Graphics->Initialise(ScreenWidth, ScreenHeight, VSYNC_ENABLED, hWnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!Result)
	{
		ShowCursor(true);
		MessageBox(hWnd, L"Failed to initialise Graphics object!", L"Error", MB_OK);
		return false;
	}

	m_Camera = new Camera();
	m_Camera->SetPosition(0.f, 0.f, 0.f);
	m_Camera->SetRotation(0.f, 0.f, 0.f);

	m_Shader = new Shader();
	Result = m_Shader->Initialise(m_Graphics->GetDevice(), hWnd);
	if (!Result)
	{
		ShowCursor(true);
		MessageBox(hWnd, L"Failed to initialise Shader object!", L"Error", MB_OK);
		return false;
	}

	char ModelFilename[128];
	strcpy_s(ModelFilename, "Models/sphere.obj");

	m_SceneLight = new Model();
	Result = m_SceneLight->Initialise(m_Graphics->GetDevice(), m_Graphics->GetDeviceContext(), ModelFilename);
	if (!Result)
	{
		ShowCursor(true);
		MessageBox(hWnd, L"Failed to initialise scene light object!", L"Error", MB_OK);
		return false;
	}

	//FALSE_IF_FAILED(AddModel(hWnd, "Models/stanford-bunny.obj"));

	m_Light = new Light();
	m_Light->SetPosition(1.f, 0.5f, 2.f);
	m_Light->SetSpecularPower(20.f);
	
	return true;
}

void Application::Shutdown()
{
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	if (m_Shader)
	{
		m_Shader->Shutdown();
		delete m_Shader;
		m_Shader = 0;
	}

	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}
}

bool Application::Frame()
{	
	double DeltaTime;
	auto Now = std::chrono::steady_clock::now();
	DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(Now - m_LastUpdate).count() / 1000000.0;
	m_LastUpdate = Now;
	m_AppTime += DeltaTime;
	
	bool Result = Render(DeltaTime);
	if (!Result)
	{
		return false;
	}
	
	return true;
}

bool Application::LoadModel(const char* ModelFilename)
{
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
	}
	
	bool Result;
	char Filename[128];
	
	strcpy_s(Filename, ModelFilename);

	m_Model = new Model();
	FALSE_IF_FAILED(m_Model->Initialise(m_Graphics->GetDevice(), m_Graphics->GetDeviceContext(), Filename));
		
	return true;
}

bool Application::Render(double DeltaTime)
{		
	float RotationAngle = fmod(m_AppTime, 360.f);
	
	DirectX::XMMATRIX WorldMatrix, ViewMatrix, ProjectionMatrix;
	bool Result;

	m_Graphics->BeginScene(0.3f, 0.6f, 0.8f, 1.f);
	
	m_Camera->Render();
	
	RenderPhysicalLight();
	
	if (m_Model)
	{
		m_Graphics->GetWorldMatrix(WorldMatrix);
		WorldMatrix *= DirectX::XMMatrixTranslation(0.02f, -0.1f, 0.f);
		WorldMatrix *= DirectX::XMMatrixScaling(5.f, 5.f, 5.f);
		WorldMatrix *= DirectX::XMMatrixRotationY(RotationAngle);
		WorldMatrix *= DirectX::XMMatrixTranslation(0.f, 0.f, 3.f);
		m_Camera->GetViewMatrix(ViewMatrix);
		m_Graphics->GetProjectionMatrix(ProjectionMatrix);

		m_Model->Render(m_Graphics->GetDeviceContext());

		FALSE_IF_FAILED(
			m_Shader->Render(
				m_Graphics->GetDeviceContext(),
				m_Model->GetIndexCount(),
				WorldMatrix,
				ViewMatrix,
				ProjectionMatrix,
				m_Camera->GetPosition(),
				m_Light->GetPosition(),
				m_Light->GetSpecularPower()
			)
		);
	}
	
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ShowCursor(true);
	static bool ShowDemoWindow = true;
	if (ShowDemoWindow)
	{
		ImGui::ShowDemoWindow(&ShowDemoWindow);
	}

	if (ImGui::Begin("Test Window"))
	{
		ImGui::SliderFloat("Light x", reinterpret_cast<float*>(m_Light->GetPositionPtr()) + 0, -5.f, 5.f);
		ImGui::SliderFloat("Light y", reinterpret_cast<float*>(m_Light->GetPositionPtr()) + 1, -5.f, 5.f);
		ImGui::SliderFloat("Light z", reinterpret_cast<float*>(m_Light->GetPositionPtr()) + 2, -5.f, 5.f);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
	}
	ImGui::End();

	static char ModelLocationBuffer[1024];
	if (ImGui::Begin("Load Models"))
	{
		if (ImGui::Button("Load Stanford Bunny"))
		{
			strcpy_s(ModelLocationBuffer, "Models/stanford-bunny.obj");
			if (LoadModel(ModelLocationBuffer))
			{
				m_ModelLoadSuccessMessage = "Loaded model successfully!";
			}
			else
			{
				m_ModelLoadSuccessMessage = "Failed to load model!";
			}
		}
		
		if (ImGui::Button("Load Suzanne"))
		{
			strcpy_s(ModelLocationBuffer, "Models/suzanne.obj");
			if (LoadModel(ModelLocationBuffer))
			{
				m_ModelLoadSuccessMessage = "Loaded model successfully!";
			}
			else
			{
				m_ModelLoadSuccessMessage = "Failed to load model!";
			}
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::InputText("Model file location", ModelLocationBuffer, sizeof(ModelLocationBuffer));
		if (ImGui::Button("Load model from file"))
		{
			if (LoadModel(ModelLocationBuffer))
			{
				m_ModelLoadSuccessMessage = "Loaded model successfully!";
			}
			else
			{
				m_ModelLoadSuccessMessage = "Failed to load model!";
			}
		}
		ImGui::Text(m_ModelLoadSuccessMessage);
	}
	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_Graphics->EndScene();

	return true;
}

bool Application::RenderPhysicalLight()
{
	DirectX::XMMATRIX WorldMatrix, ViewMatrix, ProjectionMatrix;
	bool Result;

	m_Graphics->GetWorldMatrix(WorldMatrix);
	auto LightPos = m_Light->GetPosition();
	WorldMatrix *= DirectX::XMMatrixScaling(0.2f, 0.2f, 0.2f);
	WorldMatrix *= DirectX::XMMatrixTranslation(LightPos.x, LightPos.y, LightPos.z);
	m_Camera->GetViewMatrix(ViewMatrix);
	m_Graphics->GetProjectionMatrix(ProjectionMatrix);

	m_SceneLight->Render(m_Graphics->GetDeviceContext());

	FALSE_IF_FAILED(
		m_Shader->Render(
			m_Graphics->GetDeviceContext(),
			m_SceneLight->GetIndexCount(),
			WorldMatrix,
			ViewMatrix,
			ProjectionMatrix,
			m_Camera->GetPosition(),
			m_Light->GetPosition(),
			m_Light->GetSpecularPower()
		)
	);
	
	return true;
}
