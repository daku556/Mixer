#pragma once
#include "Layer.h"
#include "Texture.h"
#include <glad/glad.h>
#include <memory>

namespace Mixer {
    class UILayer : public Layer
    {
    public:
        UILayer();
        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnEvent(Event& e) override; // F1키로 끄고 켜기 위해 필요

    private:
        std::unique_ptr<Texture> m_HelpTexture;
        GLuint m_VAO, m_VBO, m_IBO, m_Shader;
        bool m_IsVisible = true; // 토글 기능
    };
}