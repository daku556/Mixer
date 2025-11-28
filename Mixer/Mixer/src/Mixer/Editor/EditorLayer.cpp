#include "EditorLayer.h"
#include <glad/glad.h> // OpenGL 함수 사용
#include <vector>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mixer/Events/MouseEvent.h"
#include "Mixer/Events/ApplicationEvent.h"
#include "Mixer/Events/KeyEvent.h"
#include "Mixer/Application.h"

namespace Mixer {
    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    // --- [1] 초기화 (데이터 생성 및 버퍼 할당) ---
    void EditorLayer::OnAttach()
    {
        // 1. 데이터 초기화 (멤버 변수에 저장)
        m_Vertices = {
            { -0.5f, -0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.0f,  0.5f, 0.0f }
        };

        // 2. OpenGL 버퍼 생성
        glGenVertexArrays(1, &m_VertexArray);
        glBindVertexArray(m_VertexArray);

        glGenBuffers(1, &m_VertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

        // [중요] GL_DYNAMIC_DRAW로 변경! (데이터가 자주 바뀜을 알림)
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3), m_Vertices.data(), GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        // 3. 셰이더 생성 (깔끔해진 코드)
        const char* vertexSrc = R"(#version 330 core
layout(location = 0) in vec3 a_Pos;
uniform mat4 u_ViewProjection;
void main() {
    gl_Position = u_ViewProjection * vec4(a_Pos, 1.0);
    gl_PointSize = 20.0;
}
)";
        const char* fragmentSrc = R"(#version 330 core
layout(location = 0) out vec4 FragColor;
uniform vec4 u_Color;
void main() {
    FragColor = u_Color;
}
)";
        // (셰이더 컴파일 및 에러 체크 코드는 기존과 동일하므로 생략하거나 함수 호출)
        // ... (CheckShaderError 함수 활용해서 컴파일 하세요) ...

        // [임시 컴파일 코드 - 함수 내부 내용을 여기에 넣으세요]
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertexSrc, nullptr);
        glCompileShader(vs);
        CheckShaderError(vs, "VERTEX");

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragmentSrc, nullptr);
        glCompileShader(fs);
        CheckShaderError(fs, "FRAGMENT");

        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vs);
        glAttachShader(m_ShaderProgram, fs);
        glLinkProgram(m_ShaderProgram);
        CheckProgramError(m_ShaderProgram);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    // --- [2] 메인 루프 (Raycasting -> Update -> Render) ---
    void EditorLayer::OnUpdate()
    {
        auto& window = Application::Get().GetWindow();
        float width = (float)window.GetWidth();
        float height = (float)window.GetHeight();
        if (width == 0 || height == 0) return;

        // 1. 화면 지우기
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. 카메라 행렬 업데이트
        float aspectRatio = width / height;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_ViewProjection = projection * view;

        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);

        // 3. Raycasting (드래그 중이 아닐 때만 새로운 Hover 대상을 찾음)

        // Ray 생성
        glm::vec3 rayDir = GetRayFromMouse();
        glm::vec3 rayOrigin = cameraPos; // 카메라는 (0,0,5)에 고정
        if (m_IsTranslationMode && m_SelectedIndex != -1)
        {
            // [이동 모드] : 마우스 움직임에 따라 정점 이동 (상대 좌표)

            // 1. 현재 마우스가 가리키는 3D 평면 위 좌표 계산
            //    (평면은 원래 정점 위치 기준 Z=0 평면)
            glm::vec3 planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 planePoint = m_VertexStartPos;

            float t = 0.0f;
            if (CalculatePlaneIntersection(rayOrigin, rayDir, planePoint, planeNormal, t))
            {
                glm::vec3 currentMousePoint = rayOrigin + (rayDir * t);

                // 2. 델타(변위) 계산: (현재 마우스 3D 위치) - (G키 눌렀을 때 마우스 3D 위치)
                glm::vec3 delta = currentMousePoint - m_MouseStartDragPoint;

                // 3. 정점 위치 업데이트: (원래 정점 위치) + 델타
                //    이렇게 해야 마우스 포인터가 정점에 딱 붙지 않고, 거리를 유지하며 따라다님
                m_Vertices[m_SelectedIndex] = m_VertexStartPos + delta;
            }

            // 이동 중에는 Hover 계산 안 함 (선택된 점만 신경 씀)
        }
        else
        {
            // [일반 모드] : Hover 감지
            m_HoveredIndex = -1;
            float minDistance = 1000.0f;

            for (int i = 0; i < m_Vertices.size(); i++)
            {
                float dist = glm::length(glm::cross(m_Vertices[i] - rayOrigin, rayDir));

                if (dist < 0.3f && dist < minDistance)
                {
                    minDistance = dist;
                    m_HoveredIndex = i;
                }
            }
        }
    // -------------------------------------------------------------------------
    // [렌더링] (기존과 거의 동일)
    // -------------------------------------------------------------------------
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_Vertices.size() * sizeof(glm::vec3), m_Vertices.data());

        glUseProgram(m_ShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_ViewProjection"), 1, GL_FALSE, glm::value_ptr(m_ViewProjection));

        glEnable(GL_PROGRAM_POINT_SIZE);
        glBindVertexArray(m_VertexArray);

        for (int i = 0; i < m_Vertices.size(); i++)
        {
            // 색상 우선순위
            if (i == m_SelectedIndex)
            {
                if (m_IsTranslationMode)
                    // 이동 중일 때는 밝은 흰색/주황색 등으로 표시해 강조 (Blender 느낌)
                    glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f);
                else
                    // 선택됨: 파란색
                    glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 0.0f, 0.5f, 1.0f, 1.0f);
            }
            else if (i == m_HoveredIndex)
            {
                // Hover: 녹색
                glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 0.0f, 1.0f, 0.0f, 1.0f);
            }
            else
            {
                // 기본: 빨간색
                glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 1.0f, 0.0f, 0.0f, 1.0f);
            }

            glDrawArrays(GL_POINTS, i, 1);
        }
    }

    // --- [3] Ray 계산 헬퍼 함수 ---
    glm::vec3 EditorLayer::GetRayFromMouse()
    {
        auto& window = Application::Get().GetWindow();
        float width = (float)window.GetWidth();
        float height = (float)window.GetHeight();

        float mouseX = (2.0f * m_MousePos.x) / width - 1.0f;
        float mouseY = 1.0f - (2.0f * m_MousePos.y) / height;

        glm::vec4 rayStart = glm::vec4(mouseX, mouseY, -1.0f, 1.0f);
        glm::vec4 rayEnd = glm::vec4(mouseX, mouseY, 1.0f, 1.0f);

        glm::mat4 inverseVP = glm::inverse(m_ViewProjection);

        glm::vec4 rayStartWorld = inverseVP * rayStart;
        rayStartWorld /= rayStartWorld.w;
        glm::vec4 rayEndWorld = inverseVP * rayEnd;
        rayEndWorld /= rayEndWorld.w;

        return glm::normalize(glm::vec3(rayEndWorld - rayStartWorld));
    }

    // --- [4] 이벤트 처리 ---
    void EditorLayer::OnEvent(Event& e)
    {
        // 1. 마우스 이동 (좌표 갱신)
        if (e.GetEventType() == EventType::MouseMoved)
        {
            MouseMovedEvent& event = (MouseMovedEvent&)e;
            m_MousePos = { event.GetX(), event.GetY() };
        }
        // 2. 윈도우 리사이즈
        else if (e.GetEventType() == EventType::WindowResize)
        {
            WindowResizeEvent& event = (WindowResizeEvent&)e;
            glViewport(0, 0, event.GetWidth(), event.GetHeight());
        }
        // 3. 키보드 입력 ('G'키 - 이동 모드 진입)
        else if (e.GetEventType() == EventType::KeyPressed)
        {
            KeyPressedEvent& event = (KeyPressedEvent&)e;

            // 'G'키를 눌렀고, 현재 선택된 정점이 있다면 -> 이동 모드 시작!
            // (키 코드는 사용하시는 엔진의 KeyCode에 맞게 수정하세요. 예: 71 or G)
            if (event.GetKeyCode() == 71 /* G Key */ && m_SelectedIndex != -1 && !m_IsTranslationMode)
            {
                m_IsTranslationMode = true;

                // (1) 취소(우클릭)를 대비해 원래 위치 저장
                m_VertexStartPos = m_Vertices[m_SelectedIndex];

                // (2) "상대적 이동"을 위해 현재 마우스가 가리키는 3D 공간 좌표 계산
                //     이때 평면은 정점의 현재 위치를 기준으로 설정
                glm::vec3 rayDir = GetRayFromMouse();
                glm::vec3 rayOrigin = glm::vec3(0.0f, 0.0f, 5.0f); // 카메라 위치

                glm::vec3 planeNormal = glm::vec3(0.0f, 0.0f, 1.0f); // Z축 수직 평면
                glm::vec3 planePoint = m_VertexStartPos;

                float t = 0.0f;
                if (CalculatePlaneIntersection(rayOrigin, rayDir, planePoint, planeNormal, t))
                {
                    m_MouseStartDragPoint = rayOrigin + (rayDir * t);
                }
            }
        }
        // 4. 마우스 버튼 (선택 / 확정 / 취소)
        else if (e.GetEventType() == EventType::MouseButtonPressed)
        {
            MouseButtonPressedEvent& event = (MouseButtonPressedEvent&)e;

            // [상황 A] 이동 모드 중일 때 ('G'키 누른 후)
            if (m_IsTranslationMode)
            {
                if (event.GetMouseButton() == 0) // 좌클릭: 확정 (Confirm)
                {
                    m_IsTranslationMode = false; // 위치 유지한 채 모드 종료
                }
                else if (event.GetMouseButton() == 1) // 우클릭: 취소 (Cancel)
                {
                    m_Vertices[m_SelectedIndex] = m_VertexStartPos; // 원래 위치로 복구
                    m_IsTranslationMode = false; // 모드 종료
                }
            }
            // [상황 B] 일반 모드일 때 (단순 선택)
            else
            {
                if (event.GetMouseButton() == 0) // 좌클릭
                {
                    // 빈 공간 클릭 시 선택 해제, 점 위 클릭 시 선택
                    if (m_HoveredIndex != -1)
                        m_SelectedIndex = m_HoveredIndex;
                    else
                        m_SelectedIndex = -1;
                }
            }
        }
    }

    void EditorLayer::OnDetach()
    {
        glDeleteVertexArrays(1, &m_VertexArray);
        glDeleteBuffers(1, &m_VertexBuffer);
        glDeleteProgram(m_ShaderProgram);
    }

    void EditorLayer::CheckShaderError(GLuint shader, const char* type) { /* ... 기존 코드 ... */ }
    void EditorLayer::CheckProgramError(GLuint program) { /* ... 기존 코드 ... */ }

    bool EditorLayer::CalculatePlaneIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, float& t)
    {
        float denom = glm::dot(planeNormal, rayDir);

        // denom이 0이면 Ray와 평면이 평행하다는 뜻 (만나지 않음)
        if (abs(denom) > 1e-6)
        {
            glm::vec3 p0l0 = planePoint - rayOrigin;
            t = glm::dot(p0l0, planeNormal) / denom;
            return (t >= 0); // t가 양수여야 카메라 앞쪽에서 만남
        }
        return false;
    }
}
