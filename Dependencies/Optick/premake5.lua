project "OpTick"
	kind "StaticLib"
	language "C++"

	targetdir "$(SolutionDir)Build/Dependencies/$(ProjectName)/$(Configuration)_$(Platform)/"
	objdir "$(SolutionDir)Build/Dependencies/$(ProjectName)/$(Configuration)_$(Platform)/"

	files
	{
		"src/**.config.h",
		"src/**.h",
		"src/**.cpp"
	}