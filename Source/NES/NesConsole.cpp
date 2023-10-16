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
		UInt8 Test[] = 
		{ 
			0xE6, 0x05,		  //INC $05
			0xE6, 0x05,		  //INC $05
			0xEE, 0x80, 0x50, //INC $5080
			0xEE, 0x80, 0x50, //INC $5080
			0xC6, 0x05,		  //DEC $05
			0xCE, 0x80, 0x50, //DEC $5080
			0xE8,			  //INX
			0xE8,			  //INX
			0xF6, 0x06,		  //INC $06, X
			0xF6, 0x06,		  //INC $06, X
			0xFE, 0x90, 0x50, //INC $5090, X
			0xFE, 0x90, 0x50, //INC $5090, X
			0xD6, 0x06,		  //DEC $06, X
			0xDE, 0x90, 0x50, //DEC $5090, X
			0xCA,			  //DEX
			0xCA			  //DEX
		};
		
		for (UInt16 i = 0; i < (sizeof(Test) / sizeof(UInt8)); i++)
		{
			Ram[0x8000 + i] = Test[i];
		}

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