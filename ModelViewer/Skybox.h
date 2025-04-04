#pragma once

#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <string>

#include "d3d11.h"

class Skybox
{
public:
	Skybox() {};

	bool Init();
	void Render();
	void Shutdown();

private:
	std::string m_TexturesDir = "Textures/skybox/";
	std::vector<std::string> m_FileNames { "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg" };
};

#endif
