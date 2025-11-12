#pragma once

#ifdef MX_PLATFORM_WINDOWS

extern Mixer::Application* Mixer::CreateApplication();

int main(int argc, char** argv)
{
	Mixer::Log::Init();
	MX_CORE_WARN("init log");
	int a = 5;
	MX_INFO("hello var = {0}", a);

	auto app = Mixer::CreateApplication();
	app->Run();
	delete app;
}

#endif // MX_PLATFORM_WINDOWS
