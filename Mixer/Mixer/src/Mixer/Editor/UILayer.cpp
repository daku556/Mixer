#include "UILayer.h"
#include "Mixer/Events/KeyEvent.h" // 키 이벤트
#include <iostream>

namespace Mixer {

    UILayer::UILayer() : Layer("UILayer") {}

    void UILayer::OnAttach()
    {
        // 1. 텍스처 로드 (assets/textures/help.png 파일을 만드셔야 합니다!)
        // 배경은 투명하고 글씨가 쓰여있는 PNG를 권장합니다.
        m_HelpTexture = std::make_unique<Texture>("assets/textures/MixerUI.png");

        // 2. 화면 전체를 덮는 사각형 (Quad) 데이터
        // 위치(x,y) + 텍스처좌표(u,v)
        float vertices[] = {
            // 위치        // UV
           -1.0f, -1.0f,   0.0f, 0.0f, // 좌하
            1.0f, -1.0f,   1.0f, 0.0f, // 우하
            1.0f,  1.0f,   1.0f, 1.0f, // 우상
           -1.0f,  1.0f,   0.0f, 1.0f  // 좌상
        };

        uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

        // 3. 버퍼 생성
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // Pos
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1); // UV
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glGenBuffers(1, &m_IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // 4. 텍스처용 셰이더 (매우 간단)
        const char* vsSrc = R"(#version 330 core
layout(location = 0) in vec2 a_Pos;
layout(location = 1) in vec2 a_TexCoord;
out vec2 v_TexCoord;
void main() {
    gl_Position = vec4(a_Pos, 0.0, 1.0); // Z=0
    v_TexCoord = a_TexCoord;
})";
        const char* fsSrc = R"(#version 330 core
layout(location = 0) out vec4 color;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
void main() {
    color = texture(u_Texture, v_TexCoord);
})";

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vsSrc, nullptr);
        glCompileShader(vs);
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fsSrc, nullptr);
        glCompileShader(fs);

        m_Shader = glCreateProgram();
        glAttachShader(m_Shader, vs);
        glAttachShader(m_Shader, fs);
        glLinkProgram(m_Shader);

        glDeleteShader(vs);
        glDeleteShader(fs);

        // 셰이더에 텍스처 슬롯 0번을 쓰겠다고 알림
        glUseProgram(m_Shader);
        glUniform1i(glGetUniformLocation(m_Shader, "u_Texture"), 0);
    }

    void UILayer::OnDetach()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_IBO);
        glDeleteProgram(m_Shader);
    }

    void UILayer::OnUpdate()
    {
        if (!m_IsVisible) return;

        // [핵심] 투명도(Alpha Blending) 활성화
        // 이게 없으면 투명 배경 이미지가 검게 나옵니다.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 2D UI이므로 깊이 테스트는 끕니다 (맨 위에 그리기 위해)
        glDisable(GL_DEPTH_TEST);

        glUseProgram(m_Shader);
        m_HelpTexture->Bind(0); // 0번 슬롯에 텍스처 바인딩

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        // 복구 (다음 3D 렌더링을 위해)
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    void UILayer::OnEvent(Event& e)
    {
        // F1 키를 누르면 도움말 토글
        if (e.GetEventType() == EventType::KeyPressed)
        {
            KeyPressedEvent& event = (KeyPressedEvent&)e;
            if (event.GetKeyCode() == 290) // F1 Key
            {
                m_IsVisible = !m_IsVisible;
            }
        }
    }
}