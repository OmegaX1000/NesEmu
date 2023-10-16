project "ImGui"
	kind "StaticLib"
	language "C++"

	targetdir "$(SolutionDir)Build/Dependencies/$(ProjectName)/$(Configuration)_$(Platform)/"
	objdir "$(SolutionDir)Build/Dependencies/$(ProjectName)/$(Configuration)_$(Platform)/"

	files
	{
		"misc/cpp/imgui_stdlib.h",
		"misc/cpp/imgui_stdlib.cpp",
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_memory_editor.h",
		"imgui_internal.h",
		"imgui_tables.cpp",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"