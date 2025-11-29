#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

namespace Mixer {

    class EditorGrid
    {
    public:
        EditorGrid() = default;
        ~EditorGrid() = default;

        void Init();  // VAO, VBO, Shader 생성
        void Render(const glm::mat4& viewProjection); // 그리기
        void Shutdown(); // 자원 해제

    private:
        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
        GLuint m_Shader = 0; // 그리드 전용 셰이더
        int m_VertexCount = 0;
    };
}
