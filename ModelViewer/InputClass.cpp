#include "InputClass.h"

InputClass::InputClass()
{
}

InputClass::InputClass(const InputClass& Other)
{
}

InputClass::~InputClass()
{
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
