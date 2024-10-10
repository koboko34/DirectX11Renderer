#pragma once

#ifndef APPLICATION_H
#define APPLICATION_H

#include <chrono>

#include "Graphics.h"
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "Light.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.f;
const float SCREEN_NEAR = 0.3f;

class Application
{
public:
	Application();
	Application(const Application& Other);
	~Application();

	bool Initialise(int ScreenWidth, int ScreenHeight, HWND hWnd);
	void Shutdown();
	bool Frame();

private:
	bool Render(double DeltaTime);

private:
	Graphics* m_Graphics;
	Shader* m_Shader;
	Model* m_Model;
	Camera* m_Camera;
	Light* m_Light;

	std::chrono::steady_clock::time_point LastUpdate;
	double AppTime;
};

#endif
