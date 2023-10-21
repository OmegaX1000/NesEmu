project "NativeFileDialog"
	kind "StaticLib"
	language "C"

	targetdir "$(SolutionDir)Build/Dependencies/$(ProjectName)/$(Configuration)_$(Platform)/"
	objdir "$(SolutionDir)Build/Dependencies/$(ProjectName)/$(Configuration)_$(Platform)/"

	files
	{
		"src/include/nfd.h",
		"src/include/nfd.hpp"
	}

	includedirs
	{
		"src/include/"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "WIN32", "_WINDOWS" }
		files { "src/nfd_win.cpp" }

	filter "system:linux"
		pic "On"
		systemversion "latest"
		files { "src/nfd_gtk.cpp" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"