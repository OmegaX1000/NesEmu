#pragma once
#include "NesCPU.h"
#include "NesPPU.h"
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
			//Devices.
			NesCPU CPU;	//The CPU (6502)
			NesPPU PPU;	//The PPU

			UInt32 SystemClockCounter = 0;

			//Memory
			UInt8 Ram[2048]; //The CPU internal ram. (2Kb)
			//UInt8 ControllerOne;
			//UInt8 ControllerTwo;

			//The Cartridge that holds our Program.
			std::shared_ptr<NesCartridge> Cartridge;

		public:

			//Constructor & Destructor
			NesConsole();

			//Other Functions
			void Clock(Diligent::IRenderDevice* RenderDevice);
			void InsertCartridge(std::string_view NewCartPath);

			//RAM Functions
			void CPUWrite(UInt16 Address, UInt8 Data); //CPU WriteRam Function.
			UInt8 CPURead(UInt16 Address);			   //CPU ReadRam Function.

			//Device Getters
			NesCPU* GetCPU();
			NesPPU* GetPPU();

			//Debug Functions
			void DrawRamContents(int StartAddress);
	};
}