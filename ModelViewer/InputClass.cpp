#include "InputClass.h"

InputClass* InputClass::ms_Instance = nullptr;

InputClass* InputClass::GetSingletonPtr()
{
	if (!InputClass::ms_Instance)
	{
		InputClass::ms_Instance = new InputClass();
		InputClass::ms_Instance->Initialise();
	}
	return InputClass::ms_Instance;
}

void InputClass::Initialise()
{
	for (int i = 0; i < 256; i++)
	{
		m_Keys[i] = false;
	}
}

void InputClass::KeyDown(unsigned int Input)
{
	m_Keys[Input] = true;
}

void InputClass::KeyUp(unsigned int Input)
{
	m_Keys[Input] = false;
}

bool InputClass::IsKeyDown(unsigned int Key)
{
	return m_Keys[Key];
}

void InputClass::SetMouseWheelDelta(short Delta)
{
	m_MouseWheelDelta = Delta;
}
