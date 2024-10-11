#pragma once

#ifndef SYSTEMCLASS_H
#define SYSTEMCLASS_H

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include "ImGuiManager.h"

#include "InputClass.h"
#include "Application.h"

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass& Other);
	~SystemClass();

	bool Initialise();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

private:
	bool Frame();
	void InitialiseWindows(int& ScreenWidth, int& ScreenHeight);
	void ShutdownWindows();

private:
	LPCWSTR m_ApplicationName;
	HINSTANCE m_hInstance;
	HWND m_hwnd;

	InputClass* m_Input;
	Application* m_Application;

	ImGuiManager ImGui;
};

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

static SystemClass* ApplicationHandle = nullptr;

#endif
