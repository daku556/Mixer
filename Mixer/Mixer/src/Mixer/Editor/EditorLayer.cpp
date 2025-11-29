#include "EditorLayer.h"
#include <iostream>
#include <unordered_map>
#include <set>
#include <algorithm> // std::min, std::max
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mixer/Events/MouseEvent.h"
#include "Mixer/Events/ApplicationEvent.h"
#include "Mixer/Events/KeyEvent.h"
#include "Mixer/Application.h"

namespace Mixer {

    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    // -------------------------------------------------------------------------
    // [1] 생명주기 (Lifecycle)
    // -------------------------------------------------------------------------
    void EditorLayer::OnAttach()
    {
        // 1. 데이터 초기화 (사각형: 점 4개)
        m_Vertices = {
            { -0.5f, -0.5f, 0.5f }, // 0: 좌하
            {  0.5f, -0.5f, 0.5f }, // 1: 우하
            {  0.5f,  0.5f, 0.5f }, // 2: 우상
            { -0.5f,  0.5f, 0.5f }, // 3: 좌상
            { -0.5f, -0.5f, -0.5f }, // 4: 좌하
            {  0.5f, -0.5f, -0.5f }, // 5: 우하
            {  0.5f,  0.5f, -0.5f }, // 6: 우상
            { -0.5f,  0.5f, -0.5f }  // 7: 좌상
        };

        // [중요 변경] GL_LINES용(점 2개)이 아니라 GL_TRIANGLES용(점 3개) 인덱스로 변경
        // 사각형은 삼각형 2개로 구성됩니다 (0-1-2, 2-3-0)
        m_Indices = {
            0, 1, 2,
            2, 3, 0,
            1, 2, 5,
            2, 5, 6,
            1, 0, 5,
            0, 5, 4,
            0, 4, 3,
            3, 4, 7,
            5, 4, 6,
            7, 4, 6,
            2, 3, 6,
            3, 6, 7
        };

        glGenBuffers(1, &m_EdgeIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EdgeIndexBuffer);

        // 2. 그래픽 자원 생성 (함수 분리)
        CreateGraphicsPipeline();

        // [수정] 그리드 초기화
        m_GridSystem.Init();
    }

    void EditorLayer::OnDetach()
    {
        glDeleteVertexArrays(1, &m_VertexArray);
        glDeleteBuffers(1, &m_VertexBuffer);
        glDeleteBuffers(1, &m_IndexBuffer);
        glDeleteProgram(m_ShaderProgram);
        // [수정] 그리드 해제
        m_GridSystem.Shutdown();
    }

    void EditorLayer::OnUpdate()
    {
        auto& window = Application::Get().GetWindow();
        float width = (float)window.GetWidth();
        float height = (float)window.GetHeight();
        if (width == 0 || height == 0) return;

        // 1. 준비
        // [수정] 깊이 테스트 활성화! (이 한 줄이면 뒤에 있는 점들이 가려집니다)
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!m_IsTranslationMode)
        {
            UpdateCameraControl();
        }

        UpdateCamera(width, height); // 행렬 계산
        // [수정] 렌더링 호출 (카메라 행렬만 넘겨주면 알아서 그림)
        m_GridSystem.Render(m_ViewProjection);
        HandleInteraction();         // 모델링 로직
        RenderScene();               // 렌더링
    }

    // -------------------------------------------------------------------------
    // [2] 핵심 로직 분리 (Logic Separation)
    // -------------------------------------------------------------------------

    void EditorLayer::UpdateCamera(float width, float height)
    {
        float aspectRatio = width / height;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

        // [수정] 구면 좌표계 공식을 이용해 카메라 위치(Position) 계산
        // x = r * sin(yaw) * cos(pitch)
        // y = r * sin(pitch)
        // z = r * cos(yaw) * cos(pitch)

        // Pitch 제한 (고개 너무 젖히면 뒤집힘 방지)
        if (m_CameraPitch > 1.565f) m_CameraPitch = 1.565f;
        if (m_CameraPitch < -1.565f) m_CameraPitch = -1.565f;

        // [수정] 헬퍼 함수 사용
        glm::vec3 position = GetCameraPosition();

        // LookAt(카메라위치, 바라보는점, 위쪽방향)
        glm::mat4 view = glm::lookAt(position, m_CameraFocalPoint, glm::vec3(0.0f, 1.0f, 0.0f));

        m_ViewProjection = projection * view;
    }

    void EditorLayer::HandleInteraction()
    {
        // 1. 현재 카메라 위치 계산 (UpdateCamera와 동일한 공식)
        // [수정] 헬퍼 함수 사용으로 로직 통일
        glm::vec3 cameraPos = GetCameraPosition();
        // 2. 카메라가 바라보는 방향 (Forward Vector) 계산
        // [수정] 평면의 Normal은 (카메라 위치 - 타겟 위치) 방향 (카메라가 평면을 바라보게)
        glm::vec3 planeNormal = glm::normalize(cameraPos - m_CameraFocalPoint);
        glm::vec3 rayDir = GetRayFromMouse();
        glm::vec3 rayOrigin = cameraPos;

        if (m_IsTranslationMode && !m_SelectedIndices.empty())
        {
            // 평면의 기준점은 '첫 번째 선택된 점의 원래 위치'로 잡습니다. (누구를 잡든 상관없음)
            // (주의: m_MultiVertexStartPositions[0]이 안전한지 empty 체크 필수)
            glm::vec3 planePoint = m_MultiVertexStartPositions[0];
            // [핵심 수정] planeNormal을 고정값 (0,0,1)에서 -> 카메라 방향으로 변경!
            // 이렇게 하면 카메라가 어디를 보든 정점은 항상 "카메라 앞 유리창" 위에서 움직입니다.

            float t = 0.0f;

            // 평면 교차 계산
            if (CalculatePlaneIntersection(rayOrigin, rayDir, planePoint, planeNormal, t))
            {
                glm::vec3 currentMousePoint = rayOrigin + (rayDir * t);

                // [추가된 로직] 모드 진입 후 첫 프레임이라면? -> 지금 위치를 기준점으로 삼는다!
                if (!m_TranslationInit)
                {
                    m_MouseStartDragPoint = currentMousePoint;
                    m_TranslationInit = true; // 초기화 완료
                }

                // 델타 계산 및 위치 적용
                glm::vec3 delta = currentMousePoint - m_MouseStartDragPoint;
                // [수정] 선택된 모든 점에 델타 적용
                for (size_t i = 0; i < m_SelectedIndices.size(); i++)
                {
                    int idx = m_SelectedIndices[i];
                    // 각 점의 '원래 위치' + 델타
                    m_Vertices[idx] = m_MultiVertexStartPositions[i] + delta;
                }
            }
        }
        else
        {
            // [일반 모드] Hover 감지
            m_HoveredIndex = -1;
            float minDistance = 1000.0f;

            for (int i = 0; i < m_Vertices.size(); i++)
            {
                // 거리 계산도 카메라 위치 기준으로 해야 정확함
                float dist = glm::length(glm::cross(m_Vertices[i] - rayOrigin, rayDir));
                if (dist < 0.3f && dist < minDistance)
                {
                    minDistance = dist;
                    m_HoveredIndex = i;
                }
            }
        }
    }

    void EditorLayer::RenderScene()
    {
        // 1. GPU 버퍼 업데이트
        // VBO 업데이트
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3), m_Vertices.data(), GL_DYNAMIC_DRAW);

        // IBO 업데이트
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(uint32_t), m_Indices.data(), GL_DYNAMIC_DRAW);

        // [추가] Edge IBO 업데이트
        if (!m_EdgeIndices.empty())
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EdgeIndexBuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_EdgeIndices.size() * sizeof(uint32_t), m_EdgeIndices.data(), GL_DYNAMIC_DRAW);
        }

        // 2. 셰이더 설정
        glUseProgram(m_ShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_ViewProjection"), 1, GL_FALSE, glm::value_ptr(m_ViewProjection));
        glBindVertexArray(m_VertexArray);

        // ---------------------------------------------------------------------
        // [PASS 1] 면(Face) 그리기 (Solid)
        // ---------------------------------------------------------------------

        // [핵심 수정] 그리기 직전에 "나는 면 버퍼를 쓸 거야!"라고 VAO에게 다시 알려줘야 함
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);

        // 면 채우기 모드 설정
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // 면 색상 (약간 어두운 회색) - Blender 기본 느낌
        glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 0.5f, 0.5f, 0.5f, 1.0f);

        // [중요] Polygon Offset Enable
        // 면을 그릴 때 깊이 값을 살짝 뒤로 밀어줍니다. 그래야 나중에 그릴 선(Wireframe)이 묻히지 않습니다.
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, nullptr);

        glDisable(GL_POLYGON_OFFSET_FILL); // 끄기

        // ---------------------------------------------------------------------
        // [PASS 2] 와이어프레임(Edge) 그리기 (Lines)
        // ---------------------------------------------------------------------

        // A. 면의 테두리 (Triangles -> Lines)
        // [핵심 수정] 여기도 명시적으로 바인딩 (이미 위에서 되어있지만 안전하게)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);

        // 선 그리기 모드 설정
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f); // 선 두께

        // 선 색상 (검은색 혹은 밝은 색)
        glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 0.0f, 0.0f, 0.0f, 1.0f); // 검은색 테두리

        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, nullptr);

        // B. [추가] 고립된 선(Isolated Lines) 그리기
        if (!m_EdgeIndices.empty())
        {
            // [핵심 수정] 이제 "나는 선 버퍼를 쓸 거야!"라고 교체
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EdgeIndexBuffer);

            // GL_LINES 모드로 그립니다 (점 2개씩 끊어서 그림)
            glDrawElements(GL_LINES, m_EdgeIndices.size(), GL_UNSIGNED_INT, nullptr);
        }

        // 다시 원래대로 복구
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // ---------------------------------------------------------------------
        // [PASS 3] 점(Vertex) 그리기 (기존 동일)
        // ---------------------------------------------------------------------
        glEnable(GL_PROGRAM_POINT_SIZE);

        for (int i = 0; i < m_Vertices.size(); i++)
        {
            // [수정] 현재 점(i)이 선택된 리스트에 있는지 확인
            bool isSelected = false;
            for (int selectedIdx : m_SelectedIndices) {
                if (selectedIdx == i) {
                    isSelected = true;
                    break;
                }
            }

            if (isSelected)
            {
                if (m_IsTranslationMode)
                    glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f); // 이동 중: 흰색
                else
                    glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 0.0f, 0.5f, 1.0f, 1.0f); // 선택됨: 파랑
            }
            else if (i == m_HoveredIndex)
            {
                glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 0.0f, 1.0f, 0.0f, 1.0f); // Hover: 녹색
            }
            else
            {
                glUniform4f(glGetUniformLocation(m_ShaderProgram, "u_Color"), 1.0f, 0.5f, 0.0f, 1.0f); // 기본: 주황
            }

            glDrawArrays(GL_POINTS, i, 1);
        }
    }

    // -------------------------------------------------------------------------
    // [3] 초기화 헬퍼 (Initialization Helpers)
    // -------------------------------------------------------------------------
    void EditorLayer::CreateGraphicsPipeline()
    {
        // VAO
        glGenVertexArrays(1, &m_VertexArray);
        glBindVertexArray(m_VertexArray);

        // VBO
        glGenBuffers(1, &m_VertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3), m_Vertices.data(), GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        // IBO
        glGenBuffers(1, &m_IndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(uint32_t), m_Indices.data(), GL_DYNAMIC_DRAW);

        // Shaders
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        const char* vsSrc = GetVertexShaderSource();
        glShaderSource(vs, 1, &vsSrc, nullptr);
        glCompileShader(vs);
        CheckShaderError(vs, "VERTEX");

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fsSrc = GetFragmentShaderSource();
        glShaderSource(fs, 1, &fsSrc, nullptr);
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

    const char* EditorLayer::GetVertexShaderSource()
    {
        return R"(#version 330 core
layout(location = 0) in vec3 a_Pos;
uniform mat4 u_ViewProjection;
void main() {
    gl_Position = u_ViewProjection * vec4(a_Pos, 1.0);
    gl_PointSize = 20.0;
}
)";
    }

    const char* EditorLayer::GetFragmentShaderSource()
    {
        return R"(#version 330 core
layout(location = 0) out vec4 FragColor;
uniform vec4 u_Color;
void main() {
    FragColor = u_Color;
}
)";
    }

    // -------------------------------------------------------------------------
    // [4] 이벤트 처리 (Event Handling)
    // -------------------------------------------------------------------------
    void EditorLayer::OnEvent(Event& e)
    {
        if (e.GetEventType() == EventType::MouseMoved)
        {
            MouseMovedEvent& event = (MouseMovedEvent&)e;
            glm::vec2 currentPos = { event.GetX(), event.GetY() };

            // 델타 계산
            m_MouseDelta = currentPos - m_MousePos;
            m_MousePos = currentPos; // 현재 위치 갱신
        }
        else if (e.GetEventType() == EventType::WindowResize)
        {
            WindowResizeEvent& event = (WindowResizeEvent&)e;
            glViewport(0, 0, event.GetWidth(), event.GetHeight());
        }
        if (e.GetEventType() == EventType::KeyPressed) {
            KeyPressedEvent& event = (KeyPressedEvent&)e;
            if (event.GetKeyCode() == 340 || event.GetKeyCode() == 344) m_IsShiftPressed = true; // Shift (GLFW 기준)
            if (event.GetKeyCode() == 341 || event.GetKeyCode() == 345) m_IsCtrlPressed = true; // Ctrl
            // ... G/E Key 로직 ...
            // 'G' Key (71) 이동
            if (event.GetKeyCode() == 71 && !m_SelectedIndices.empty() && !m_IsTranslationMode)
            {
                m_IsTranslationMode = true;
                m_TranslationInit = false; // [핵심] "아직 초기값 계산 안 했어!" 라고 표시
                // [수정] 선택된 모든 점의 현재 위치를 백업 (취소 및 델타 계산용)
                m_MultiVertexStartPositions.clear();
                for (int index : m_SelectedIndices)
                {
                    m_MultiVertexStartPositions.push_back(m_Vertices[index]);
                }
            }
            // [신규] 'E' Key (Extrude - 돌출)
            // 로직: 선택된 점 복제 -> 선 연결 -> 이동 모드 진입
            else if (event.GetKeyCode() == 69 && !m_SelectedIndices.empty() && !m_IsTranslationMode)
            {
                // 선택된 점이 있고, 이동 모드가 아닐 때
               
                // 1. [점 1개 선택 시] -> 선(Edge) 생성 로직
                if (m_SelectedIndices.size() == 1)
                {
                    int oldIdx = m_SelectedIndices[0];
                    glm::vec3 currentPos = m_Vertices[oldIdx];

                    // 1. 점 복제 (위치는 같음)
                    m_Vertices.push_back(currentPos);
                    int newIdx = (int)m_Vertices.size() - 1;

                    // 2. [중요] 선 인덱스(m_EdgeIndices)에만 추가!
                    // 기존 m_Indices(면)에는 절대 손대지 마세요.
                    m_EdgeIndices.push_back(oldIdx);
                    m_EdgeIndices.push_back(newIdx);

                    // 3. 선택 변경 (이제부터 움직일 놈은 새 점이다)
                    m_SelectedIndices.clear();
                    m_SelectedIndices.push_back(newIdx);

                    // 디버깅용 로그
                    // std::cout << "Extruded Vertex! Old: " << oldIdx << " New: " << newIdx << std::endl;
                }
                // 2. [점 2개 이상 선택 시] -> 면(Face) 생성 로직 (기존 코드)
                else
                {
                    // ... (아까 작성한 면 생성 로직 그대로 유지) ...
                    // indexMap 만들고, processedEdges 체크해서 m_Indices(면)에 추가하는 코드
                    // 1. [매핑 준비] 옛날 인덱스 -> 새 인덱스
                    std::unordered_map<int, int> indexMap;
                    std::vector<int> newSelectedIndices;

                    // 2. [점 복제] 선택된 점들을 복사해서 m_Vertices 뒤에 추가
                    for (int oldIdx : m_SelectedIndices)
                    {
                        glm::vec3 currentPos = m_Vertices[oldIdx];

                        m_Vertices.push_back(currentPos); // 점 추가

                        int newIdx = (int)m_Vertices.size() - 1;
                        indexMap[oldIdx] = newIdx; // 족보 기록 (예: 1번 점은 4번이 되었다)
                        newSelectedIndices.push_back(newIdx); // 나중에 얘네를 선택해야 함
                    }

                    // 3. [면 생성] 기존 연결 정보를 보고, 선택된 점들 사이의 "빈 공간"을 면으로 채움

                    // 중복 엣지 생성 방지용 (1-2 엣지를 처리했는데 2-1에서 또 만들면 안 되니까)
                    std::set<std::pair<int, int>> processedEdges;

                    // 기존 인덱스 버퍼의 크기를 미리 저장 (루프 돌면서 push_back 하면 무한루프 돌 수 있음)
                    size_t oldIndexCount = m_Indices.size();

                    for (size_t i = 0; i < oldIndexCount; i += 3)
                    {
                        // 현재 삼각형의 세 점
                        int idx[3] = { (int)m_Indices[i], (int)m_Indices[i + 1], (int)m_Indices[i + 2] };

                        // 삼각형의 3개 엣지를 각각 검사 (0-1, 1-2, 2-0)
                        for (int j = 0; j < 3; j++)
                        {
                            int a = idx[j];
                            int b = idx[(j + 1) % 3];

                            // 두 점(Edge)이 모두 "이번에 Extrude된 점" 인가?
                            if (indexMap.find(a) != indexMap.end() && indexMap.find(b) != indexMap.end())
                            {
                                // 이미 처리한 엣지인지 확인 (순서 무관하게 저장)
                                int minIdx = std::min(a, b);
                                int maxIdx = std::max(a, b);
                                if (processedEdges.find({ minIdx, maxIdx }) == processedEdges.end())
                                {
                                    // 처리 안 했으면 -> 쿼드(Quad) 생성!
                                    // A -- B
                                    // |    |
                                    // A'-- B'
                                    int a_new = indexMap[a];
                                    int b_new = indexMap[b];

                                    // 삼각형 1: (A, B, B')
                                    m_Indices.push_back(a);
                                    m_Indices.push_back(b);
                                    m_Indices.push_back(b_new);

                                    // 삼각형 2: (B', A', A) - 순서는 렌더링 면 방향(Culling) 고려
                                    // (양면 렌더링을 끄면 순서가 중요하지만, 지금은 일단 채우는 게 목표)
                                    m_Indices.push_back(b_new);
                                    m_Indices.push_back(a_new);
                                    m_Indices.push_back(a);

                                    // 처리 목록에 등록
                                    processedEdges.insert({ minIdx, maxIdx });
                                }
                            }
                        }

                        // [옵션] 뚜껑(Cap) 생성: 삼각형의 세 점이 모두 선택되었다면, 그 면도 복사해줌
                        if (indexMap.find(idx[0]) != indexMap.end() &&
                            indexMap.find(idx[1]) != indexMap.end() &&
                            indexMap.find(idx[2]) != indexMap.end())
                        {
                            m_Indices.push_back(indexMap[idx[0]]);
                            m_Indices.push_back(indexMap[idx[1]]);
                            m_Indices.push_back(indexMap[idx[2]]);
                        }
                    }

                    m_SelectedIndices = newSelectedIndices;
                }

                // 3. 공통: 이동 모드 진입 (G키와 완전히 동일한 깔끔한 코드)
                m_IsTranslationMode = true;
                m_TranslationInit = false; // [핵심] 여기서도 계산 안 함! HandleInteraction에게 미룸

                m_MultiVertexStartPositions.clear();
                for (int index : m_SelectedIndices)
                    m_MultiVertexStartPositions.push_back(m_Vertices[index]);
                
            }
            // J Key code 
            else if (event.GetKeyCode() == 74) 
            {
                // 정확히 2개가 선택되었을 때만 동작
                if (m_SelectedIndices.size() == 2)
                {
                    int idx1 = m_SelectedIndices[0];
                    int idx2 = m_SelectedIndices[1];

                    // 선 인덱스에 추가
                    m_EdgeIndices.push_back(idx1);
                    m_EdgeIndices.push_back(idx2);

                    // 로그
                    // std::cout << "Joined vertices " << idx1 << " and " << idx2 << std::endl;
                }
            }
            // [신규] 'F' Key (Face) - 면 생성
            else if (event.GetKeyCode() == 70) // F Key code
            {
                // 점이 3개 이상이어야 면을 만들 수 있음
                if (m_SelectedIndices.size() >= 3)
                {
                    // Triangle Fan 알고리즘 사용
                    // 첫 번째 선택된 점(root)을 기준으로 부채꼴 모양으로 삼각형을 만듦
                    int rootIdx = m_SelectedIndices[0];

                    for (size_t i = 1; i < m_SelectedIndices.size() - 1; i++)
                    {
                        int idxB = m_SelectedIndices[i];
                        int idxC = m_SelectedIndices[i + 1];

                        // 면 인덱스(m_Indices)에 추가
                        // 반시계 방향(CCW)이 앞면이므로 순서 유의
                        m_Indices.push_back(rootIdx);
                        m_Indices.push_back(idxB);
                        m_Indices.push_back(idxC);
                    }

                    // 로그
                    // std::cout << "Created Face with " << m_SelectedIndices.size() << " vertices." << std::endl;
                }
        }
            
            // ---------------------------------------------------------
            // [신규] 뷰포트 전환 (Numpad)
            // ---------------------------------------------------------

            // Numpad 1 (Front / Back)
            if (event.GetKeyCode() == 321) // GLFW_KEY_KP_1
            {
                m_CameraPitch = 0.0f; // 수평
                if (m_IsCtrlPressed) m_CameraYaw = PI; // Back (뒤에서 봄)
                else                 m_CameraYaw = 0.0f; // Front (앞에서 봄)
            }

            // Numpad 3 (Right / Left) - 덤으로 넣어드림
            else if (event.GetKeyCode() == 323) // GLFW_KEY_KP_3
            {
                m_CameraPitch = 0.0f;
                if (m_IsCtrlPressed) m_CameraYaw = -HALF_PI; // Left
                else                 m_CameraYaw = HALF_PI;  // Right
            }

            // Numpad 7 (Top / Bottom)
            else if (event.GetKeyCode() == 327) // GLFW_KEY_KP_7
            {
                m_CameraYaw = 0.0f;
                // 주의: 정확히 90도(HALF_PI)가 되면 LookAt 함수 계산 시 Up벡터와 겹쳐서
                // 화면이 뒤집히거나(Gimbal Lock) 깨질 수 있습니다.
                // 아주 미세하게 작은 값을 빼줍니다 (예: 1.56f)
                float safePitch = 1.56f;

                if (m_IsCtrlPressed) m_CameraPitch = -safePitch; // Bottom
                else                 m_CameraPitch = safePitch;  // Top
            }

            // Numpad 9 (Top / Bottom 반대 기능 - 블렌더 스타일)
            // 만약 Ctrl+7이 불편하면 9번을 Bottom으로 써도 됩니다.
            else if (event.GetKeyCode() == 329) // GLFW_KEY_KP_9
            {
                // 토글 로직 등을 넣을 수 있음 (생략)
            }
        }
        else if (e.GetEventType() == EventType::KeyReleased) {
            KeyReleasedEvent& event = (KeyReleasedEvent&)e;
            if (event.GetKeyCode() == 340 || event.GetKeyCode() == 344) m_IsShiftPressed = false;

            // [추가] Ctrl 해제 (Left Ctrl: 341, Right Ctrl: 345)
            if (event.GetKeyCode() == 341 || event.GetKeyCode() == 345) m_IsCtrlPressed = false;
        }
        else if (e.GetEventType() == EventType::MouseButtonPressed)
        {
            MouseButtonPressedEvent& event = (MouseButtonPressedEvent&)e;

            if (event.GetMouseButton() == 2) { // Middle Button (Wheel Click)
                if (m_IsShiftPressed) m_IsPanning = true;
                else m_IsRotating = true;
            }

            if (m_IsTranslationMode)
            {
                // [이동 모드 중]
                if (event.GetMouseButton() == 0) // 좌클릭: 확정
                {
                    m_IsTranslationMode = false;
                }
                else if (event.GetMouseButton() == 1) // 우클릭: 취소
                {
                    // [수정] 모든 점을 원래 위치로 원상복구
                    for (size_t i = 0; i < m_SelectedIndices.size(); i++)
                    {
                        int idx = m_SelectedIndices[i];
                        m_Vertices[idx] = m_MultiVertexStartPositions[i];
                    }
                    m_IsTranslationMode = false;
                }
            }
            else
            {
                // [일반 모드: 선택]
                if (event.GetMouseButton() == 0) // 좌클릭
                {
                    if (m_HoveredIndex != -1)
                    {
                        // 1. Shift 키를 누른 상태 (다중 선택/토글)
                        if (m_IsShiftPressed)
                        {
                            // 이미 선택된 놈인지 확인
                            auto it = std::find(m_SelectedIndices.begin(), m_SelectedIndices.end(), m_HoveredIndex);
                            if (it != m_SelectedIndices.end())
                            {
                                // 이미 있으면 -> 선택 해제 (제거)
                                m_SelectedIndices.erase(it);
                            }
                            else
                            {
                                // 없으면 -> 추가
                                m_SelectedIndices.push_back(m_HoveredIndex);
                            }
                        }
                        // 2. 그냥 클릭 (단일 선택)
                        else
                        {
                            // 기존 선택 다 지우고, 얘만 선택
                            m_SelectedIndices.clear();
                            m_SelectedIndices.push_back(m_HoveredIndex);
                        }
                    }
                    else
                    {
                        // 빈 공간 클릭 -> 모두 선택 해제
                        // (단, Shift 누른 상태에서는 유지하는 게 툴 국룰이지만, 일단 다 해제)
                        if (!m_IsShiftPressed)
                            m_SelectedIndices.clear();
                    }
                }
            }
        }
        else if (e.GetEventType() == EventType::MouseButtonReleased) {
            MouseButtonReleasedEvent& event = (MouseButtonReleasedEvent&)e;
            if (event.GetMouseButton() == 2) {
                m_IsRotating = false;
                m_IsPanning = false;
            }
        }
        else if (e.GetEventType() == EventType::MouseScrolled) {
            MouseScrolledEvent& event = (MouseScrolledEvent&)e;
            float zoomSpeed = 0.5f;
            m_CameraDistance -= event.GetYOffset() * zoomSpeed;
            if (m_CameraDistance < 0.1f) m_CameraDistance = 0.1f; // 너무 가까워지지 않게
        }
    }

    // -------------------------------------------------------------------------
    // [5] 수학 및 기타 헬퍼 (Math Helpers)
    // -------------------------------------------------------------------------
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

    bool EditorLayer::CalculatePlaneIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, float& t)
    {
        float denom = glm::dot(planeNormal, rayDir);
        if (abs(denom) > 1e-6)
        {
            glm::vec3 p0l0 = planePoint - rayOrigin;
            t = glm::dot(p0l0, planeNormal) / denom;
            return (t >= 0);
        }
        return false;
    }

    void EditorLayer::CheckShaderError(GLuint shader, const char* type) {
        GLint success;
        char infoLog[1024];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            MX_CORE_ERROR("SHADER_ERROR ( {0} ): {1}", type, infoLog);
        }
    }
    void EditorLayer::CheckProgramError(GLuint program) {
        GLint success;
        char infoLog[1024];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 1024, NULL, infoLog);
            MX_CORE_ERROR("PROGRAM_ERROR: {0}", infoLog);
        }
    }
    void EditorLayer::UpdateCameraControl()
    {
        // 델타값이 너무 튀지 않게 확인
        if (glm::length(m_MouseDelta) > 100.0f) { m_MouseDelta = { 0,0 }; return; }

        if (m_IsRotating)
        {
            // [회전] Pitch, Yaw 업데이트 (기존과 동일)
            float rotationSpeed = 0.005f;
            m_CameraYaw += m_MouseDelta.x * rotationSpeed;
            m_CameraPitch += m_MouseDelta.y * rotationSpeed;
        }
        else if (m_IsPanning)
        {
            // [이동] Focal Point 자체를 이동 (Local Axis 적용)
            float panSpeed = 0.005f * m_CameraDistance;

            // 1. 카메라의 현재 Basis Vector(축) 계산
            //    (카메라 위치를 알아야 방향을 구할 수 있음)
            glm::vec3 position = GetCameraPosition();

            // Forward: 카메라가 바라보는 방향
            glm::vec3 forward = glm::normalize(m_CameraFocalPoint - position);
            glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

            // Right: 카메라의 오른쪽 방향 (Forward와 WorldUp의 외적)
            glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));

            // Up: 카메라의 위쪽 방향 (Right와 Forward의 외적)
            // (WorldUp을 그대로 쓰면 위를 보고 있을 때 이동이 이상해짐)
            glm::vec3 cameraUp = glm::normalize(glm::cross(right, forward));

            // 2. 이동 적용
            // 마우스를 오른쪽으로 드래그(+X) -> 카메라는 왼쪽(-Right)으로 가야 물체가 오른쪽으로 옴
            m_CameraFocalPoint -= right * m_MouseDelta.x * panSpeed;

            // 마우스를 아래로 드래그(+Y) -> 카메라는 위쪽(+Up)으로 가야 물체가 아래로 옴
            // (좌표계 방향이 헷갈린다면 + / - 를 반대로 바꿔보세요)
            m_CameraFocalPoint += cameraUp * m_MouseDelta.y * panSpeed;
        }

        // 델타 초기화
        m_MouseDelta = { 0.0f, 0.0f };
    }
    glm::vec3 EditorLayer::GetCameraPosition()
    {
        glm::vec3 position;
        position.x = m_CameraFocalPoint.x + m_CameraDistance * sin(m_CameraYaw) * cos(m_CameraPitch);
        position.y = m_CameraFocalPoint.y + m_CameraDistance * sin(m_CameraPitch);
        position.z = m_CameraFocalPoint.z + m_CameraDistance * cos(m_CameraYaw) * cos(m_CameraPitch);
        return position;
    }
}