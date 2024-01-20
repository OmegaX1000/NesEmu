#pragma once
#include "NesCPU.h"
#include "NesPPU.h"
#include "NesCartridge.h"
#include "NesController.h"
#include "SDL_events.h"
#include "imgui.h"
#include "imgui_internal.h"
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

			UInt64 SystemClockCounter = 0;
			Diligent::RefCntAutoPtr<Diligent::ITextureView> VideoOutput;

			//Devices.
			NesCPU CPU;	//The CPU (6502)
			NesPPU PPU;	//The PPU

			//Memory
			UInt8 Ram[2048]; //The CPU internal ram. (2Kb)
			NesController ControllerOne;
			NesController ControllerTwo;

			//The Cartridge that holds our Program.
			std::shared_ptr<NesCartridge> Cartridge;

			//DMA
			bool DMA_Transfer = false;
			bool DMA_Wait = true;
			UInt8 DMA_Page = 0x00;
			UInt8 DMA_Address = 0x00;
			UInt8 DMA_Data = 0x00;

			//Other Variables
			bool PollingInput = false;

		public:

			//Constructor & Destructor
			NesConsole();

			//Other Functions
			void Clock();
			void Reset();
			void DrawVideo();
			void InsertCartridge(std::string_view NewCartPath);
			void UpdateVideoOutput(Diligent::IRenderDevice* RenderDevice, Diligent::IDeviceContext* Context);
			void PollControllers();

			void TurnoffPolling();

			//RAM Functions
			void CPUWrite(UInt16 Address, UInt8 Data); //CPU WriteRam Function.
			UInt8 CPURead(UInt16 Address);			   //CPU ReadRam Function.

			//Device Getters
			NesCPU* GetCPU();
			NesPPU* GetPPU();
			NesController* GetControllerOne();
			NesController* GetControllerTwo();
	};
}