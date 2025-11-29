#pragma once

#include "Mixer/Layer.h"
#include "EditorGrid.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

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
		GLuint m_IndexBuffer; // [추가] 인덱스 버퍼 ID

		// [핵심 데이터]
		glm::mat4 m_ViewProjection;
		glm::vec2 m_MousePos = { 0.0f, 0.0f };
		glm::vec2 m_MouseDelta = { 0.0f, 0.0f }; // 이번 프레임의 마우스 이동량
		std::vector<glm::vec3> m_Vertices; // 실제 정점 데이터 저장소
		std::vector<uint32_t> m_Indices; // [추가] 정점 연결 순서 (0, 1, 2 ...)
		std::vector<uint32_t> m_EdgeIndices;
		GLuint m_EdgeIndexBuffer;

		// [상수] 자주 쓰는 각도 (라디안)
		const float PI = 3.1415926535f;
		const float HALF_PI = 1.5707963267f;

		// [상호작용 상태]
		int m_HoveredIndex = -1;  // 마우스가 올라간 점
		//int m_SelectedIndex = -1; // 클릭해서 선택된 점 (이동 대상)
		std::vector<int> m_SelectedIndices; // 선택된 점들의 인덱스 목록
		bool m_IsRotating = false; // 휠 클릭 중인가?
		bool m_IsPanning = false;  // Shift + 휠 클릭 중인가? (Key Event로 Shift 상태 체크 필요)
		bool m_IsShiftPressed = false;
		bool m_IsCtrlPressed = false; // [추가] Ctrl 키 상태
		//bool m_IsDragging = false; // 현재 드래그 중인가?

		// [G키 이동 모드 관련 변수]
		bool m_IsTranslationMode = false;     // 현재 'G'키를 눌러 이동 중인가?
		//glm::vec3 m_VertexStartPos;           // 이동 시작 전 정점의 원래 위치 (취소용)
		std::vector<glm::vec3> m_MultiVertexStartPositions; // 선택된 점들의 원래 위치 목록
		glm::vec3 m_MouseStartDragPoint;      // 이동 시작 시점의 마우스-평면 교차점 (델타 계산용)
		bool m_TranslationInit = false; // [추가] 이동 모드 초기화가 되었는가?

		// [카메라 관련 변수]
        glm::vec3 m_CameraFocalPoint = { 0.0f, 0.0f, 0.0f }; // 카메라가 바라보는 중심점 (Target)
        float m_CameraDistance = 5.0f; // 중심점과 카메라 사이의 거리
        
        // 회전 각도 (Radians)
        float m_CameraPitch = 0.0f; // 상하 회전 (X축 기준)
        float m_CameraYaw = 0.0f; // 좌우 회전 (Y축 기준)

        // 마우스 델타 계산용
        glm::vec2 m_LastMousePos = { 0.0f, 0.0f }; // 이전 프레임 마우스 위치

		// [에디터 그리드]
		EditorGrid m_GridSystem;

	private:
		// --- [내부 로직 분리] ---
		void CreateGraphicsPipeline();  // VAO, VBO, Shader 생성
		void UpdateCamera(float width, float height); // 카메라 행렬 계산
		void HandleInteraction();       // Raycasting 및 이동 로직 처리
		void RenderScene();             // 실제 그리기 명령 (Draw Calls)
		void UpdateCameraControl();		// 카메라 이동 조작
		
		// --- [헬퍼 함수] ---
		glm::vec3 GetRayFromMouse();
		bool CalculatePlaneIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, float& t);
		glm::vec3 GetCameraPosition();

		// 셰이더 관련
		void CheckShaderError(GLuint shader, const char* type);
		void CheckProgramError(GLuint program);
		const char* GetVertexShaderSource();
		const char* GetFragmentShaderSource();
		
	};
}