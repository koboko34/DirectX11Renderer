#include "Application.h"

#include <iostream>

#include "MyMacros.h"

Application::Application()
{
	m_Graphics = 0;
	m_Shader = 0;
	m_Camera = 0;

	m_Models = std::vector<Model*>();
	LastUpdate = std::chrono::steady_clock::now();
	AppTime = 0.0;
}

Application::Application(const Application& Other)
{
}

Application::~Application()
{
}

bool Application::Initialise(int ScreenWidth, int ScreenHeight, HWND hWnd)
{
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

	FALSE_IF_FAILED(AddModel(hWnd, "Models/stanford-bunny.obj"));

	m_Light = new Light();
	m_Light->SetPosition(1.f, 0.5f, 2.f);
	m_Light->SetSpecularPower(20.f);
	
	return true;
}

void Application::Shutdown()
{
	for (Model* m : m_Models)
	{
		if (m)
		{
			m->Shutdown();
			delete m;
		}
	}
	m_Models.clear();
}

bool Application::Frame()
{	
	double DeltaTime;
	auto Now = std::chrono::steady_clock::now();
	DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(Now - LastUpdate).count() / 1000000.0;
	LastUpdate = Now;
	AppTime += DeltaTime;
	
	bool Result = Render(DeltaTime);
	if (!Result)
	{
		return false;
	}
	
	return true;
}

bool Application::AddModel(HWND hWnd, const char* ModelFilename)
{
	bool Result;
	char Filename[128];
	
	strcpy_s(Filename, ModelFilename);

	Model* TheModel = new Model();
	Result = TheModel->Initialise(m_Graphics->GetDevice(), m_Graphics->GetDeviceContext(), Filename);
	if (!Result)
	{
		ShowCursor(true);
		MessageBox(hWnd, L"Failed to initialise Model object!", L"Error", MB_OK);
		return false;
	}
	m_Models.push_back(TheModel);
	
	return true;
}

bool Application::Render(double DeltaTime)
{		
	float RotationAngle = fmod(AppTime, 360.f);
	
	DirectX::XMMATRIX WorldMatrix, ViewMatrix, ProjectionMatrix;
	bool Result;

	m_Graphics->BeginScene(0.3f, 0.6f, 0.8f, 1.f);
	m_Camera->Render();
	
	RenderPhysicalLight();

	if (m_Models.size() >= 0)
	{
		m_Graphics->GetWorldMatrix(WorldMatrix);
		WorldMatrix *= DirectX::XMMatrixTranslation(0.02f, -0.1f, 0.f);
		WorldMatrix *= DirectX::XMMatrixScaling(5.f, 5.f, 5.f);
		WorldMatrix *= DirectX::XMMatrixRotationY(RotationAngle);
		WorldMatrix *= DirectX::XMMatrixTranslation(0.f, 0.f, 3.f);
		m_Camera->GetViewMatrix(ViewMatrix);
		m_Graphics->GetProjectionMatrix(ProjectionMatrix);

		m_Models[0]->Render(m_Graphics->GetDeviceContext());

		FALSE_IF_FAILED(
			m_Shader->Render(
				m_Graphics->GetDeviceContext(),
				m_Models[0]->GetIndexCount(),
				WorldMatrix,
				ViewMatrix,
				ProjectionMatrix,
				m_Camera->GetPosition(),
				m_Light->GetPosition(),
				m_Light->GetSpecularPower()
			)
		);
	}

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
