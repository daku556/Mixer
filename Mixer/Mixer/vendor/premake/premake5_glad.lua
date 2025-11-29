project "Glad"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"../Glad/include/glad/glad.h",
        "../Glad/include/KHR/khrplatform.h",
        "../Glad/src/glad.c"
	}

    includedirs
    {
        "../Glad/include"
    }

	filter "system:windows"
		systemversion "latest"
        -- staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/MT"