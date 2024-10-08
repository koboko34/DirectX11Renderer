#include "SystemClass.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	SystemClass* System;
	bool Result;

	System = new SystemClass();

	Result = System->Initialise();
	if (Result)
	{
		System->Run();
	}

	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}