#include "NesMapper0.h"

namespace NesEmulator
{
	NesMapper0::NesMapper0(UInt8 ProgramChunks, UInt8 CharacterChunks) : NesMapper(ProgramChunks, CharacterChunks)
	{

	}

	bool NesMapper0::CPUReadRam(UInt16 Address, UInt32& MappedAddress)
	{
		if (Address >= 0x8000 && Address <= 0xFFFF)
		{
			MappedAddress = Address & (ProgramBanks > 1 ? 0x7FFF : 0x3FFF);
			return true;
		}

		return false;
	}
	bool NesMapper0::CPUWriteRam(UInt16 Address, UInt32& MappedAddress)
	{
		if (Address >= 0x8000 && Address <= 0xFFFF)
		{
			MappedAddress = Address & (ProgramBanks > 1 ? 0x7FFF : 0x3FFF);
			return true;
		}

		return false;
	}

	bool NesMapper0::PPUReadRam(UInt16 Address, UInt32& MappedAddress)
	{
		if (Address >= 0x000 && Address <= 0x1FFF)
		{
			MappedAddress = Address;
			return true;
		}

		return false;
	}
	bool NesMapper0::PPUWriteRam(UInt16 Address, UInt32& MappedAddress)
	{
		if (Address >= 0x000 && Address <= 0x1FFF)
		{
			MappedAddress = Address;
			return true;
		}

		return false;
	}
}