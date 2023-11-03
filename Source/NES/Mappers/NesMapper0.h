#pragma once
#include "NesMapper.h"

namespace NesEmulator
{
	class NesMapper0 : public NesMapper
	{
		private:

		public:

			//Constructor
			NesMapper0(UInt8 ProgramChunks, UInt8 CharacterChunks);

			//CPU Read/Write Functions
			bool CPUReadRam(UInt16 Address, UInt32& MappedAddress) override;
			bool CPUWriteRam(UInt16 Address, UInt32& MappedAddress) override;

			//PPU Read/Write Functions
			bool PPUReadRam(UInt16 Address, UInt32& MappedAddress) override;
			bool PPUWriteRam(UInt16 Address, UInt32& MappedAddress) override;
	};
}