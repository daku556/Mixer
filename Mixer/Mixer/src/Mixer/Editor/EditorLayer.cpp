#include "EditorLayer.h"
#include <glad/glad.h> // OpenGL 함수 사용
#include <vector>
#include <iostream>

namespace Mixer {
    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    // 셰이더 에러 확인용 헬퍼 함수
    void CheckShaderError(GLuint shader, const char* type) {
        GLint success;
        char infoLog[1024];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            MX_CORE_ERROR("SHADER_COMPILATION_ERROR [{0}]: {1}", type, infoLog);
            __debugbreak();
        }
    }

    void CheckProgramError(GLuint program) {
        GLint success;
        char infoLog[1024];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 1024, NULL, infoLog);
            MX_CORE_ERROR("PROGRAM_LINKING_ERROR: {0}", infoLog);
            __debugbreak();
        }
    }

    void EditorLayer::OnAttach()
    {
        // 1. 정점 데이터 (삼각형 모양 배치)
        float vertices[] = {
            -0.5f, -0.5f, 0.0f, // 좌측 하단
             0.5f, -0.5f, 0.0f, // 우측 하단
             0.0f,  0.5f, 0.0f  // 상단 중앙
        };

        // 2. VAO, VBO 생성
        glGenVertexArrays(1, &m_VertexArray);
        glBindVertexArray(m_VertexArray);

        glGenBuffers(1, &m_VertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // 3. 속성 설정
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // 4. 셰이더 소스 (수정됨: gl_PointSize 추가)
        const char* vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Pos;

        // [추가] 카메라 행렬 (View * Projection)
        uniform mat4 u_ViewProjection; 

        void main() {
            // [수정] 3D 좌표 -> 카메라 변환 -> 화면 좌표
            gl_Position = u_ViewProjection * vec4(a_Pos, 1.0);
            
            gl_PointSize = 20.0; 
        }
    )";

        const char* fragmentSrc = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            // 빨간색으로 변경 (흰색과 확실히 구분하기 위해)
            FragColor = vec4(1.0, 0.0, 0.0, 1.0); 
        }
    )";

        // 5. 셰이더 컴파일 및 에러 체크
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
        glCompileShader(vertexShader);
        CheckShaderError(vertexShader, "VERTEX"); // 에러 확인

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
        glCompileShader(fragmentShader);
        CheckShaderError(fragmentShader, "FRAGMENT"); // 에러 확인

        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vertexShader);
        glAttachShader(m_ShaderProgram, fragmentShader);
        glLinkProgram(m_ShaderProgram);
        CheckProgramError(m_ShaderProgram); // 링크 에러 확인

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void EditorLayer::OnUpdate()
    {
        // 배경 지우기
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 프로그램 사용
        glUseProgram(m_ShaderProgram);

        // [중요] 프로그램 포인트 사이즈 활성화 (일부 드라이버 필수)
        glEnable(GL_PROGRAM_POINT_SIZE);

        glBindVertexArray(m_VertexArray);
        glDrawArrays(GL_POINTS, 0, 3);
    }

    void EditorLayer::OnDetach()
    {
        glDeleteVertexArrays(1, &m_VertexArray);
        glDeleteBuffers(1, &m_VertexBuffer);
        glDeleteProgram(m_ShaderProgram);
    }

    void EditorLayer::OnEvent(Event& event) {}
}
