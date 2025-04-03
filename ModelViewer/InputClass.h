#pragma once

#ifndef INPUTCLASS_H
#define INPUTCLASS_H

class InputClass
{
private:
	InputClass() {}

	static InputClass* ms_Instance;

public:
	static InputClass* GetSingletonPtr();

	void Initialise();

	void KeyDown(unsigned int Input);
	void KeyUp(unsigned int Input);

	bool IsKeyDown(unsigned int Key);

private:
	bool m_Keys[256];
};

#endif
