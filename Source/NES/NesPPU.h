#pragma once
#include "Definitions.h"
#include "NesCartridge.h"
#include "imgui.h"

#include "Primitives/interface/BasicTypes.h"
#include "Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "Graphics/GraphicsAccessories/interface/GraphicsAccessories.hpp"

#include <vector>
#include <string>
#include <fstream>

namespace NesEmulator
{
	//Picture Processing Unit
	class NesPPU
	{
		private:

			//The console this device is a part of.
			std::shared_ptr<NesCartridge> Cartridge;
			UInt16 PixelCounter = 0; //341 per scanline.
			UInt32 ClockCounter = 0;

			//Memory
			ImVec4 PaletteColors[64];
			UInt8 PaletteRAM[0x20];
			UInt8 NameTable[2][1024];

			//Registers
			UInt8  Controller  = 0;
			UInt8  Mask		   = 0;
			UInt8  Status	   = 0;
			UInt8  OAM_Address = 0;
			UInt8  OAM_Data	   = 0;
			UInt16 Scroll	   = 0;
			UInt16 PPUAddress  = 0;
			UInt8  PPUData	   = 0;
			UInt8  OAM_DMA	   = 0;

			//Background Registers
			UInt16 TempVRaddr = 0;
			UInt8 FineScrollX = 0;
			bool WriteToggle = false;

			UInt16 HighBGtileInfo = 0;
			UInt16 LowBGtileInfo = 0;
			UInt8 LowAttributeByte = 0;
			UInt8 HighAttributeByte = 0;

			//Sprite Registers

			//Helper Variables
			Diligent::RefCntAutoPtr<Diligent::ITextureView> VideoOutput;
			UInt8* PixelOutputData;
			Int16 ScanlineCounter = -1;
			bool EvenFrames = false;
			
			//Debug Stuff
			Diligent::RefCntAutoPtr<Diligent::ITextureView> LeftPatternTable;
			Diligent::RefCntAutoPtr<Diligent::ITextureView> RightPatternTable;

		public:

			//Constructor
			NesPPU();

			//Public Variables
			bool NMI = false;

			//Other Functions
			void Clock(Diligent::IRenderDevice* RenderDevice);
			void Reset();
			void ConnectCartridge(const std::shared_ptr<NesCartridge>& Cartridge);
			void UpdateVideoOutput(Diligent::IRenderDevice* RenderDevice);

			//Ram Functions
			void CPUWrite(UInt16 Address, UInt8 Data);
			UInt8 CPURead(UInt16 Address);
			void PPUWrite(UInt16 Address, UInt8 Data);
			UInt8 PPURead(UInt16 Address);

			//Debug Functions
			void DrawVideo(); //Temporary
			void DrawRegisters();
			void DrawPalettes();
			void DrawPatternTable();
			void DrawNametable();

			void UpdateDebugPatternTable(Diligent::IRenderDevice* RenderDevice);
	};
}