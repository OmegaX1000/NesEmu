#include "NesPPU.h"

namespace NesEmulator
{
	NesPPU::NesPPU()
	{
		//Setup Registers If Needed
		Status = 0;

		//Fill in our available palettes.
		std::ifstream PaletteFile;
		PaletteFile.open("F:/OmegaGamingHunters Folder/TestNES Emulator/Build/Emulator/Distribute_x64/Assets/Palette/Original Hardware (FBX).pal", std::ifstream::binary);

		for (int i = 0; i < 64; i++)
		{
			UInt8 ColorBytes[3];
			PaletteFile.read((char*)ColorBytes, sizeof(ColorBytes));
			PaletteColors[i].x = ColorBytes[0] / float(0xFF);
			PaletteColors[i].y = ColorBytes[1] / float(0xFF);
			PaletteColors[i].z = ColorBytes[2] / float(0xFF);
			PaletteColors[i].w = 1.00f;		
		}

		for (auto& i : PaletteRAM)
		{
			i = 0x00;
		}

		for (auto& i : NameTable[0])
		{
			i = 0x00;
		}
		for (auto& i : NameTable[1])
		{
			i = 0x00;
		}

		PaletteFile.close();

		PaletteRAM[0x00] = 0x22;
		PaletteRAM[0x01] = 0x29;
		PaletteRAM[0x02] = 0x1A;
		PaletteRAM[0x03] = 0x0F;

		//Allocate our pixel data.
		PixelOutputData = new UInt8[245760];
		std::fill_n(PixelOutputData, 245760, 0);
	}

	void NesPPU::Clock(Diligent::IRenderDevice* RenderDevice)
	{
		if (ScanlineCounter == -1 || ScanlineCounter == 261)
		{
			//Turn off VBlank
			if (ScanlineCounter == -1 && PixelCounter == 1)
			{
				Status = Status & ~((UInt8)1 << 7);
			}
		}
		else if (ScanlineCounter >= 0 && ScanlineCounter <= 239)
		{
		
		}
		else if (ScanlineCounter == 240)
		{

		}
		else if (ScanlineCounter >= 241 && ScanlineCounter <= 260)
		{
			//Turn on VBlanking
			if (ScanlineCounter == 241 && PixelCounter == 1)
			{
				//Turn on at the second tick of the scanline.
				Status |= 0x80;

				EvenFrames = !EvenFrames;

				if (Controller & 0x80)
				{
					NMI = true;
				}
			}
		}

		UpdateVideoOutput(RenderDevice);
		PixelCounter++;
		ClockCounter++;

		//Increment the Scanline Counter every 341 Cycles.
		if (PixelCounter >= 341)
		{
			if (ScanlineCounter == 261)
			{
				ScanlineCounter = -1;
				PixelCounter = 0;
			}
			else
			{
				ScanlineCounter++;
				PixelCounter = 0;
			}
		}
	}
	void NesPPU::Reset()
	{
		LeftPatternTable.Release();
		RightPatternTable.Release();
	}
	void NesPPU::ConnectCartridge(const std::shared_ptr<NesCartridge>& Cartridge)
	{
		this->Cartridge = Cartridge;
	}
	void NesPPU::UpdateVideoOutput(Diligent::IRenderDevice* RenderDevice)
	{
		if (VideoOutput == nullptr)
		{
			Diligent::TextureDesc VideoOutputDesc;
			VideoOutputDesc.Name = "Video Output";
			VideoOutputDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
			VideoOutputDesc.Width = 256;
			VideoOutputDesc.Height = 240;
			VideoOutputDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
			VideoOutputDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			VideoOutputDesc.Usage = Diligent::USAGE_DYNAMIC;

			Diligent::TextureSubResData TextureData[] = { {PixelOutputData, 4 * UInt64{VideoOutputDesc.Width}} };
			Diligent::TextureData VideoData(TextureData, _countof(TextureData));
			Diligent::RefCntAutoPtr<Diligent::ITexture> TexData;
			RenderDevice->CreateTexture(VideoOutputDesc, &VideoData, &TexData);
			VideoOutput = TexData->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
		}
		else
		{

		}
	}

	void NesPPU::CPUWrite(UInt16 Address, UInt8 Data)
	{
		switch (Address)
		{
			case 0x000: Controller = Data; break;
			case 0x001: Mask = Data; break;
			case 0x002: break;
			case 0x003: OAM_Address = Data; break;
			case 0x004: OAM_Data = Data; break;
			case 0x005: 
			{
				if (WriteToggle == false)
				{
					Scroll = Data;
					WriteToggle = true;
				}
				else
				{
					Scroll = (PPUAddress << 8) | Data;
					WriteToggle = false;
				}

				break;
			}
			case 0x006: 
			{
				if (WriteToggle == false)
				{
					PPUAddress = Data;
					WriteToggle = true;
				}
				else
				{
					PPUAddress = (PPUAddress << 8) | Data;
					WriteToggle = false;
				}

				break;
			}
			case 0x007: 
			{
				PPUData = Data; 
				PPUWrite(PPUAddress, PPUData);

				UInt8 Increment = (Controller & 0x04) ? 32 : 1;
				PPUAddress += Increment;
				break;
			}
		}
	}
	UInt8 NesPPU::CPURead(UInt16 Address)
	{
		UInt8 Data = 0;

		switch (Address)
		{
			case 0x000: break;
			case 0x001: break;
			case 0x002: 
			{
				Data = Status;
				Status = Status & ~((UInt8)1 << 7);
				WriteToggle = false;
				break;
			}
			case 0x003: break;
			case 0x004: 
			{
				Data = OAM_Data;
				break;
			}
			case 0x005: return 0x00; break;
			case 0x006: return 0x00; break;
			case 0x007: 
			{
				Data = PPUData;
				UInt8 Increment = (Controller & 0x04) ? 32 : 1;
				PPUAddress += Increment;
				break;
			}
		}

		return Data;
	}
	void NesPPU::PPUWrite(UInt16 Address, UInt8 Data)
	{
		if (Cartridge->PPUWrite(Address, Data))
		{

		}
		else if (Address >= 0x2000 && Address <= 0x3EFF)
		{
			UInt8 TableNum = floor(((Address - 0x2000) & 0x1000) / 0x400);
			NameTable[TableNum][Address & 0x0400] = Data;
		}
		else if (Address >= 0x3F00 && Address <= 0x3FFF)
		{
			PaletteRAM[Address] = Data;
		}
	}
	UInt8 NesPPU::PPURead(UInt16 Address)
	{
		UInt8 ReturnData = 0;

		if (Cartridge->PPURead(Address, ReturnData))
		{

		}
		else if (Address >= 0x2000 && Address <= 0x3EFF)
		{
			UInt8 TableNum = floor(((Address - 0x2000) & 0x1000) / 0x400);
			ReturnData = NameTable[TableNum][Address & 0x0400];
		}
		else if (Address >= 0x3F00 && Address <= 0x3FFF)
		{
			ReturnData = PaletteRAM[Address];
		}

		return ReturnData;
	}

	void NesPPU::DrawVideo()
	{
		ImGui::Begin("Video");
		ImGui::Image((void*)VideoOutput, ImVec2(512, 480), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		ImGui::End();
	}
	void NesPPU::DrawRegisters()
	{
		ImGui::Begin("PPU Registers");

		ImGui::Text("Clock Counter: %d", ClockCounter);
		ImGui::Text("Scanline: %d", ScanlineCounter);
		ImGui::Text("Pixel Counter: %d", PixelCounter);

		ImGui::Separator();

		ImGui::Text("Controller: 0x%X", Controller);
		ImGui::Indent();
		ImGui::Text("Nametable Select: %d", Controller & 0x07);
		ImGui::Text("Increment Mode: %d", (Controller & 0x04) ? 1 : 0);
		ImGui::Text("Sprite Tile Select: %d", (Controller & 0x08) ? 1 : 0);
		ImGui::Text("Background Tile Select: %d", (Controller & 0x10) ? 1 : 0);
		ImGui::Text("Sprite Size: %d", (Controller & 0x20) ? 1 : 0);
		ImGui::Text("PPU Master/Slave: %d", (Controller & 0x40) ? 1 : 0);
		ImGui::Text("NMI Enable: %d", (Controller & 0x80) ? 1 : 0);
		ImGui::Unindent();

		ImGui::Text("Mask: 0x%X", Mask);
		ImGui::Indent();
		ImGui::Text("Greyscale: %d", (Mask & 0x01) ? 1 : 0);
		ImGui::Text("BG Left Col Enable: %d", (Mask & 0x02) ? 1 : 0);
		ImGui::Text("SP Left Col Enable: %d", (Mask & 0x04) ? 1 : 0);
		ImGui::Text("Background Enable: %d", (Mask & 0x08) ? 1 : 0);
		ImGui::Text("Sprite Enable: %d", (Mask & 0x10) ? 1 : 0);
		ImGui::Text("Emphasize Red: %d", (Mask & 0x20) ? 1 : 0);
		ImGui::Text("Emphasize Green: %d", (Mask & 0x40) ? 1 : 0);
		ImGui::Text("Emphasize Blue: % d", (Mask & 0x80) ? 1 : 0);
		ImGui::Unindent();

		ImGui::Text("Status: 0x%X", Status);
		ImGui::Indent();
		ImGui::Text("Sprite Overflow: %d", (Status & 0x20) ? 1 : 0);
		ImGui::Text("Sprite Zero: %d", (Status & 0x40) ? 1 : 0);
		ImGui::Text("VBlank: %d", (Status & 0x80) ? 1 : 0);
		ImGui::Unindent();

		ImGui::Text("OAM Address: 0x%X", OAM_Address);
		ImGui::Text("OAM Data: 0x%X", OAM_Data);
		ImGui::Text("Scroll: 0x%X", Scroll);
		ImGui::Text("PPU Address: 0x%X", PPUAddress);
		ImGui::Text("PPU Data: 0x%X", PPUData);
		ImGui::Text("OAM DMA: 0x%X", OAM_DMA);

		ImGui::End();
	}
	void NesPPU::DrawPalettes()
	{
		static UInt8 Index = 0;
		ImGui::Begin("Palette Windows");
		ImGui::Text("Palette: 0x%X", Index);

		for (UInt8 i = 0; i < 64; i++)
		{
			std::string Text = "##Color " + i;
			ImGui::ColorEdit4(Text.c_str(), &PaletteColors[i].x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoLabel);
			
			if (ImGui::IsItemHovered())
			{
				Index = i;
			}

			if ((i + 1) % 16 != 0)
			{
				ImGui::SameLine(0.0f, 1.0f);
			}
		}

		ImGui::End();
	}
	void NesPPU::DrawPatternTable()
	{
		static UInt8 Index = 0;
		ImGui::Begin("Pattern Table");

		ImGui::BeginGroup();
		static UInt8 LeftTileIndex = 0;
		ImGui::Image((void*)LeftPatternTable, ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		if (ImGui::IsItemHovered())
		{
			ImVec2 ImagePos = ImGui::GetItemRectMin();
			ImVec2 Size = ImGui::GetItemRectSize();
			ImVec2 CursorPos = ImVec2(ImGui::GetMousePos().x - ImagePos.x, ImGui::GetMousePos().y - ImagePos.y);

			LeftTileIndex = (std::floor(CursorPos.y / 16) * 16) + std::floor(CursorPos.x / 16);
		}
		else
		{
			LeftTileIndex = 0;
		}
		ImGui::Text("Tile: 0x%X", LeftTileIndex);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		static UInt8 RightTileIndex = 0;
		ImGui::Image((void*)RightPatternTable, ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		if (ImGui::IsItemHovered())
		{
			ImVec2 ImagePos = ImGui::GetItemRectMin();
			ImVec2 Size = ImGui::GetItemRectSize();
			ImVec2 CursorPos = ImVec2(ImGui::GetMousePos().x - ImagePos.x, ImGui::GetMousePos().y - ImagePos.y);

			RightTileIndex = (std::floor(CursorPos.y / 16) * 16) + std::floor(CursorPos.x / 16);
		}
		else
		{
			RightTileIndex = 0;
		}
		ImGui::Text("Tile: 0x%X", RightTileIndex);
		ImGui::EndGroup();

		ImGui::Separator();

		ImGui::Text("Palette: 0x%X", Index);

		for (UInt8 i = 0; i < 32; i++)
		{
			ImGui::ColorEdit4("##Color", &PaletteColors[PaletteRAM[i]].x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoLabel);

			if ((i + 1) % 16 != 0)
			{
				ImGui::SameLine(0.0f, 1.0f);
			}

			if (ImGui::IsItemHovered())
			{
				Index = PaletteRAM[i];
			}
		}

		ImGui::End();
	}
	void NesPPU::DrawNametable()
	{

	}

	void NesPPU::UpdateDebugPatternTable(Diligent::IRenderDevice* RenderDevice)
	{
		Diligent::TextureDesc PatternTexDesc;
		PatternTexDesc.Type      = Diligent::RESOURCE_DIM_TEX_2D;
		PatternTexDesc.Width     = 128;
		PatternTexDesc.Height    = 128;
		PatternTexDesc.Format    = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
		PatternTexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		PatternTexDesc.Usage     = Diligent::USAGE_IMMUTABLE;

		UInt8* LeftPatternTablePixel = new UInt8[65536];
		UInt8* RightPatternTablePixel = new UInt8[65536];

		std::fill_n(LeftPatternTablePixel, 65536, 200);
		std::fill_n(RightPatternTablePixel, 65536, 200);

		//Left Pattern Table
		for (int y = 0; y < 16; y++)
		{
			UInt16 YOffset = y * 256;

			for (int x = 0; x < 16; x++)
			{
				UInt16 XOffset = x * 16;

				for (int row = 0; row < 8; row++)
				{
					UInt8 PlaneZero = PPURead(0x0000 + YOffset + XOffset + row);
					UInt8 PlaneOne = PPURead(0x0000 + YOffset + XOffset + row + 8);

					for (int col = 0; col < 8; col++)
					{
						UInt8 PalLow = PlaneZero  & 0x01;
						UInt8 PalHigh = (PlaneOne & 0x01) << 1;
						UInt8 Palette = PalHigh + PalLow;

						PlaneZero >>= 1;
						PlaneOne >>= 1;

						UInt16 RedPixelPos = (YOffset * 16) + (XOffset * 2) + (row * 512) + ((7 - col) * 4);
						UInt16 GreenPixelPos = (YOffset * 16) + (XOffset * 2) + (row * 512) + ((7 - col) * 4) + 1;
						UInt16 BluePixelPos = (YOffset * 16) + (XOffset * 2) + (row * 512) + ((7 - col) * 4) + 2;
						UInt16 AlphaPixelPos = (YOffset * 16) + (XOffset * 2) + (row * 512) + ((7 - col) * 4) + 3;

						UInt8 RedValue = PaletteColors[PaletteRAM[Palette]].x * float(0xFF);
						UInt8 GreenValue = PaletteColors[PaletteRAM[Palette]].y * float(0xFF);
						UInt8 BlueValue = PaletteColors[PaletteRAM[Palette]].z * float(0xFF);

						LeftPatternTablePixel[RedPixelPos] = RedValue;
						LeftPatternTablePixel[GreenPixelPos] = GreenValue;
						LeftPatternTablePixel[BluePixelPos] = BlueValue;
						LeftPatternTablePixel[AlphaPixelPos] = 255;
					}
				}
			}
		}

		PatternTexDesc.Name = "Left Pattern Table";
		Diligent::TextureSubResData TextureOneData[] = { {LeftPatternTablePixel, 4 * UInt64{PatternTexDesc.Width}} };
		Diligent::TextureData LeftPTData(TextureOneData, _countof(TextureOneData));
		Diligent::RefCntAutoPtr<Diligent::ITexture> LeftPT;
		RenderDevice->CreateTexture(PatternTexDesc, &LeftPTData, &LeftPT);
		LeftPatternTable = LeftPT->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

		//Right Pattern Table
		for (int y = 0; y < 16; y++)
		{
			UInt16 YOffset = y * 256;

			for (int x = 0; x < 16; x++)
			{
				UInt16 XOffset = x * 16;

				for (int row = 0; row < 8; row++)
				{
					UInt8 PlaneZero = PPURead(0x1000 + YOffset + XOffset + row);
					UInt8 PlaneOne = PPURead(0x1000 + YOffset + XOffset + row + 8);

					for (int col = 0; col < 8; col++)
					{
						UInt8 PalLow = PlaneZero & 0x01;
						UInt8 PalHigh = (PlaneOne & 0x01) << 1;
						UInt8 Palette = PalHigh + PalLow;

						PlaneZero >>= 1;
						PlaneOne >>= 1;

						UInt16 RedPixelPos = (YOffset * 16) + (XOffset * 2) + (row * 512) + ((7 - col) * 4);
						UInt16 GreenPixelPos = (YOffset * 16) + (XOffset * 2) + (row * 512) + ((7 - col) * 4) + 1;
						UInt16 BluePixelPos = (YOffset * 16) + (XOffset * 2) + (row * 512) + ((7 - col) * 4) + 2;
						UInt16 AlphaPixelPos = (YOffset * 16) + (XOffset * 2) + (row * 512) + ((7 - col) * 4) + 3;

						UInt8 RedValue = PaletteColors[PaletteRAM[Palette]].x * float(0xFF);
						UInt8 GreenValue = PaletteColors[PaletteRAM[Palette]].y * float(0xFF);
						UInt8 BlueValue = PaletteColors[PaletteRAM[Palette]].z * float(0xFF);

						RightPatternTablePixel[RedPixelPos] = RedValue;
						RightPatternTablePixel[GreenPixelPos] = GreenValue;
						RightPatternTablePixel[BluePixelPos] = BlueValue;
						RightPatternTablePixel[AlphaPixelPos] = 255;
					}
				}
			}
		}

		PatternTexDesc.Name = "Right Pattern Table";
		Diligent::TextureSubResData TextureTwoData[] = { {RightPatternTablePixel, 4 * UInt64{PatternTexDesc.Width}} };
		Diligent::TextureData RightPTData(TextureTwoData, _countof(TextureTwoData));
		Diligent::RefCntAutoPtr<Diligent::ITexture> RightPT;
		RenderDevice->CreateTexture(PatternTexDesc, &RightPTData, &RightPT);
		RightPatternTable = RightPT->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

		delete[] LeftPatternTablePixel;
		delete[] RightPatternTablePixel;
	}
}