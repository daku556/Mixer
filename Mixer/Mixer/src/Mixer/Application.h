#pragma once
#include "Core.h"

namespace Mixer {
	class MIXER_API Application
	{
	public:
		Application();
		virtual ~Application();


		void Run();
	};


	// to be defind in client
	Application* CreateApplication();
}
