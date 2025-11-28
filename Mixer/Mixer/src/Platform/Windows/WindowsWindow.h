#pragma once

#include "Mixer/Window.h"

#include <GLFW/glfw3.h>

namespace Mixer {
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		// Inherited via Window
		void OnUpdate() override;

		unsigned int GetWidth() const override { return m_Data.Width; };

		unsigned int GetHeigth() const override { return m_Data.Height; };

		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; };

		void SetVSync(bool enabled) override;

		bool IsVSync() const override;
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
		GLFWwindow* m_Window;
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}