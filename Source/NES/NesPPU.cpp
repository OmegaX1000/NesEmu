#include "NesPPU.h"
#include "imgui_memory_editor.h"

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

		//Allocate our pixel data.
		PixelOutputData = new UInt8[245760];
		std::fill_n(PixelOutputData, 245760, 0);
	}
	NesPPU::~NesPPU()
	{
		delete[] PixelOutputData;
	}

	void NesPPU::Clock(Diligent::IRenderDevice* RenderDevice, Diligent::IDeviceContext* Context)
	{
		if (ScanlineCounter >= -1 && ScanlineCounter <= 240)
		{
			//Get the info needed to render a pixel (fetching bg tiles and sprites.
			if (PixelCounter >= 1 && PixelCounter <= 256)
			{
				//Disable Vertical Blank
				if (ScanlineCounter == -1 && PixelCounter == 1)
				{
					Status = Status & ~((UInt8)1 << 7);
				}

				if (PixelCounter >= 2)
				{
					ShiftBGRegisters();
				}

				//Fetch tile data and load them into our shift registers every 8 cycles.
				switch ((PixelCounter - 1) % 8)
				{
					case 0:
					{
						LoadBGShiftRegisters();
						TileAddress = PPURead(0x2000 | (PPUAddress.Register & 0x0FFF));
						break;
					}
					case 2:
					{
						AttributeByte = PPURead(0x23C0 | (PPUAddress.NameY << 11) | (PPUAddress.NameX << 10) | ((PPUAddress.CoarseY >> 2) << 3) | (PPUAddress.CoarseX >> 2));

						if (PPUAddress.CoarseY & 0x02)
						{
							AttributeByte >>= 4;
						}
						if (PPUAddress.CoarseX & 0x02)
						{
							AttributeByte >>= 2;
						}

						AttributeByte &= 0x03;

						break;
					}
					case 4:
					{
						PatternTableLow = PPURead((Controller.BGTableAddr << 12) + ((UInt16)TileAddress << 4) + (PPUAddress.FineY) + 0);
						break;
					}
					case 6:
					{
						PatternTableHigh = PPURead((Controller.BGTableAddr << 12) + ((UInt16)TileAddress << 4) + (PPUAddress.FineY) + 8);
						break;
					}
					case 7:
					{
						//Increment our XScoll, Ony if rendering is enabled
						if (Mask & 0x08 || Mask & 0x10)
						{
							if ((PPUAddress.CoarseX & 0x1F) == 31)
							{
								//Leaving nametable so wrap address round
								PPUAddress.CoarseX = 0;
								//Flip target nametable bit
								PPUAddress.NameX = ~PPUAddress.NameX;
							}
							else
							{
								// Staying in current nametable, so just increment
								PPUAddress.CoarseX++;
							}
						}

						break;
					}
				}

				//At the end of the visible scanline, we scroll down.
				if (PixelCounter == 256)
				{
					if (Mask & 0x08 || Mask & 0x10)
					{
						if (PPUAddress.FineY < 7)
						{
							PPUAddress.FineY++;
						}
						else
						{
							PPUAddress.FineY = 0;

							//Check if we need to swap vertical nametable targets
							if (PPUAddress.CoarseY == 29)
							{
								PPUAddress.CoarseY = 0;
								PPUAddress.NameY = ~PPUAddress.NameY;
							}
							else if (PPUAddress.CoarseY == 31)
							{
								PPUAddress.CoarseY = 0;
							}
							else
							{
								PPUAddress.CoarseY++;
							}
						}
					}
				}
			}
			else if (PixelCounter >= 257 && PixelCounter <= 320)
			{
				if (PixelCounter == 257)
				{
					ShiftBGRegisters();
					LoadBGShiftRegisters();

					//Ony if rendering is enabled
					if (Mask & 0x08 || Mask & 0x10)
					{
						PPUAddress.NameX = TempVRaddr.NameX;
						PPUAddress.CoarseX = TempVRaddr.CoarseX;
					}
				}

				if (ScanlineCounter == -1 && PixelCounter >= 280 && PixelCounter < 305)
				{
					//Ony if rendering is enabled
					if (Mask & 0x08 || Mask & 0x10)
					{
						PPUAddress.FineY = TempVRaddr.FineY;
						PPUAddress.NameY = TempVRaddr.NameY;
						PPUAddress.CoarseY = TempVRaddr.CoarseY;
					}
				}
			}
			else if (PixelCounter >= 321 && PixelCounter <= 336)
			{
				ShiftBGRegisters();

				//Fetch tile data and load them into our shift registers every 8 cycles.
				switch ((PixelCounter - 1) % 8)
				{
					case 0:
					{
						LoadBGShiftRegisters();
						TileAddress = PPURead(0x2000 | (PPUAddress.Register & 0x0FFF));
						break;
					}
					case 2:
					{
						AttributeByte = PPURead(0x23C0 | (PPUAddress.NameY << 11) | (PPUAddress.NameX << 10) | ((PPUAddress.CoarseY >> 2) << 3) | (PPUAddress.CoarseX >> 2));

						if (PPUAddress.CoarseY & 0x02) AttributeByte >>= 4;
						if (PPUAddress.CoarseX & 0x02) AttributeByte >>= 2;
						AttributeByte &= 0x03;

						break;
					}
					case 4:
					{
						PatternTableLow = PPURead((Controller.BGTableAddr << 12) + ((UInt16)TileAddress << 4) + (PPUAddress.FineY) + 0);
						break;
					}
					case 6:
					{
						PatternTableHigh = PPURead((Controller.BGTableAddr << 12) + ((UInt16)TileAddress << 4) + (PPUAddress.FineY) + 8);
						break;
					}
					case 7:
					{
						//Increment our XScoll, Ony if rendering is enabled
						if (Mask & 0x08 || Mask & 0x10)
						{
							if ((PPUAddress.CoarseX & 0x1F) == 31)
							{
								//Leaving nametable so wrap address round
								PPUAddress.CoarseX = 0;
								//Flip target nametable bit
								PPUAddress.NameX = ~PPUAddress.NameX;
							}
							else
							{
								// Staying in current nametable, so just increment
								PPUAddress.CoarseX++;
							}
						}

						break;
					}
				}
			}
			else if (PixelCounter >= 337 && PixelCounter <= 340)
			{
				ShiftBGRegisters();

				if (PixelCounter == 338 || PixelCounter == 340)
				{
					TileAddress = PPURead(0x2000 | (PPUAddress.Register & 0x0FFF));
				}
			}
		}
		else if (ScanlineCounter >= 241 && ScanlineCounter < 261)
		{
			//Turn on VBlanking
			if (ScanlineCounter == 241 && PixelCounter == 1)
			{
				//Turn on at the second tick of the scanline.
				Status |= 0x80;

				if (Controller.NMIEnable)
				{
					NMI = true;
				}

				UpdateVideoOutput(RenderDevice, Context);
			}
		}

		//Create our pixel.
		UInt8 PixelIndex = 0x00;   //The 2-bit pixel to be rendered
		UInt8 PaletteIndex = 0x00; //The 3-bit index of the palette the pixel indexes

		if (Mask & 0x08)
		{
			UInt16 bit_mux = 0x8000 >> Scroll;

			UInt8 PixIndexLow = (LowBGtileInfo & bit_mux) > 0;
			UInt8 PixIndexHigh = (HighBGtileInfo & bit_mux) > 0;

			//Combine to form pixel index
			PixelIndex = (PixIndexHigh << 1) | PixIndexLow;

			//Get palette
			UInt8 PalIndexLow = (LowAttributeByte & bit_mux) > 0;
			UInt8 PalIndexHigh = (HighAttributeByte & bit_mux) > 0;
			PaletteIndex = (PalIndexHigh << 1) | PalIndexLow;
		}

		UInt8 PalatteSelect = PPURead(0x3F00 + (PaletteIndex << 2) + PixelIndex) & 0x3F;
		UInt8 RedPixel = PaletteColors[PalatteSelect].x * float(0xFF);
		UInt8 GreenPixel = PaletteColors[PalatteSelect].y * float(0xFF);
		UInt8 BluePixel = PaletteColors[PalatteSelect].z * float(0xFF);

		//Put our pixel in our VideoOutput texture.
		if (PixelCounter >= 1 && PixelCounter <= 256)
		{
			if (ScanlineCounter >= 0 && ScanlineCounter <= 239)
			{
				UInt32 XOffset = (PixelCounter - 1) * 4;
				UInt32 YOffset = ScanlineCounter * 1024;

				PixelOutputData[YOffset + XOffset] = RedPixel;
				PixelOutputData[YOffset + XOffset + 1] = GreenPixel;
				PixelOutputData[YOffset + XOffset + 2] = BluePixel;
				PixelOutputData[YOffset + XOffset + 3] = 255;
			}
		}

		//Update our output.
		PixelCounter++;
		ClockCounter++;

		//Increment the Scanline Counter every 341 Cycles.
		if (PixelCounter >= 341)
		{
			if (ScanlineCounter == 261)
			{
				ScanlineCounter = -1;
				FrameComplete = true;
				EvenFrames = !EvenFrames;
				PixelCounter = 0;
				FrameCounter++;
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
		ScanlineCounter = -1;
		PixelCounter = 0;
		ClockCounter = 0;

		Controller.Register = 0;
		Mask = 0;
		Scroll = 0;
		PPUAddress.Register = 0x0000;
		TempVRaddr.Register = 0x0000;
		PPUData = 0;

		TileAddress = 0;
		AttributeByte = 0;
		PatternTableLow = 0;
		PatternTableHigh = 0;

		HighBGtileInfo = 0;
		LowBGtileInfo = 0;
		LowAttributeByte = 0;
		HighAttributeByte = 0;

		WriteToggle = false;

		//Allocate our pixel data.
		delete[] PixelOutputData;
		PixelOutputData = new UInt8[245760];
		std::fill_n(PixelOutputData, 245760, 0);

		LeftPatternTable.Release();
		RightPatternTable.Release();
		NametableView.Release();
	}
	void NesPPU::ConnectCartridge(const std::shared_ptr<NesCartridge>& Cartridge)
	{
		this->Cartridge = Cartridge;
	}
	void NesPPU::UpdateVideoOutput(Diligent::IRenderDevice* RenderDevice, Diligent::IDeviceContext* Context)
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
			TexData.Release();
		}
		else
		{
			Diligent::Box MapRegion;
			UInt32 Width = 256;
			UInt32 Height = 240;
			MapRegion.MaxX = Width;
			MapRegion.MaxY = Height;

			Diligent::TextureSubResData SubresData;
			SubresData.Stride = size_t{ Width } * 4u;
			SubresData.pData = PixelOutputData;
			UInt32 MipLevel = 0;
			UInt32 ArraySlice = 0;

			Context->UpdateTexture(VideoOutput->GetTexture(), MipLevel, ArraySlice, MapRegion, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
	}

	void NesPPU::CPUWrite(UInt16 Address, UInt8 Data)
	{
		switch (Address)
		{
			case 0x000: 
			{
				Controller.Register = Data;
				TempVRaddr.NameX = Controller.NameX;
				TempVRaddr.NameY = Controller.NameY;

				break;
			}
			case 0x001: Mask = Data; break;
			case 0x002: break;
			case 0x003: OAM_Address = Data; break;
			case 0x004: OAM_Data = Data; break;
			case 0x005: 
			{
				if (WriteToggle == false)
				{
					Scroll = Data & 0x07;
					TempVRaddr.CoarseX = Data >> 3;
					WriteToggle = true;
				}
				else
				{
					TempVRaddr.FineY = Data & 0x07;
					TempVRaddr.CoarseY = Data >> 3;
					WriteToggle = false;
				}

				break;
			}
			case 0x006: 
			{
				if (WriteToggle == false)
				{
					TempVRaddr.Register = (UInt16)((Data & 0x3F) << 8) | (TempVRaddr.Register & 0x00FF);
					WriteToggle = true;
				}
				else
				{
					TempVRaddr.Register = (TempVRaddr.Register & 0xFF00) | Data;
					PPUAddress = TempVRaddr;
					WriteToggle = false;
				}

				break;
			}
			case 0x007: 
			{
				PPUWrite(PPUAddress.Register, Data);

				UInt8 Increment = ((Controller.Register) ? 32 : 1);
				PPUAddress.Register += Increment;
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
				Data = (Status & 0xE0) | (PPUData & 0x1F);
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
				PPUData = PPURead(PPUAddress.Register);

				if (PPUAddress.Register >= 0x3F00)
				{
					Data = PPUData;
				}

				UInt8 Increment = ((Controller.Register) ? 32 : 1);
				PPUAddress.Register += Increment;

				break;
			}
		}

		return Data;
	}
	void NesPPU::PPUWrite(UInt16 Address, UInt8 Data)
	{
		Address &= 0x3FFF;

		if (Cartridge->PPUWrite(Address, Data))
		{

		}
		else if (Address >= 0x2000 && Address <= 0x3EFF)
		{
			Address &= 0x0FFF;

			if (Cartridge->GetMirrorMode() == Vertical)
			{
				// Vertical
				if (Address >= 0x0000 && Address <= 0x03FF)
				{
					NameTable[0][Address & 0x03FF] = Data;
				}
				else if (Address >= 0x0400 && Address <= 0x07FF)
				{
					NameTable[1][Address & 0x03FF] = Data;
				}
				else if (Address >= 0x0800 && Address <= 0x0BFF)
				{
					NameTable[0][Address & 0x03FF] = Data;
				}
				else if (Address >= 0x0C00 && Address <= 0x0FFF)
				{
					NameTable[1][Address & 0x03FF] = Data;
				}
			}
			else if (Cartridge->GetMirrorMode() == Horizontal)
			{
				if (Address >= 0x0000 && Address <= 0x03FF)
				{
					NameTable[0][Address & 0x03FF] = Data;
				}
				else if (Address >= 0x0400 && Address <= 0x07FF)
				{
					NameTable[0][Address & 0x03FF] = Data;
				}
				else if (Address >= 0x0800 && Address <= 0x0BFF)
				{
					NameTable[1][Address & 0x03FF] = Data;
				}
				else if (Address >= 0x0C00 && Address <= 0x0FFF)
				{
					NameTable[1][Address & 0x03FF] = Data;
				}
			}
		}
		else if (Address >= 0x3F00 && Address <= 0x3FFF)
		{
			Address &= 0x001F;

			if (Address == 0x0010)
			{
				Address = 0x0000;
			}
			else if (Address == 0x0014)
			{
				Address = 0x0004;
			}
			else if (Address == 0x0018)
			{
				Address = 0x0008;
			}
			else if (Address == 0x001C)
			{
				Address = 0x000C;
			}

			PaletteRAM[Address] = Data;
		}
	}
	UInt8 NesPPU::PPURead(UInt16 Address)
	{
		Address &= 0x3FFF;
		UInt8 ReturnData = 0;

		if (Cartridge->PPURead(Address, ReturnData))
		{

		}
		else if (Address >= 0x2000 && Address <= 0x3EFF)
		{
			Address &= 0x0FFF;

			if (Cartridge->GetMirrorMode() == Vertical)
			{
				// Vertical
				if (Address >= 0x0000 && Address <= 0x03FF)
				{
					ReturnData = NameTable[0][Address & 0x03FF];
				}
				else if (Address >= 0x0400 && Address <= 0x07FF)
				{
					ReturnData = NameTable[1][Address & 0x03FF];
				}
				else if (Address >= 0x0800 && Address <= 0x0BFF)
				{
					ReturnData = NameTable[0][Address & 0x03FF];
				}
				else if (Address >= 0x0C00 && Address <= 0x0FFF)
				{
					ReturnData = NameTable[1][Address & 0x03FF];
				}
			}
			else if (Cartridge->GetMirrorMode() == Horizontal)
			{
				if (Address >= 0x0000 && Address <= 0x03FF)
				{
					ReturnData = NameTable[0][Address & 0x03FF];
				}
				else if (Address >= 0x0400 && Address <= 0x07FF)
				{
					ReturnData = NameTable[0][Address & 0x03FF];
				}
				else if (Address >= 0x0800 && Address <= 0x0BFF)
				{
					ReturnData = NameTable[1][Address & 0x03FF];
				}
				else if (Address >= 0x0C00 && Address <= 0x0FFF)
				{
					ReturnData = NameTable[1][Address & 0x03FF];
				}
			}
		}
		else if (Address >= 0x3F00 && Address <= 0x3FFF)
		{
			Address &= 0x001F;

			if (Address == 0x0010)
			{
				Address = 0x0000;
			}
			else if (Address == 0x0014)
			{
				Address = 0x0004;
			}
			else if (Address == 0x0018)
			{
				Address = 0x0008;
			}
			else if (Address == 0x001C)
			{
				Address = 0x000C;
			}

			ReturnData = PaletteRAM[Address] & ((Mask & 0x01) ? 0x30 : 0x3F);
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
		ImGui::Text("Frames: %d", FrameCounter);

		ImGui::Separator();

		ImGui::Text("Controller: 0x%X", Controller.Register);
		ImGui::Indent();
		UInt8 NametableAddr = ((Controller.NameX) ? 1 : 0) + ((Controller.NameY) ? 1 : 0);
		ImGui::Text("Nametable Select: %d", NametableAddr);
		ImGui::Text("Increment Mode: %d", Controller.IncrementMode);
		ImGui::Text("Sprite Tile Select: %d", Controller.SpriteTableAddr);
		ImGui::Text("Background Tile Select: %d", Controller.BGTableAddr);
		ImGui::Text("Sprite Size: %d", Controller.SpriteSize);
		ImGui::Text("PPU Master/Slave: %d", Controller.MasterPin);
		ImGui::Text("NMI Enable: %d", Controller.NMIEnable);
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
		ImGui::Text("PPU Address: 0x%X", PPUAddress.Register);
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
			//Get Palette
			UInt8 SelectedPalette = 0;

			if (i == 0x0010)
			{
				SelectedPalette = 0;
			}
			else if (i == 0x0014)
			{
				SelectedPalette = 0x0004;
			}
			else if (i == 0x0018)
			{
				SelectedPalette = 0x0008;
			}
			else if (i == 0x001C)
			{
				SelectedPalette = 0x000C;
			}
			else
			{
				SelectedPalette = i;
			}

			ImGui::ColorEdit4("##Color", &PaletteColors[PaletteRAM[SelectedPalette]].x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoLabel);

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
		static MemoryEditor NameTableView;

		ImGui::Begin("Nametable");
		NameTableView.DrawContents(NameTable[0], 1024, 0x0000);
		ImGui::End();
	}

	void NesPPU::ShiftBGRegisters()
	{
		//Shift Our Registers.
		if (Mask & 0x08)
		{
			HighBGtileInfo <<= 1;
			LowBGtileInfo <<= 1;
			HighAttributeByte <<= 1;
			LowAttributeByte <<= 1;
		}
	}
	void NesPPU::LoadBGShiftRegisters()
	{
		LowBGtileInfo  = (LowBGtileInfo & 0xFF00) | PatternTableLow;
		HighBGtileInfo = (HighBGtileInfo & 0xFF00) | PatternTableHigh;

		LowAttributeByte  = (LowAttributeByte & 0xFF00) | ((AttributeByte & 0b01) ? 0xFF : 0x00);
		HighAttributeByte = (HighAttributeByte & 0xFF00) | ((AttributeByte & 0b10) ? 0xFF : 0x00);
	}
	void NesPPU::LoadSpriteShiftRegisters()
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