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

				//Mirror Mode
				UInt8 MirrorTemp = Header.MapperFlags1 & 0x01;
				UInt8 IsFourScreen = Header.MapperFlags1 & 0x08;
				HardwareMirrorMode = (Mirror)(MirrorTemp + IsFourScreen);

				//Trainer Area.
				if (Header.MapperFlags1 & 0x04)
				{
					ProgramFile.seekg(512, std::ios::cur);
				}

				//Program ROM Area.
				UInt8 PRG_Chunks_MSB = Header.RomSizeMSB & 0xF;
				UInt8 PRG_Chunks_LSB = Header.ProgramRomSizeLSB;

				//Exponent-multiplier notation.
				if (PRG_Chunks_MSB == 0xF)
				{

				}
				else
				{
					ProgramBanks = (PRG_Chunks_MSB << 8) | PRG_Chunks_LSB; //From: 0x000 - 0xEFF, or 0 - 3839. 3839 * 16KB = 61424KB, 61.42MB.
				}

				ProgramMemory.resize(ProgramBanks * 16384);
				ProgramFile.read((char*)ProgramMemory.data(), ProgramMemory.size());

				//Character ROM Area
				UInt8 CHR_Chunks_MSB = (Header.RomSizeMSB >> 4) & 0xF;
				UInt8 CHR_Chunks_LSB = Header.CharacterRomSizeLSB;

				//Exponent-multiplier notation.
				if (CHR_Chunks_MSB == 0xF)
				{

				}
				else
				{
					CharacterBanks = (CHR_Chunks_MSB << 8) | CHR_Chunks_LSB; //From: 0x000 - 0xEFF, or 0 - 3839. 3839 * 8KB = 30712KB, 30.71MB.
				}

				CharacterMemory.resize(CharacterBanks * 8192);
				ProgramFile.read((char*)CharacterMemory.data(), CharacterMemory.size());

				//Create our Mapper.
				switch (MapperID)
				{
					case 0:
					{
						Mapper = std::make_shared<NesMapper0>(ProgramBanks, CharacterBanks);
						break;
					}
				}
			}
		}
	}

	bool NesCartridge::CPUWrite(UInt16 Address, UInt8 Data)
	{
		UInt32 WriteAddress = 0;

		if (Mapper->CPUWriteRam(Address, WriteAddress))
		{
			ProgramMemory[WriteAddress] = Data;
			return true;
		}

		return false;
	}
	bool NesCartridge::CPURead(UInt16 Address, UInt8& ReturnData)
	{
		UInt32 ReadAddress = 0;

		if (Mapper->CPUReadRam(Address, ReadAddress))
		{
			ReturnData = ProgramMemory[ReadAddress];
			return true;
		}

		return false;
	}
	bool NesCartridge::PPUWrite(UInt16 Address, UInt8 Data)
	{
		UInt32 WriteAddress = 0;

		if (Mapper->PPUReadRam(Address, WriteAddress))
		{
			CharacterMemory[WriteAddress] = Data;
			return true;
		}

		return false;
	}
	bool NesCartridge::PPURead(UInt16 Address, UInt8& ReturnData)
	{
		UInt32 ReadAddress = 0;

		if (Mapper->PPUReadRam(Address, ReadAddress))
		{
			ReturnData = CharacterMemory[ReadAddress];
			return true;
		}

		return false;
	}

	Mirror NesCartridge::GetMirrorMode()
	{
		return HardwareMirrorMode;
	}
}