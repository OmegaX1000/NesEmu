#pragma once
#include "Definitions.h"

#include "Mappers/NesMapper0.h"

#include <vector>
#include <string>
#include <fstream>

namespace NesEmulator
{
	//Holds all of our Game Data.
	class NesCartridge
	{
		private:

			//Memory that holds code, data and sprites.
			std::vector<UInt8> ProgramMemory;
			std::vector<UInt8> CharacterMemory;

			UInt8 ProgramBanks = 0;
			UInt8 CharacterBanks = 0;

			//Mapper
			UInt16 MapperID = 0;
			std::shared_ptr<NesMapper> Mapper;

			//16 Byte Header for .nes Files.
			struct FileHeader
			{
				char NesId[4];			   //Identification String for the File.
				UInt8 ProgramRomSizeLSB;   //Size of the Program ROM (LSB, Least significant Byte?)
				UInt8 CharacterRomSizeLSB; //Size of the Character ROM (LSB, Least significant Byte?)
				UInt8 MapperFlags1;		   //Mapper number (D0 to D3) and other properties stored in the other half of the byte.
				UInt8 MapperFlags2;		   //Mapper number (D4 to D7) and other properties stored in the other half of the byte.
				UInt8 MapperVariants;	   //Mapper number (D8 to D11) and also stores the variant of the mapper.
				UInt8 RomSizeMSB;		   //The size of the Program and Character ROM (MSB, Most significant Byte?)
				UInt8 RAM_EEPROM_Size;	   //The size of Program RAM and EEPROM.
				UInt8 CharacterRamSize;    //The size of the Character RAM. (if present)
				UInt8 SystemTiming;		   //The timing settings between the CPU and PPU.
				UInt8 HardwareInfo;		   //System Type
				UInt8 MiscRoms;			   //Number of miscellaneous ROMs present.
				UInt8 DefaultExpDevice;    //Default Expansion Device.
			};

		public:

			//Constructor, Basically Inserts a New Game.
			NesCartridge(std::string_view ProgramPath);

			//RAM Functions
			bool CPUWrite(UInt16 Address, UInt8 Data);		 //CPU WriteRam Function.
			bool CPURead(UInt16 Address, UInt8 &ReturnData); //CPU ReadRam Function.
			bool PPUWrite(UInt16 Address, UInt8 Data);		 //PPU WriteRam Function.
			bool PPURead(UInt16 Address, UInt8 &ReturnData); //PPU ReadRam Function.
	};
}