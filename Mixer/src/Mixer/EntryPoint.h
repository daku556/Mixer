#pragma once

#ifdef MX_PLATFORM_WINDOWS

extern Mixer::Application* Mixer::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Mixer::CreateApplication();
	app->Run();
	delete app;
}

#endif // MX_PLATFORM_WINDOWS
