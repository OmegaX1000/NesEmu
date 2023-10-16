#pragma once
#include "NesCPU.h"
#include "imgui.h"
#include "imgui_memory_editor.h"

#include <string>
#include "spdlog/fmt/fmt.h"

namespace NesEmulator
{
	//The machine that holds of of our components to emulate.
	class NesConsole
	{
		private:
			NesCPU CPU;		  //The CPU (6502)
			UInt8 Ram[65536]; //Our 64Kb of RAM

			MemoryEditor MemoryViewer;

		public:

			//Constructor & Destructor
			NesConsole();

			//RAM Functions
			void WriteRAM(UInt16 Address, UInt8 Data);
			UInt8 ReadRAM(UInt16 Address);
			void DrawRamContents(int StartAddress);

			//Device Getters
			NesCPU* GetCPU();

			//Etc Functions
			
	};
}