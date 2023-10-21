#pragma once
#include "NesCPU.h"
#include "NesCartridge.h"
#include "imgui.h"
#include "imgui_memory_editor.h"

#include <string>
#include <memory>
#include "spdlog/fmt/fmt.h"

namespace NesEmulator
{
	//The machine that holds of of our components to emulate.
	class NesConsole
	{
		private:
			NesCPU CPU;		  //The CPU (6502)
			UInt8 Ram[65536]; //Our 64Kb of RAM

			//The Cartridge that holds our Program.
			std::shared_ptr<NesCartridge> Cartridge;

			//Debug Stuff
			MemoryEditor MemoryViewer;

		public:

			//Constructor & Destructor
			NesConsole();

			//RAM Functions
			void WriteRAM(UInt16 Address, UInt8 Data); //CPU WriteRam Function.
			UInt8 ReadRAM(UInt16 Address);			   //CPU ReadRam Function.

			void InsertCartridge(std::string_view NewCartPath);

			//Device Getters
			NesCPU* GetCPU();

			//Debug Functions
			void DrawRamContents(int StartAddress);
	};
}