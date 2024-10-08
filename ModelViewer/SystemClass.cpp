#include "SystemClass.h"

SystemClass::SystemClass()
{
	m_Input = nullptr;
	m_Application = nullptr;
}

SystemClass::SystemClass(const SystemClass& Other)
{
}

SystemClass::~SystemClass()
{
}

bool SystemClass::Initialise()
{
	int ScreenWidth, ScreenHeight;
	bool Result;

	ScreenWidth = 0;
	ScreenHeight = 0;

	InitialiseWindows(ScreenWidth, ScreenHeight);

	m_Input = new InputClass();
	m_Input->Initialise();

	m_Application = new Application();

	Result = m_Application->Initialise(ScreenWidth, ScreenHeight, m_hwnd);
	if (!Result)
	{
		return false;
	}

	return true;
}

void SystemClass::Shutdown()
{
	if (m_Application)
	{
		m_Application->Shutdown();
		delete m_Application;
		m_Application = nullptr;
	}

	if (m_Input)
	{
		delete m_Input;
		m_Input = nullptr;
	}

	ShutdownWindows();
}

void SystemClass::Run()
{
	MSG msg;
	bool ShouldClose, Result;

	ZeroMemory(&msg, sizeof(MSG));

	ShouldClose = false;
	while (!ShouldClose)
	{
		// handle the windows messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// if windows signals to end the application then exit out
		if (msg.message == WM_QUIT)
		{
			ShouldClose = true;
		}
		else
		{
			Result = Frame();
			if (!Result)
			{
				ShouldClose = true;
			}
		}
	}
}

bool SystemClass::Frame()
{
	bool Result;

	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	Result = m_Application->Frame();
	if (!Result)
	{
		return false;
	}

	return true;
}

LRESULT SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_KEYDOWN:
	{
		m_Input->KeyDown((unsigned int)wparam);
		return 0;
	}
	case WM_KEYUP:
	{
		m_Input->KeyUp((unsigned int)wparam);
		return 0;
	}
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

void SystemClass::InitialiseWindows(int& ScreenWidth, int& ScreenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	ApplicationHandle = this;

	m_hInstance = GetModuleHandle(NULL);

	m_ApplicationName = L"Engine";

	// setup the windows class with default settings
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_ApplicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// register the window class
	RegisterClassEx(&wc);

	ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	// setup the screen settings depending on whether it is running in full screen or in windowed mode
	if (FULL_SCREEN)
	{
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)ScreenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)ScreenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = posY = 0;
	}
	else
	{
		ScreenWidth = 800;
		ScreenHeight = 600;

		posX = (GetSystemMetrics(SM_CXSCREEN) - ScreenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - ScreenHeight) / 2;
	}

	m_hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		m_ApplicationName,
		m_ApplicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY,
		ScreenWidth, ScreenHeight,
		NULL,
		NULL,
		m_hInstance,
		NULL
	);

	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	ShowCursor(false);
}

void SystemClass::ShutdownWindows()
{
	ShowCursor(true);

	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	UnregisterClass(m_ApplicationName, m_hInstance);
	m_hInstance = NULL;

	ApplicationHandle = nullptr;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
	{
		return ApplicationHandle->MessageHandler(hWnd, uMessage, wParam, lParam);
	}
	}
}
