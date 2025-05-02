#pragma once

#ifndef LOGGER_H
#define LOGGER_H

#include "d3d11.h"

class Logger
{
public:
	static void OutputShaderErrorMessage(ID3D10Blob* ErrorMessage, HWND hWnd, const WCHAR* ShaderFilename);

};

#endif
