#pragma once
#include "Definitions.h"
#include "imgui.h"

#include <string>

namespace NesEmulator
{
	//Forward Delcelation
	class NesConsole;

	//The NES MOS-6502 CPU
	class NesCPU
	{
		private:

			//The console this device is a part of.
			NesConsole* System = nullptr;

			enum AddressingMode; //Forward Delcelation.

			//Our Data structure for getting and executing instructions.
			struct Instruction
			{
				char Name[4];						   //The Opcode Name. (ex. LDX, ADC, INC, etc)
				UInt8 Opcode = 0;					   //The instruction to execute.
				AddressingMode Mode;				   //The addressing mode of this instruction.
				UInt8 Size = 0;						   //The size of the instruction. (either 1, 2, or 3)
				UInt8 Cycles = 0;					   //The cycles needed to complete this instruction.
				void (*OpFunc)(NesCPU* CPU) = nullptr; //The function its self.
			};

			//Lookup Table for our instructions.
			Instruction OpTable[256];

			//Our Clock Signal.
			UInt64 CycleCounter = 0;

		public:

			//Constructor & Destructor
			NesCPU();
			~NesCPU();

			//Helper Variables/Functions for executing instructions.
			void Fetch();
			Instruction* CurrOp = nullptr;
			bool Jam = false;
			UInt8 CycleRemain = 0;

			//CPU Registers
			UInt8 Accumulator = 0;
			UInt8 Index_X = 0;
			UInt8 Index_Y = 0;
			UInt16 ProgramCounter = 0;
			UInt8 StackPointer = 0;
			UInt8 StatusFlags = 0;

			//Status Flags for the Status Register/Processor State.
			enum StatusFlag
			{
				Carry	  =	(1 << 0), //Carry Bit
				Zero	  =	(1 << 1), //Zero
				Interrupt = (1 << 2), //Disable Interrupts
				Decimal   =	(1 << 3), //Decimal Mode
				Break     =	(1 << 4), //Break
				Unsed     =	(1 << 5), //Unused
				Overflow  =	(1 << 6), //Overflow
				Negative  =	(1 << 7)  //Negative
			};

			//Addressing Mode
			enum AddressingMode
			{
				AccumulatorAddressing = 0,
				Absolute			  = 1,
				AbsoluteIndexedX	  = 2,
				AbsoluteIndexedY	  = 3,
				Immediate			  = 4,
				Implied				  = 5,
				Indirect			  = 6,
				IndirectIndexedX	  = 7,
				IndirectIndexedY	  = 8,
				Relative			  = 9,
				ZeroPage			  = 10,
				ZeroPageIndexX		  = 11,
				ZeroPageIndexY		  = 12
			};

			std::string GetAddressMode(AddressingMode Mode);

			//Connect this device to the rest of the console so you can read/write to ram and other stuff.
			void Connect(NesConsole* Console);
			void WriteRAM(UInt16 Address, UInt8 Data);
			UInt8 ReadRAM(UInt16 Address);
			
			//Signal/Pin Functions
			void Clock();
			void Reset();
			void NMI();
			void IRQ();

			//Flags
			UInt8 GetFlag(StatusFlag Flag);
			void  SetFlag(StatusFlag Flag, bool v);

			void DrawRegisters();
	};
}