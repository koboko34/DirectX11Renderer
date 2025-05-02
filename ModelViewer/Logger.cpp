#include <cstring>
#include <fstream>

#include "Logger.h"

void Logger::OutputShaderErrorMessage(ID3D10Blob* ErrorMessage, HWND hWnd, const WCHAR* ShaderFilename)
{
	char* CompileErrors;
	unsigned long long BufferSize, i;
	std::ofstream fout;

	CompileErrors = (char*)(ErrorMessage->GetBufferPointer());
	BufferSize = ErrorMessage->GetBufferSize();
	fout.open("shader-error.txt");

	const char* str1 = "Error compiling shader!\n\n";

	size_t len1 = std::strlen(str1) + 1;
	size_t len2 = std::strlen(CompileErrors) + 1;

	wchar_t* wstr1 = new wchar_t[len1];
	wchar_t* wstr2 = new wchar_t[len2];

	size_t Converted1, Converted2;
	mbstowcs_s(&Converted1, wstr1, len1, str1, len1 - 1);
	mbstowcs_s(&Converted2, wstr2, len2, CompileErrors, len2 - 1);

	size_t CombinedLen = len1 + len2 - 1;
	wchar_t* CombinedWstr = new wchar_t[CombinedLen];

	wcscpy_s(CombinedWstr, CombinedLen, wstr1);
	wcscat_s(CombinedWstr, CombinedLen, wstr2);

	for (i = 0; i < BufferSize; i++)
	{
		fout << CompileErrors[i];
	}

	fout.close();

	ErrorMessage->Release();
	ErrorMessage = 0;

	MessageBox(hWnd, CombinedWstr, ShaderFilename, MB_OK);

	delete[] wstr1;
	delete[] wstr2;
	delete[] CombinedWstr;
}
