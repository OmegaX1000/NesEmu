#include "NesConsole.h"

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
		MemoryViewer.WriteFn = [](ImU8* data, size_t off, ImU8 d)
		{

		};

		//Testing Instructions
		//std::string TestPath = "F:/OmegaGamingHunters Folder/TestNES Emulator/Assets/Programs/nestest.nes";
		//std::ifstream ProgramFile(TestPath, std::ifstream::binary);
		//ProgramFile.seekg(0x10);
		//ProgramFile.read((char*)Ram + 0x8000, 0x4000);
		//ProgramFile.seekg(0x10);
		//ProgramFile.read((char*)Ram + 0xC000, 0x4000);

		//Set our interrupt Vectors.
		Ram[0xFFFC] = 0x00;
		Ram[0xFFFD] = 0x80;

		//Trigger Reset Interrupt.
		CPU.Reset();
	}

	void NesConsole::WriteRAM(UInt16 Address, UInt8 Data)
	{
		if (Address >= 0x0000 && Address <= 0xFFFF)
		{
			Ram[Address] = Data;
		}
	}
	UInt8 NesConsole::ReadRAM(UInt16 Address)
	{
		if (Address >= 0x0000 && Address <= 0xFFFF)
		{
			return Ram[Address];
		}

		return 0x00;
	}
	void NesConsole::InsertCartridge(std::string_view NewCartPath)
	{
		this->Cartridge = std::make_shared<NesCartridge>(NewCartPath);
	}

	void NesConsole::DrawRamContents(int StartAddress)
	{
		ImGui::Begin("RAM Contents");

		MemoryViewer.DrawContents(Ram, 65536, StartAddress);
		/*
		for (int x = 0; x < Rows; x++)
		{
			std::string Line = fmt::format("${:#04X}:", Ram[StartAddress]);

			for (int col = 0; col < Columns; col++)
			{
				Line += fmt::format(" {:#04X}", Ram[StartAddress]);
				StartAddress++;
			}

			ImGui::Text(Line.c_str());
		}*/

		ImGui::End();
	}

	NesCPU* NesConsole::GetCPU()
	{
		return &CPU;
	}
}