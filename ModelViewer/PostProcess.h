#pragma once

#include <iostream>

class PostProcess
{
public:
	virtual void ApplyPostProcess() = 0;

	virtual ~PostProcess() {}
};

class PostProcessFog : public PostProcess
{
public:
	PostProcessFog() {} // pass whatever we need into here
	
	void ApplyPostProcess() override
	{
		std::cout << "Applying fog..." << std::endl;
	}
};

class PostProcessBlur : public PostProcess
{
public:
	PostProcessBlur(int BlurStrength) : m_BlurStrength(BlurStrength) {}

	void ApplyPostProcess() override
	{
		std::cout << "Applying blur..." << std::endl;
	}

private:
	int m_BlurStrength = 5;
};