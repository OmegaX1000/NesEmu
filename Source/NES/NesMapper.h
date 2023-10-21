#pragma once
#include "Definitions.h"

namespace NesEmulator
{
	//The mapper is responsible for mapping the Memory to the CPU and PPU Memory Map. 
	class NesMapper
	{
		private:

		public:

			//CPU Read/Write Functions
			virtual void CPUReadRam() = 0;
			virtual void CPUWriteRam() = 0;

			//PPU Read/Write Functions
			virtual void PPUReadRam() = 0;
			virtual void PPUWriteRam() = 0;
	};
}