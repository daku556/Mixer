#pragma once

#include "Mixer/Layer.h"

#include <glad/glad.h>

namespace Mixer {
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnEvent(Event& e) override;

	private:
		GLuint m_ShaderProgram;
		GLuint m_VertexArray;
		GLuint m_VertexBuffer;
	};
}