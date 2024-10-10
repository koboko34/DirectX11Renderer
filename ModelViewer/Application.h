#pragma once

#ifndef APPLICATION_H
#define APPLICATION_H

#include <chrono>
#include <vector>

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
	bool AddModel(HWND hWnd, const char* ModelFilename);
	bool Render(double DeltaTime);
	bool RenderPhysicalLight();

private:
	Graphics* m_Graphics;
	Shader* m_Shader;
	Camera* m_Camera;
	Light* m_Light;

	Model* m_SceneLight;
	std::vector<Model*> m_Models;

	std::chrono::steady_clock::time_point LastUpdate;
	double AppTime;
};

#endif
