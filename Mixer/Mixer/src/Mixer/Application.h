#pragma once

#include "Core.h"
#include "Mixer/Events/Event.h"
#include "Window.h"
#include "Mixer/LayerStack.h"

namespace Mixer {
	class MIXER_API Application
	{
	public:
		Application();
		virtual ~Application();


		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;
	};


	// to be defind in client
	Application* CreateApplication();
}
