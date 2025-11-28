#pragma once

#include "Mixer/Layer.h"

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
		// 헬퍼 함수: 마우스 위치에서 Ray 방향을 계산
		glm::vec3 GetRayFromMouse();

		// 헬퍼 함수: 셰이더 오류 체크
		void CheckShaderError(GLuint shader, const char* type);
		void CheckProgramError(GLuint program);

	private:
		GLuint m_ShaderProgram;
		GLuint m_VertexArray;
		GLuint m_VertexBuffer;
		
		glm::mat4 m_ViewProjection;
		glm::vec2 m_MousePos = { 0.0f, 0.0f };

		// [핵심 데이터]
		std::vector<glm::vec3> m_Vertices; // 실제 정점 데이터 저장소

		// [상호작용 상태]
		int m_HoveredIndex = -1;  // 마우스가 올라간 점
		int m_SelectedIndex = -1; // 클릭해서 선택된 점 (이동 대상)
		//bool m_IsDragging = false; // 현재 드래그 중인가?

		// [G키 이동 모드 관련 변수]
		bool m_IsTranslationMode = false;     // 현재 'G'키를 눌러 이동 중인가?
		glm::vec3 m_VertexStartPos;           // 이동 시작 전 정점의 원래 위치 (취소용)
		glm::vec3 m_MouseStartDragPoint;      // 이동 시작 시점의 마우스-평면 교차점 (델타 계산용)

	private:
		bool CalculatePlaneIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, float& t);

	};
}