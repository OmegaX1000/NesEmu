#include "NesConsole.h"
#include "Application.h"

namespace NesEmulator
{
	NesConsole::NesConsole()
	{
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

	void NesConsole::Clock(Diligent::IRenderDevice* RenderDevice, Diligent::IDeviceContext* Context)
	{
		PPU.Clock(RenderDevice, Context);

		if (SystemClockCounter % 3 == 0)
		{
			CPU.Clock();
		}

		SystemClockCounter++;

		if (CPU.Jam == false && PPU.NMI == true)
		{
			CPU.NMI();
			PPU.NMI = false;
		}
	}
	void NesConsole::InsertCartridge(std::string_view NewCartPath)
	{
		this->Cartridge = std::make_shared<NesCartridge>(NewCartPath);
		PPU.ConnectCartridge(this->Cartridge);
		CPU.Reset();
		PPU.Reset();

		PPU.UpdateDebugPatternTable(Application::Get().GraphicsSystem.GetDevice());
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
			
		}

		return ReturnData;
	}

	void NesConsole::DrawRamContents(int StartAddress)
	{
		
	}

	NesCPU* NesConsole::GetCPU()
	{
		return &CPU;
	}
	NesPPU* NesConsole::GetPPU()
	{
		return &PPU;
	}
}