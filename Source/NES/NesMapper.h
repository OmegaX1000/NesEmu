#pragma once
#include "Definitions.h"

namespace NesEmulator
{
	enum Mirror
	{
		Horizontal,
		Vertical,
		FourScreen
	};

	//The mapper is responsible for mapping the Memory to the CPU and PPU Memory Map. 
	class NesMapper
	{
		protected:

			UInt8 ProgramBanks = 0;
			UInt8 CharacterBanks = 0;

		public:

			//Constructor
			NesMapper(UInt8 ProgramChunks, UInt8 CharacterChunks);

			//CPU Read/Write Functions
			virtual bool CPUReadRam(UInt16 Address, UInt32& MappedAddress) = 0;
			virtual bool CPUWriteRam(UInt16 Address, UInt32& MappedAddress) = 0;

			//PPU Read/Write Functions
			virtual bool PPUReadRam(UInt16 Address, UInt32& MappedAddress) = 0;
			virtual bool PPUWriteRam(UInt16 Address, UInt32& MappedAddress) = 0;
	};
}