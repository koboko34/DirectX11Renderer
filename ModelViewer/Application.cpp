#include "Application.h"

#include <iostream>

#include "MyMacros.h"

Application::Application()
{
	m_Graphics = 0;
	m_Shader = 0;
	m_Model = 0;
	m_Camera = 0;

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
	char ModelFilename[128];
	
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

	strcpy_s(ModelFilename, "Models/sphere.txt");

	m_Model = new Model();
	Result = m_Model->Initialise(m_Graphics->GetDevice(), m_Graphics->GetDeviceContext(), ModelFilename);
	if (!Result)
	{
		ShowCursor(true);
		MessageBox(hWnd, L"Failed to initialise Model object!", L"Error", MB_OK);
		return false;
	}
	
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

	if (m_Shader)
	{
		m_Shader->Shutdown();
		delete m_Shader;
		m_Shader = 0;
	}
	
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}
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

bool Application::Render(double DeltaTime)
{		
	float RotationAngle = fmod(AppTime, 360.f);
	
	DirectX::XMMATRIX WorldMatrix, ViewMatrix, ProjectionMatrix;
	bool Result;

	m_Graphics->BeginScene(0.3f, 0.6f, 0.8f, 1.f);

	m_Camera->Render();

	m_Graphics->GetWorldMatrix(WorldMatrix);
	WorldMatrix *= DirectX::XMMatrixRotationY(RotationAngle);
	WorldMatrix *= DirectX::XMMatrixTranslation(0.f, 0.f, 5.f);
	m_Camera->GetViewMatrix(ViewMatrix);
	m_Graphics->GetProjectionMatrix(ProjectionMatrix);

	m_Model->Render(m_Graphics->GetDeviceContext());

	FALSE_IF_FAILED(
		m_Shader->Render(
			m_Graphics->GetDeviceContext(),
			m_Model->GetIndexCount(),
			WorldMatrix,
			ViewMatrix,
			ProjectionMatrix
		)
	);

	m_Graphics->EndScene();

	return true;
}
