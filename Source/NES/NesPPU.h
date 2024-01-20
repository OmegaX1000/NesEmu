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
			union Mask
			{
				struct
				{
					UInt8 Grayscale : 1;
					UInt8 RenderBGLeft : 1;
					UInt8 RenderSpriteLeft : 1;
					UInt8 RenderBG : 1;
					UInt8 RenderSprites : 1;
					UInt8 Red : 1;
					UInt8 Green : 1;
					UInt8 Blue : 1;
				};

				UInt8 Register;
			};
			union Status
			{
				struct
				{
					UInt8 Unused : 5;
					UInt8 SpriteOverflow : 1;
					UInt8 SpriteZero : 1;
					UInt8 VerticalBlank : 1;
				};

				UInt8 Register = 0x00;
			};

			union SpriteAttribute
			{
				struct
				{
					UInt8 Palette : 2;
					UInt8 Unused : 3;
					UInt8 Priority : 1;
					UInt8 FlipH : 1;
					UInt8 FlipV : 1;
				};

				UInt8 Register;
			};
			struct SpriteData
			{
				UInt8 PosX;
				UInt8 PosY;
				UInt8 TileIndex;
				SpriteAttribute Attribute;
			};

		private:

			//The console this device is a part of.
			std::shared_ptr<NesCartridge> Cartridge;
			UInt16 PixelCounter = 0; //341 per scanline.
			UInt32 ClockCounter = 0;
			UInt32 FrameCounter = 0;

			//Memory
			UInt8 PaletteRAM[0x20];
			UInt8 NameTable[2][1024];

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
			UInt8 SpriteListCount = 0;
			UInt8 SpritePatternLow[8];
			UInt8 SpritePatternHigh[8];
			UInt8 SpriteAttributes[8];
			UInt8 SpriteXposCounters[8];

			UInt8 SpriteEvalEntryIndex = 0;
			Int16 SpriteEvalPosYdiff = 0;
			bool SpriteZeroHitPossible = false;
			bool SpriteZeroRender = false;

			UInt8 SpriteCurrentFetch = 0;
			UInt16 SpritePatternLowAddr = 0;
			UInt16 SpritePatternHighAddr = 0;
			UInt8 SpritePatternLowByte = 0;
			UInt8 SpritePatternHighByte = 0;

			//Helper Variables
			Int16 ScanlineCounter = -1;
			bool EvenFrames = false;

			//Helper Functions
			void ShiftBGRegisters(); //Shift our background registers.
			void LoadBGShiftRegisters(); //Loads up the shift registers for the background.
			void ShiftSpriteRegisters(); //Loads up the shift registers for the sprites.
			void GetBackgroundPixelData(UInt8 &PixelIndex, UInt8 &PaletteIndex);
			void GetSpritePixelData(UInt8& PixelIndex, UInt8& PaletteIndex, UInt8 &Priority);
			void GetFinalPixelData(UInt8 &OutPixel, UInt8 &OutPalette, UInt8 BgPixel, UInt8 BGpalette, UInt8 SpritePixel, UInt8 SpritePalette, UInt8 Priority);
			
			//Debug Stuff
			Diligent::RefCntAutoPtr<Diligent::ITextureView> LeftPatternTable;
			Diligent::RefCntAutoPtr<Diligent::ITextureView> RightPatternTable;
			Diligent::RefCntAutoPtr<Diligent::ITextureView> NametableView;

		public:

			//Constructor
			NesPPU();
			~NesPPU();

			//Registers
			Controller  Controller;
			Mask		Mask;
			Status		Status;
			UInt8		OAM_Address = 0;
			UInt8		OAM_Data = 0;
			UInt16		Scroll = 0;
			VRAMAddress PPUAddress;
			UInt8		PPUData = 0;
			UInt8		OAM_DMA = 0;

			//Public Variables
			UInt8* PixelOutputData;
			ImVec4 PaletteColors[64];
			bool FrameComplete = false;
			bool NMI = false;

			SpriteData PrimaryOAM[64];
			SpriteData SecondaryOAM[8];

			//Other Functions
			void Clock();
			void Reset();
			void ConnectCartridge(const std::shared_ptr<NesCartridge>& Cartridge);

			//Ram Functions
			void CPUWrite(UInt16 Address, UInt8 Data);
			UInt8 CPURead(UInt16 Address);
			void PPUWrite(UInt16 Address, UInt8 Data);
			UInt8 PPURead(UInt16 Address);

			//Debug Functions
			void DrawRegisters();
			void DrawPalettes();
			void DrawPatternTable();
			void DrawNametable();

			void UpdateDebugPatternTable(Diligent::IRenderDevice* RenderDevice);
	};
}