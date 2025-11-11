#include <Mixer.h>

class Sandbox : public Mixer::Application
{
public: 
	Sandbox()
	{

	}
	~Sandbox()
	{

	}
	

};

Mixer::Application* Mixer::CreateApplication()
{
	return new Sandbox();
}