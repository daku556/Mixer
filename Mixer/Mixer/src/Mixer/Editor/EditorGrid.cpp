#include "EditorGrid.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace Mixer {

    void EditorGrid::Init()
    {
        // 1. 그리드 데이터 생성
        std::vector<float> vertices;
        float gridSize = 10.0f;
        float step = 1.0f;

        // 격자 (Grid Lines)
        for (float i = -gridSize; i <= gridSize; i += step)
        {
            vertices.push_back(i); vertices.push_back(0.0f); vertices.push_back(-gridSize);
            vertices.push_back(i); vertices.push_back(0.0f); vertices.push_back(gridSize);
            vertices.push_back(-gridSize); vertices.push_back(0.0f); vertices.push_back(i);
            vertices.push_back(gridSize);  vertices.push_back(0.0f); vertices.push_back(i);
        }

        // 축 (Axes)
        // X (Red)
        vertices.push_back(-gridSize); vertices.push_back(0.0f); vertices.push_back(0.0f);
        vertices.push_back(gridSize);  vertices.push_back(0.0f); vertices.push_back(0.0f);
        // Z (Blue)
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(-gridSize);
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(gridSize);
        // Y (Green)
        vertices.push_back(0.0f); vertices.push_back(-gridSize); vertices.push_back(0.0f);
        vertices.push_back(0.0f); vertices.push_back(gridSize);  vertices.push_back(0.0f);

        m_VertexCount = vertices.size() / 3;

        // 2. OpenGL 버퍼
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // 3. 그리드 전용 셰이더 (간단한 버전)
        const char* vsSrc = R"(#version 330 core
layout(location = 0) in vec3 a_Pos;
uniform mat4 u_ViewProjection;
void main() {
    gl_Position = u_ViewProjection * vec4(a_Pos, 1.0);
})";
        const char* fsSrc = R"(#version 330 core
layout(location = 0) out vec4 FragColor;
uniform vec4 u_Color;
void main() {
    FragColor = u_Color;
})";

        // 셰이더 컴파일 (간략화)
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
    }

    void EditorGrid::Render(const glm::mat4& viewProjection)
    {
        glUseProgram(m_Shader);
        glUniformMatrix4fv(glGetUniformLocation(m_Shader, "u_ViewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
        glBindVertexArray(m_VAO);

        // 1. 격자 (회색)
        glLineWidth(1.0f);
        glUniform4f(glGetUniformLocation(m_Shader, "u_Color"), 0.4f, 0.4f, 0.4f, 1.0f);
        int gridLinesCount = m_VertexCount - 6;
        glDrawArrays(GL_LINES, 0, gridLinesCount);

        // [핵심] 깊이 테스트를 잠시 꺼서, 격자 위에 덮어쓰도록 강제함
        glDisable(GL_DEPTH_TEST);

        // 축은 좀 더 두껍게 그림
        glLineWidth(2.0f);

        // X축 (Red)
        glUniform4f(glGetUniformLocation(m_Shader, "u_Color"), 1.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINES, gridLinesCount, 2);

        // Z축 (Blue)
        glUniform4f(glGetUniformLocation(m_Shader, "u_Color"), 0.0f, 0.0f, 1.0f, 1.0f);
        glDrawArrays(GL_LINES, gridLinesCount + 2, 2);

        // Y축 (Green)
        glUniform4f(glGetUniformLocation(m_Shader, "u_Color"), 0.0f, 1.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINES, gridLinesCount + 4, 2);

        // [복구] 다시 켜줘야 다음 물체(모델)들이 정상적으로 그려짐
        glEnable(GL_DEPTH_TEST);
        glLineWidth(1.0f); // 두께 원상복구
    }

    void EditorGrid::Shutdown()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteProgram(m_Shader);
    }
}