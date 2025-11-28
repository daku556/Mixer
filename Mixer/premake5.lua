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
include "Mixer/vendor/GLFW"

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
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/src/Mixer",
		"%{prj.name}/vendor/spdlog/include",
		"%{Includedir.GLFW}"
	}

	links
	{
		"GLFW",
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
		}

		postbuildcommands
		{
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
		"Mixer/src"
	}

	links
	{
		"Mixer"
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
		defines "MX_DIST"
		buildoptions "/MD"
		optimize "On"