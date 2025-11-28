#include <Mixer.h>

//class ExampleLayer : public Mixer::Layer
//{
//public:
//	ExampleLayer()
//		: Layer("Example")
//	{
//	}
//
//	void OnUpdate() override
//	{
//		MX_INFO("ExampleLayer::Update");
//	}
//
//	void OnEvent(Mixer::Event& event) override
//	{
//		MX_TRACE("{0}", event.ToString());
//	}
//
//};

class Sandbox : public Mixer::Application
{
public: 
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
	}
	~Sandbox()
	{

	}
	

};

Mixer::Application* Mixer::CreateApplication()
{
	return new Sandbox();
}