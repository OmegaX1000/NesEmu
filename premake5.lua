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
group ""

include "Source/"
include "Test/"