#include "NesConsole.h"
#include "Application.h"

#include "optick.h"

namespace NesEmulator
{
	NesConsole::NesConsole()
	{
		OPTICK_EVENT();
		CPU.Connect(this);

		//Clear the RAM.
		for (auto& i : Ram)
		{
			i = 0x00;
		}

		//Empty Write function
		//MemoryViewer.WriteFn = [](ImU8* data, size_t off, ImU8 d)
		//{

		//};
	}

	void NesConsole::Clock()
	{
		PPU.Clock();

		if (SystemClockCounter % 3 == 0)
		{
			CPU.Clock();
		}

		if (CPU.Jam == false && PPU.NMI == true)
		{
			CPU.NMI();
			PPU.NMI = false;
		}

		SystemClockCounter++;
	}
	void NesConsole::DrawVideo()
	{
		OPTICK_EVENT();
		ImGuiWindowFlags VideoFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		VideoFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		VideoFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImVec2 ImageSize = ImVec2(512, 480);
		ImVec2 WindowSize = ImGui::GetWindowViewport()->Size;
		ImVec2 ImagePos = ImVec2((WindowSize.x - ImageSize.x) * 0.5f, (WindowSize.y - ImageSize.y) * 0.5f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PopStyleVar();
		ImGui::Begin("Video", NULL, VideoFlags);
		ImGui::PopStyleVar(2);

		ImGui::SetCursorPos(ImagePos);
		ImGui::Image((void*)VideoOutput, ImageSize);
		ImGui::End();
	}
	void NesConsole::InsertCartridge(std::string_view NewCartPath)
	{
		OPTICK_EVENT();
		this->Cartridge = std::make_shared<NesCartridge>(NewCartPath);
		PPU.ConnectCartridge(this->Cartridge);
		CPU.Reset();
		PPU.Reset();

		PPU.UpdateDebugPatternTable(Application::Get().GraphicsSystem.GetDevice());
	}
	void NesConsole::UpdateVideoOutput(Diligent::IRenderDevice* RenderDevice, Diligent::IDeviceContext* Context)
	{
		OPTICK_EVENT();

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

			Diligent::TextureSubResData TextureData[] = { {PPU.PixelOutputData, 4 * UInt64{VideoOutputDesc.Width}} };
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
			SubresData.Stride = size_t{ Width } *4u;
			SubresData.pData = PPU.PixelOutputData;
			UInt32 MipLevel = 0;
			UInt32 ArraySlice = 0;

			Context->UpdateTexture(VideoOutput->GetTexture(), MipLevel, ArraySlice, MapRegion, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
	}
	void NesConsole::PollControllers()
	{
		OPTICK_EVENT();
		
		if (ControllerOne.GetWriteBuffer()->ControlPort == 1)
		{
			switch (ControllerOne.GetType())
			{
				case NesController::ControllerType::StandardController:
				{
					ControllerOne.StandardControllerButtons = 0x00;
					ControllerOne.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_Z) ? 0x01 : 0x00;
					ControllerOne.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_X) ? 0x02 : 0x00;
					ControllerOne.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_A) ? 0x04 : 0x00;
					ControllerOne.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_S) ? 0x08 : 0x00;
					ControllerOne.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_UpArrow) ? 0x10 : 0x00;
					ControllerOne.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_DownArrow) ? 0x20 : 0x00;
					ControllerOne.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_LeftArrow) ? 0x40 : 0x00;
					ControllerOne.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_RightArrow) ? 0x80 : 0x00;
					break;
				}
				default: break;
			}
		}

		if (ControllerTwo.GetWriteBuffer()->ControlPort == 1)
		{
			switch (ControllerTwo.GetType())
			{
				case NesController::ControllerType::StandardController:
				{
					ControllerTwo.StandardControllerButtons = 0x00;
					ControllerTwo.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_Z) ? 0x01 : 0x00;
					ControllerTwo.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_X) ? 0x02 : 0x00;
					ControllerTwo.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_A) ? 0x04 : 0x00;
					ControllerTwo.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_S) ? 0x08 : 0x00;
					ControllerTwo.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_UpArrow) ? 0x10 : 0x00;
					ControllerTwo.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_DownArrow) ? 0x20 : 0x00;
					ControllerTwo.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_LeftArrow) ? 0x40 : 0x00;
					ControllerTwo.StandardControllerButtons |= ImGui::IsKeyDown(ImGuiKey_RightArrow) ? 0x80 : 0x00;
					break;
				}
				default: break;
			}
		}
	}
	void NesConsole::DrawCPUMemory()
	{
		static MemoryEditor MemoryViewer;
		MemoryViewer.DrawWindow("CPU RAM", Ram, sizeof(Ram));
	}

	void NesConsole::CPUWrite(UInt16 Address, UInt8 Data)
	{
		if (Cartridge->CPUWrite(Address, Data))
		{

		}
		else if (Address >= 0x0000 && Address <= 0x1FFF)
		{
			Ram[Address & 0x07FF] = Data;
		}
		else if (Address >= 0x2000 && Address <= 0x3FFF)
		{
			PPU.CPUWrite(Address & 0x07, Data);
		}
		else if (Address >= 0x4000 && Address <= 0x4017)
		{
			switch (Address)
			{
				case 0x4016:
				{
					ControllerOne.ControllerWrite(Data);
					ControllerTwo.ControllerWrite(Data);
					PollControllers();
					break;
				}
			}
		}
	}
	UInt8 NesConsole::CPURead(UInt16 Address)
	{
		UInt8 ReturnData = 0;

		if (Cartridge->CPURead(Address, ReturnData))
		{

		}
		if (Address >= 0x0000 && Address <= 0x1FFF)
		{
			ReturnData = Ram[Address & 0x07FF];
		}
		else if (Address >= 0x2000 && Address <= 0x3FFF)
		{
			ReturnData = PPU.CPURead(Address & 0x07);
		}
		else if (Address >= 0x4000 && Address <= 0x4017)
		{
			switch (Address)
			{
				case 0x4016:
				{
					ControllerOne.ControllerRead(ReturnData);
					break;
				}
				case 0x4017:
				{
					ControllerTwo.ControllerRead(ReturnData);
					break;
				}
			}
		}

		return ReturnData;
	}

	NesCPU* NesConsole::GetCPU()
	{
		return &CPU;
	}
	NesPPU* NesConsole::GetPPU()
	{
		return &PPU;
	}
	NesController* NesConsole::GetControllerOne()
	{
		return &ControllerOne;
	}
	NesController* NesConsole::GetControllerTwo()
	{
		return &ControllerTwo;
	}
}