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

			union VRAMAddress
			{
				struct
				{
					UInt16 CoarseX : 5;
					UInt16 CoarseY : 5;
					UInt16 NameX : 1;
					UInt16 NameY : 1;
					UInt16 FineY : 3;
					UInt16 Unused : 1;
				};

				UInt16 Register = 0x0000;
			};

			union Controller
			{
				struct
				{
					UInt8 NameX : 1;
					UInt8 NameY : 1;
					UInt8 IncrementMode : 1;
					UInt8 SpriteTableAddr : 1;
					UInt8 BGTableAddr : 1;
					UInt8 SpriteSize : 1;
					UInt8 MasterPin : 1;
					UInt8 NMIEnable : 1;
				};

				UInt8 Register = 0x00;
			};

		private:

			//The console this device is a part of.
			std::shared_ptr<NesCartridge> Cartridge;
			UInt16 PixelCounter = 0; //341 per scanline.
			UInt32 ClockCounter = 0;
			UInt32 FrameCounter = 0;

			//Memory
			ImVec4 PaletteColors[64];
			UInt8 PaletteRAM[0x20];
			UInt8 NameTable[2][1024];

			//Registers
			Controller Controller;
			UInt8  Mask		   = 0;
			UInt8  Status	   = 0;
			UInt8  OAM_Address = 0;
			UInt8  OAM_Data	   = 0;
			UInt16 Scroll	   = 0;
			VRAMAddress PPUAddress;
			UInt8  PPUData	   = 0;
			UInt8  OAM_DMA	   = 0;

			//Background Registers
			VRAMAddress TempVRaddr;
			UInt8 FineScrollX = 0;
			bool WriteToggle = false;

			UInt8 TileAddress       = 0;
			UInt8 AttributeByte     = 0;
			UInt8 PatternTableLow   = 0;
			UInt8 PatternTableHigh  = 0;

			UInt16 HighBGtileInfo   = 0;
			UInt16 LowBGtileInfo    = 0;
			UInt16 LowAttributeByte  = 0;
			UInt16 HighAttributeByte = 0;

			//Sprite Registers

			//Helper Variables
			Diligent::RefCntAutoPtr<Diligent::ITextureView> VideoOutput;
			UInt8* PixelOutputData;
			Int16 ScanlineCounter = -1;
			bool EvenFrames = false;

			//Helper Functions
			void ShiftBGRegisters(); //Shift our background registers.
			void LoadBGShiftRegisters(); //Loads up the shift registers for the background.
			void LoadSpriteShiftRegisters(); //Loads up the shift registers for the sprites.
			
			//Debug Stuff
			Diligent::RefCntAutoPtr<Diligent::ITextureView> LeftPatternTable;
			Diligent::RefCntAutoPtr<Diligent::ITextureView> RightPatternTable;
			Diligent::RefCntAutoPtr<Diligent::ITextureView> NametableView;

		public:

			//Constructor
			NesPPU();
			~NesPPU();

			//Public Variables
			bool FrameComplete = false;
			bool NMI = false;

			//Other Functions
			void Clock(Diligent::IRenderDevice* RenderDevice, Diligent::IDeviceContext* Context);
			void Reset();
			void ConnectCartridge(const std::shared_ptr<NesCartridge>& Cartridge);
			void UpdateVideoOutput(Diligent::IRenderDevice* RenderDevice, Diligent::IDeviceContext* Context);

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