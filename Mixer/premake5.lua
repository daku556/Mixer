workspace "Mixer"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

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
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
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
		symbols "On"

	filter "configurations:Release"
		defines "MX_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "MX_DIST"
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
		staticruntime "On"
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
		symbols "On"

	filter "configurations:Release"
		defines "MX_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "MX_DIST"
		optimize "On"