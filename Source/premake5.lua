project "Emulator"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir "$(SolutionDir)Build/$(ProjectName)/$(Configuration)_$(Platform)/"
	objdir "!$(SolutionDir)Build/$(ProjectName)/$(Configuration)_$(Platform)/"

	files
	{
		"Core/**.h",
		"Core/**.cpp",
		"NES/**.h",
		"NES/**.cpp"

	}

	includedirs
	{
		"Core/",
		"NES/",
		"%{IncludeDir.SDL2}",
		"%{IncludeDir.Spdlog}",
		"%{IncludeDir.Diligent}",
		"%{IncludeDir.Imgui}",
		"%{IncludeDir.ImGuiFonts}",
		"%{IncludeDir.NatFileDial}"
	}

	libdirs
	{
		"%{LibDir.SDL2}",
		"%{LibDir.Diligent}"
	}

	links
	{
		"ImGui",
		"NativeFileDialog",
		"SDL2.lib",
		"SDL2main.lib",
		"DiligentCore.lib",
		"glslang.lib",
		"HLSL.lib",
		"OGLCompiler.lib",
 		"OSDependent.lib",
 		"spirv-cross-core.lib", 
		"SPIRV.lib", 
		"SPIRV-Tools-opt.lib", 
		"SPIRV-Tools.lib",
		"glew-static.lib", 
		"GenericCodeGen.lib", 
		"MachineIndependent.lib",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"
		links 
		{ 
			"dxgi.lib", 
			"d3d11.lib", 
			"d3d12.lib",
			"d3dcompiler.lib"
		}

		defines
		{
			"NOMINMAX"
		}