#pragma once

#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

#include "ImGui/imgui.h"

class ImGuiManager
{
public:
	ImGuiManager();
	~ImGuiManager();

	static void RenderPostProcessWindow();
	static void RenderWorldHierarchyWindow();
	static void RenderCamerasWindow();
};

#endif
