include "Dependencies.lua"

workspace "TestNES"
	architecture "x86_64"
	startproject "Emulator"

	configurations
	{ 
		"Debug", 
		"Release",
		"Distribute"
	}

	flags
	{
		"MultiProcessorCompile"
	}

group "Dependencies"
	include "Dependencies/imgui/"
	include "Dependencies/nativefiledialog/"
	include "Dependencies/Optick/"
group ""

include "Source/"