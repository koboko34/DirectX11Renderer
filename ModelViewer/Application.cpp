#include "Application.h"

Application::Application()
{
	m_Graphics = 0;
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
		MessageBox(hWnd, L"Could not initialise Graphics object!", L"Error", MB_OK);
		return false;
	}
	
	return true;
}

void Application::Shutdown()
{
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}
}

bool Application::Frame()
{
	bool Result = Render();
	if (!Result)
	{
		return false;
	}
	
	return true;
}

bool Application::Render()
{
	DirectX::XMMATRIX WorldMatrix, ViewMatrix, ProjectionMatrix;
	bool Result;

	m_Graphics->BeginScene(0.3f, 0.6f, 0.8f, 1.f);
	m_Graphics->GetWorldMatrix(WorldMatrix);

	// load model

	// render using shader
	
	m_Graphics->EndScene();

	return true;
}
