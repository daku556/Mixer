workspace "Mixer"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- include directories
Includedir = {}
Includedir["GLFW"] = "Mixer/vendor/GLFW/include"
Includedir["Glad"] = "Mixer/vendor/Glad/include"
Includedir["ImGui"] = "Mixer/vendor/imgui/include"
Includedir["glm"] = "Mixer/vendor/glm"
Includedir["stb"] = "Mixer/vendor/stb/include"

include "Mixer/vendor/premake/premake5_GLFW.lua"
include "Mixer/vendor/premake/premake5_glad.lua"
include "Mixer/vendor/premake/premake5_imgui.lua"

project "Mixer"
	location "Mixer"
	kind "SharedLib"
	language "C++"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/stb/include/**.h",
		"%{prj.name}/vendor/stb/include/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/src/Mixer",
		"%{prj.name}/vendor/spdlog/include",
		"%{Includedir.GLFW}",
		"%{Includedir.Glad}",
		"%{Includedir.ImGui}",
		"%{Includedir.glm}",
		"%{Includedir.stb}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		-- staticruntime "On"
		systemversion "latest"

		defines
		{
			"MX_BUILD_DLL",
			"MX_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
            ("{MKDIR} ../bin/" .. outputdir .. "/Sandbox"),
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

		buildoptions
		{
			"/utf-8"
		}

	filter "configurations:Debug"
		defines "MX_DEBUG"
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines "MX_RELEASE"
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		defines "MX_DIST"
		buildoptions "/MD"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	
	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs 
	{
		"Mixer/vendor/spdlog/include",
		"Mixer/src",
		"%{Includedir.glm}"
	}

	links
	{
		"Mixer"
	}

	postbuildcommands
    {
		-- 1. 목적지 폴더(assets)를 먼저 생성합니다. (Mixer 방식)
        ("{MKDIR} ../bin/" .. outputdir .. "/Sandbox/assets"),

        -- 2. xcopy로 내용물을 복사합니다. 
        -- [중요] xcopy는 경로에 백슬래시(\)가 필요하므로 Lua에서 \\로 적어줍니다.
        -- "../assets" 폴더의 내용을 -> "../bin/.../Sandbox/assets" 폴더로 복사
        ("xcopy /Q /E /Y /I \"assets\" \"..\\bin\\" .. outputdir .. "\\Sandbox\\assets\"")
    }

	filter "system:windows"
		cppdialect "C++17"
		-- staticruntime "On"
		systemversion "latest"

		defines
		{
			"MX_PLATFORM_WINDOWS",
		}

		buildoptions
		{
			"/utf-8"
		}

	filter "configurations:Debug"
		defines "MX_DEBUG"
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines "MX_RELEASE"
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		kind "WindowedApp"
		defines "MX_DIST"
		buildoptions "/MD"
		optimize "On"
        entrypoint "mainCRTStartup"