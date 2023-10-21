#include "NesCartridge.h"

namespace NesEmulator
{
	NesCartridge::NesCartridge(std::string_view ProgramPath)
	{
		std::ifstream ProgramFile(ProgramPath.data(), std::ifstream::binary);
		FileHeader Header;
		bool iNesFormat = false;
		bool NesVersion2 = false;

		if (ProgramFile.is_open())
		{
			//Determine if the file is a iNes file and then see if it's version 1.0 or 2.0
			char Id[4];
			ProgramFile.read(Id, 4);

			if (Id[0] == 'N' && Id[1] == 'E' && Id[2] == 'S' && Id[3] == 0x1A)
			{
				iNesFormat = true;
			}

			if (iNesFormat == true)
			{
				ProgramFile.seekg(7);
				char Byte;
				ProgramFile.read(&Byte, 1);

				if ((Byte & 0x0C) == 0x08)
				{
					NesVersion2 = true;
				}
			}

			ProgramFile.seekg(0, std::ios::beg);

			//Read the file
			if (iNesFormat == true)
			{
				//Read File header
				ProgramFile.read((char*)&Header, sizeof(Header));

				//Trainer Area.
				if (Header.MapperFlags1 & 0x04)
				{
					ProgramFile.seekg(512, std::ios::cur);
				}

				//Program ROM Area.
				UInt8 PRG_Chunks_MSB = Header.RomSizeMSB & 0xF;
				UInt8 PRG_Chunks_LSB = Header.ProgramRomSizeLSB;
				UInt16 ProgramChunks = 0; // (PRG_Chunks_MSB << 8) | PRG_Chunks_LSB; //From: 0x000 - 0xEFF, or 0 - 3839. 3839 * 16KB = 61424KB, 61.42MB.

				//Exponent-multiplier notation.
				if (PRG_Chunks_MSB == 0xF)
				{

				}
				else
				{
					ProgramChunks = (PRG_Chunks_MSB << 8) | PRG_Chunks_LSB; //From: 0x000 - 0xEFF, or 0 - 3839. 3839 * 16KB = 61424KB, 61.42MB.
				}

				ProgramMemory.resize(ProgramChunks * 16384);
				ProgramFile.read((char*)ProgramMemory.data(), ProgramMemory.size());

				//Character ROM Area
				UInt8 CHR_Chunks_MSB = (Header.RomSizeMSB >> 4) & 0xF;
				UInt8 CHR_Chunks_LSB = Header.CharacterRomSizeLSB;
				UInt16 CharacterChunks = 0; // (CHR_Chunks_MSB << 8) | CHR_Chunks_LSB; //From: 0x000 - 0xEFF, or 0 - 3839. 3839 * 8KB = 30712KB, 30.71MB.

				//Exponent-multiplier notation.
				if (CHR_Chunks_MSB == 0xF)
				{

				}
				else
				{
					CharacterChunks = (CHR_Chunks_MSB << 8) | CHR_Chunks_LSB; //From: 0x000 - 0xEFF, or 0 - 3839. 3839 * 8KB = 30712KB, 30.71MB.
				}

				CharacterMemory.resize(CharacterChunks * 8192);
				ProgramFile.read((char*)CharacterMemory.data(), CharacterMemory.size());

				//Create our Mapper.
				switch (MapperID)
				{
					case 0:
					{
						break;
					}
				}
			}
		}
	}
}