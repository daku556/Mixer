This project is 3th Computer Graphics lecture project




프로젝트 목표
: 3D 모델링 툴 제작

--------------------------------

프로젝트 세부사항
1. 마우스 조작을 통해 정점과 선분을 추가할 수 있다.
2. 카메라를 조작하여 물체를 둘러볼 수 있다.
3. 점을 이동, 복제할 수 있다.
4. 점을 선으로, 선을 면으로 만들 수 있다.

--------------------------------

빌드하고 실행하기
1. 프로젝트를 다운받아 주세요. git clone 이나 zip파일로 받으시면 됩니다.
2. 현재 윈도우에서 visual studio만 지원합니다. vs 버전에 따라서 2019 라면 generateproject.bat을 2022라면 2022가 붙은 배치파일을 실행하세요.
3. 솔루션 파일이 생성될겁니다. 이를 vs로 켜서 솔루션 빌드를 진행하면 됩니다. 로그출력을 안하고 최적화된 버전은 상단에서 dist로 변경하세요.


---------------------------------

사용한 오픈소스 라이브러리


glad: opengl 로더
https://github.com/Dav1dde/glad

GLFW: 창 생성 및 입력
https://github.com/glfw/glfw

glm: 백터, 행렬 연산 
https://github.com/g-truc/glm

premake: 플랫폼 빌드설정, VS 설정 자동화
https://github.com/premake/premake-core

spdlog: 디버깅 로그
https://github.com/gabime/spdlog

stb: 이미지 로드
https://github.com/nothings/stb

imgui: gui생성에서 사용하려 했으나 사용하지 않음
https://github.com/ocornut/imgui

hazel: 기본 구조 사용
https://github.com/TheCherno/Hazel
