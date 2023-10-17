#include "NesCPU.h"
#include "NesConsole.h"

namespace NesEmulator
{
	NesCPU::NesCPU()
	{
		//Define our instructions for our CPU. (note: probably wouldn't do this ...)
		OpTable[0x00] = { { 'B', 'R', 'K', '\0'}, 0x00, Implied, 1, 7, [](NesCPU* CPU)
			{
				CPU->ProgramCounter++;

				UInt8 temp = CPU->ReadRAM(CPU->ProgramCounter);
				CPU->ProgramCounter++;

				CPU->SetFlag(Interrupt, true);
				CPU->WriteRAM(0x0100 + CPU->StackPointer, (CPU->ProgramCounter >> 8) & 0x00FF);
				CPU->StackPointer--;
				CPU->WriteRAM(0x0100 + CPU->StackPointer, CPU->ProgramCounter & 0x00FF);
				CPU->StackPointer--;

				CPU->SetFlag(Break, true);
				CPU->WriteRAM(0x0100 + CPU->StackPointer, CPU->StatusFlags);
				CPU->StackPointer--;
				CPU->SetFlag(Break, false);

				CPU->ProgramCounter = (uint16_t)CPU->ReadRAM(0xFFFF) | ((uint16_t)CPU->ReadRAM(0xFFFE) << 8);
			}};

		//Load and Store Instructions
		{
			//Load Accumulator with Memory (LDA)
			{
				OpTable[0xA9] = { { 'L', 'D', 'A', '\0'}, 0xA9, Immediate, 2, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 Value = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->Accumulator = Value;

						CPU->SetFlag(Zero, Value == 0);
						CPU->SetFlag(Negative, Value & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xAD] = { { 'L', 'D', 'A', '\0'}, 0xAD, Absolute, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Accumulator = MemoryValue;

						CPU->SetFlag(Zero, CPU->Accumulator == 0);
						CPU->SetFlag(Negative, CPU->Accumulator & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xBD] = { { 'L', 'D', 'A', '\0'}, 0xBD, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Accumulator = MemoryValue;

						if ((MemoryAddress & 0xFF00) != (HighAddresByte << 8))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Zero, CPU->Accumulator == 0);
						CPU->SetFlag(Negative, CPU->Accumulator & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xB9] = { { 'L', 'D', 'A', '\0'}, 0xB9, AbsoluteIndexedY, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Accumulator = MemoryValue;

						if ((MemoryAddress & 0xFF00) != (HighAddresByte << 8))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Zero, CPU->Accumulator == 0);
						CPU->SetFlag(Negative, CPU->Accumulator & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xA5] = { { 'L', 'D', 'A', '\0'}, 0xA5, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Accumulator = MemoryValue;

						CPU->SetFlag(Zero, CPU->Accumulator == 0);
						CPU->SetFlag(Negative, CPU->Accumulator & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xB5] = { { 'L', 'D', 'A', '\0'}, 0xB5, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Accumulator = MemoryValue;

						CPU->SetFlag(Zero, CPU->Accumulator == 0);
						CPU->SetFlag(Negative, CPU->Accumulator & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xA1] = { { 'L', 'D', 'A', '\0'}, 0xA1, IndirectIndexedX, 2, 6,[](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;

						UInt8 Value = CPU->ReadRAM(FinalMemoryAddress);
						CPU->Accumulator = Value;

						CPU->SetFlag(Zero, CPU->Accumulator == 0);
						CPU->SetFlag(Negative, CPU->Accumulator & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xB1] = { { 'L', 'D', 'A', '\0'}, 0xB1, IndirectIndexedY, 2, 5,[](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;

						UInt8 Value = CPU->ReadRAM(FinalMemoryAddress);
						CPU->Accumulator = Value;

						if ((MemoryAddress & 0xFF00) != (SecondAddressHigh << 8))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Zero, CPU->Accumulator == 0);
						CPU->SetFlag(Negative, CPU->Accumulator & 0x80);
						CPU->ProgramCounter++;
					} };
			}

			//Load Index X with Memory (LDX)	
			{
				OpTable[0xA2] = { { 'L', 'D', 'X', '\0'}, 0xA2, Immediate, 2, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->Index_X = MemoryValue;

						CPU->SetFlag(Zero, CPU->Index_X == 0);
						CPU->SetFlag(Negative, CPU->Index_X & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xA6] = { { 'L', 'D', 'X', '\0'}, 0xA6, ZeroPage, 2, 3,[](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Index_X = MemoryValue;

						CPU->SetFlag(Zero, CPU->Index_X == 0);
						CPU->SetFlag(Negative, CPU->Index_X & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xB6] = { { 'L', 'D', 'X', '\0'}, 0xB6, ZeroPageIndexY, 2, 4,[](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Index_X = MemoryValue;

						CPU->SetFlag(Zero, CPU->Index_X == 0);
						CPU->SetFlag(Negative, CPU->Index_X & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xAE] = { { 'L', 'D', 'X', '\0'}, 0xAE, Absolute, 3, 4,[](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Index_X = MemoryValue;

CPU->SetFlag(Zero, CPU->Index_X == 0);
CPU->SetFlag(Negative, CPU->Index_X & 0x80);
CPU->ProgramCounter++;
					} };
				OpTable[0xBE] = { { 'L', 'D', 'X', '\0'}, 0xBE, AbsoluteIndexedY, 3, 4,[](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Index_X = MemoryValue;

						if ((MemoryAddress & 0xFF00) != (HighAddresByte << 8))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Zero, CPU->Index_X == 0);
						CPU->SetFlag(Negative, CPU->Index_X & 0x80);
						CPU->ProgramCounter++;
					} };
			}

			//Load Index Y with Memory (LDX)	
			{
				OpTable[0xA0] = { { 'L', 'D', 'Y', '\0'}, 0xA0, Immediate, 2, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->Index_Y = MemoryValue;

						CPU->SetFlag(Zero, CPU->Index_Y == 0);
						CPU->SetFlag(Negative, CPU->Index_Y & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xA4] = { { 'L', 'D', 'Y', '\0'}, 0xA4, ZeroPage, 2, 3,[](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Index_Y = MemoryValue;

						CPU->SetFlag(Zero, CPU->Index_Y == 0);
						CPU->SetFlag(Negative, CPU->Index_Y & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xB4] = { { 'L', 'D', 'Y', '\0'}, 0xB4, ZeroPageIndexX, 2, 4,[](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Index_Y = MemoryValue;

						CPU->SetFlag(Zero, CPU->Index_Y == 0);
						CPU->SetFlag(Negative, CPU->Index_Y & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xAC] = { { 'L', 'D', 'Y', '\0'}, 0xAC, Absolute, 3, 4,[](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Index_Y = MemoryValue;

						CPU->SetFlag(Zero, CPU->Index_Y == 0);
						CPU->SetFlag(Negative, CPU->Index_Y & 0x80);
						CPU->ProgramCounter++;
					} };
				OpTable[0xBC] = { { 'L', 'D', 'Y', '\0'}, 0xBC, AbsoluteIndexedX, 3, 4,[](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->Index_Y = MemoryValue;

						if ((MemoryAddress & 0xFF00) != (HighAddresByte << 8))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Zero, CPU->Index_Y == 0);
						CPU->SetFlag(Negative, CPU->Index_Y & 0x80);
						CPU->ProgramCounter++;
					} };
			}

			//Store Accumulator in Memory (STA)
			{
				OpTable[0x8D] = { { 'S', 'T', 'A', '\0'}, 0x8D, Absolute, 3, 4, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						CPU->WriteRAM(MemoryAddress, CPU->Accumulator);
					} };
				OpTable[0x9D] = { { 'S', 'T', 'A', '\0'}, 0x9D, AbsoluteIndexedX, 3, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						CPU->WriteRAM(MemoryAddress, CPU->Accumulator);
					} };
				OpTable[0x99] = { { 'S', 'T', 'A', '\0'}, 0x99, AbsoluteIndexedY, 3, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						CPU->WriteRAM(MemoryAddress, CPU->Accumulator);
					} };
				OpTable[0x85] = { { 'S', 'T', 'A', '\0'}, 0x85, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						CPU->WriteRAM(AddresByte, CPU->Accumulator);
					} };
				OpTable[0x95] = { { 'S', 'T', 'A', '\0'}, 0x95, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;

						CPU->WriteRAM(AddresByte, CPU->Accumulator);
					} };
				OpTable[0x81] = { { 'S', 'T', 'A', '\0'}, 0x81, IndirectIndexedX, 2, 6, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						CPU->WriteRAM(FinalMemoryAddress, CPU->Accumulator);
					} };
				OpTable[0x91] = { { 'S', 'T', 'A', '\0'}, 0x91, IndirectIndexedY, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						CPU->WriteRAM(FinalMemoryAddress, CPU->Accumulator);
					} };
			}

			//Store Index X In Memory (STX)
			{
				OpTable[0x8E] = { { 'S', 'T', 'X', '\0'}, 0x8E, Absolute, 3, 4, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						CPU->WriteRAM(MemoryAddress, CPU->Index_X);
					} };
				OpTable[0x86] = { { 'S', 'T', 'X', '\0'}, 0x86, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						CPU->WriteRAM(AddresByte, CPU->Index_X);
					} };
				OpTable[0x96] = { { 'S', 'T', 'X', '\0'}, 0x96, ZeroPageIndexY, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;

						CPU->WriteRAM(AddresByte, CPU->Index_X);
					} };
			}

			//Store Index Y In Memory (STY)
			{
				OpTable[0x8C] = { { 'S', 'T', 'Y', '\0'}, 0x8C, Absolute, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						CPU->WriteRAM(MemoryAddress, CPU->Index_Y);
					} };
				OpTable[0x84] = { { 'S', 'T', 'Y', '\0'}, 0x84, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						CPU->WriteRAM(AddresByte, CPU->Index_Y);
					} };
				OpTable[0x94] = { { 'S', 'T', 'Y', '\0'}, 0x94, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;

						CPU->WriteRAM(AddresByte, CPU->Index_Y);
					} };
			}
		}

		//Transfer Instructions
		{
			OpTable[0xAA] = { {'T', 'A', 'X', '\0'}, 0xAA, Implied, 1, 2, [](NesCPU* CPU) 
				{
					CPU->Index_X = CPU->Accumulator;

					CPU->SetFlag(Zero, (CPU->Index_X == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (CPU->Index_X & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} }; //Transfer Accumulator to X (TAX)
			OpTable[0xA8] = { {'T', 'A', 'Y', '\0'}, 0xA8, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->Index_Y = CPU->Accumulator;

					CPU->SetFlag(Zero, (CPU->Index_Y == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (CPU->Index_Y & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} }; //Transfer Accumulator to Y (TAY)
			OpTable[0xBA] = { {'T', 'S', 'X', '\0'}, 0xBA, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->StackPointer++;
					CPU->Index_X = CPU->ReadRAM(0x0100 + CPU->StackPointer);

					CPU->SetFlag(Zero, (CPU->Index_X == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (CPU->Index_X & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} }; //Transfer StackPointer to X (TSX)
			OpTable[0x8A] = { {'T', 'X', 'A', '\0'}, 0x8A, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->Accumulator = CPU->Index_X;

					CPU->SetFlag(Zero, (CPU->Accumulator == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (CPU->Accumulator & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} }; //Transfer Index X To Accumulator (TXA)
			OpTable[0x9A] = { {'T', 'X', 'S', '\0'}, 0x9A, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->WriteRAM(0x0100 + CPU->StackPointer, CPU->Index_X);
					CPU->StackPointer--;
					CPU->ProgramCounter++;
				} }; //Transfer Index X To StackPointer (TXS)
			OpTable[0x98] = { {'T', 'Y', 'S', '\0'}, 0x98, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->Accumulator = CPU->Index_Y;

					CPU->SetFlag(Zero, (CPU->Accumulator == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (CPU->Accumulator & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} }; //Transfer Index Y To Accumulator (TYS)
		}

		//Stack Instructions
		{
			OpTable[0x48] = { {'P', 'H', 'A', '\0'}, 0x48, Implied, 1, 3, [](NesCPU* CPU)
				{
					CPU->WriteRAM(0x0100 + CPU->StackPointer, CPU->Accumulator);
					CPU->StackPointer--;
					CPU->ProgramCounter++;
				}}; //Stack Push (Accumulator)
			OpTable[0x08] = { {'P', 'H', 'P', '\0'}, 0x08, Implied, 1, 3, [](NesCPU* CPU)
				{
					CPU->SetFlag(Break, 1);
					CPU->SetFlag(Unsed, 1);
					CPU->WriteRAM(0x0100 + CPU->StackPointer, CPU->StatusFlags);
					CPU->StackPointer--;
					CPU->ProgramCounter++;
				}}; //Stack Push (Status)
			OpTable[0x68] = { {'P', 'L', 'A', '\0'}, 0x68, Implied, 1, 4, [](NesCPU* CPU)
				{
					CPU->StackPointer++;
					CPU->Accumulator = CPU->ReadRAM(0x0100 + CPU->StackPointer);

					CPU->SetFlag(Zero, (CPU->Accumulator == 0) ? 1 : 0); //Set if Accumulator is 0
					CPU->SetFlag(Negative, CPU->Accumulator & 0x80); //Set if Bit 7 of Accumulator is set. (AKA if accumulator is negative)
					CPU->ProgramCounter++;
				}}; //Stack Pull (Accumulator)
			OpTable[0x28] = { {'P', 'L', 'P', '\0'}, 0x28, Implied, 1, 4, [](NesCPU* CPU)
				{
					CPU->StackPointer++;
					CPU->StackPointer = CPU->ReadRAM(0x0100 + CPU->StackPointer);
					CPU->ProgramCounter++;
				}}; //Stack Pull (Status)
		}

		//Increment & Decrement Instructions
		{
			//Memory
			OpTable[0xE6] = { {'I', 'N', 'C', '\0'}, 0xE6, ZeroPage, 2, 5, [](NesCPU* CPU) 
				{
					CPU->ProgramCounter++;
					UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
					UInt8 NewValue = MemoryValue + 1;
					CPU->WriteRAM(MemoryAddress, NewValue);

					CPU->SetFlag(Zero, (NewValue == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (NewValue & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				}};		   //Increment Memory (Zero Page)
			OpTable[0xF6] = { {'I', 'N', 'C', '\0'}, 0xF6, ZeroPageIndexX, 2, 6, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 MemoryAddress = (CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X) % 256;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
					UInt8 NewValue = MemoryValue + 1;
					CPU->WriteRAM(MemoryAddress, NewValue);

					CPU->SetFlag(Zero, (NewValue == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (NewValue & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				}};	   //Increment Memory (Zero Page, Index X)
			OpTable[0xEE] = { {'I', 'N', 'C', '\0'}, 0xEE, Absolute, 3, 6, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
					UInt8 NewValue = MemoryValue + 1;

					CPU->WriteRAM(MemoryAddress, NewValue);

					CPU->SetFlag(Zero, (NewValue == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (NewValue & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} };		   //Increment Memory (Absolute)
			OpTable[0xFE] = { {'I', 'N', 'C', '\0'}, 0xFE, AbsoluteIndexedX, 3, 7, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
					UInt8 NewValue = MemoryValue + 1;

					CPU->WriteRAM(MemoryAddress, NewValue);

					CPU->SetFlag(Zero, (NewValue == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (NewValue & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} }; //Increment Memory (Absolute, Index X)
			OpTable[0xC6] = { {'D', 'E', 'C', '\0'}, 0xC6, ZeroPage, 2, 5, [](NesCPU* CPU) {
					CPU->ProgramCounter++;
					UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
					UInt8 NewValue = MemoryValue - 1;
					CPU->WriteRAM(MemoryAddress, NewValue);

					CPU->SetFlag(Zero, (NewValue == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (NewValue & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				}};		   //Decrement Memory (Zero Page)
			OpTable[0xD6] = { {'D', 'E', 'C', '\0'}, 0xD6, ZeroPageIndexX, 2, 6, [](NesCPU* CPU) {
					CPU->ProgramCounter++;
					UInt8 MemoryAddress = (CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X) % 256;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
					UInt8 NewValue = MemoryValue - 1;
					CPU->WriteRAM(MemoryAddress, NewValue); \

					CPU->SetFlag(Zero, (NewValue == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (NewValue & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} };   //Decrement Memory (Zero Page, Index X)
			OpTable[0xCE] = { {'D', 'E', 'C', '\0'}, 0xCE, Absolute, 3, 6, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
					UInt8 NewValue = MemoryValue - 1;

					CPU->WriteRAM(MemoryAddress, NewValue);

					CPU->SetFlag(Zero, (NewValue == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (NewValue & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} };		   //Decrement Memory (Absolute)
			OpTable[0xDE] = { {'D', 'E', 'C', '\0'}, 0xDE, AbsoluteIndexedX, 3, 7, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
					UInt8 NewValue = MemoryValue - 1;

					CPU->WriteRAM(MemoryAddress, NewValue);

					CPU->SetFlag(Zero, (NewValue == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (NewValue & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				} }; //Decrement Memory (Absolute, Index X)
			
			//X and Y Registers
			OpTable[0xE8] = { {'I', 'N', 'X', '\0'}, 0xE8, Implied, 1, 2, [](NesCPU* CPU) 
				{
					UInt8 NewValue = CPU->Index_X + 1;
					CPU->Index_X = NewValue;

					CPU->SetFlag(Zero, (CPU->Index_X == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (CPU->Index_X & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				}}; //Increment Index X
			OpTable[0xC8] = { {'I', 'N', 'Y', '\0'}, 0xC8, Implied, 1, 2, [](NesCPU* CPU)
				{
					UInt8 NewValue = CPU->Index_Y + 1;
					CPU->Index_Y = NewValue;

					CPU->SetFlag(Zero, (CPU->Index_Y == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (CPU->Index_Y & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				}}; //Increment Index Y
			OpTable[0xCA] = { {'D', 'E', 'X', '\0'}, 0xCA, Implied, 1, 2, [](NesCPU* CPU)
				{
					UInt8 NewValue = CPU->Index_X - 1;
					CPU->Index_X = NewValue;

					CPU->SetFlag(Zero, (CPU->Index_X == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (CPU->Index_X & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				}}; //Decrement Index X
			OpTable[0x88] = { {'D', 'E', 'Y', '\0'}, 0x88, Implied, 1, 2, [](NesCPU* CPU)
				{
					UInt8 NewValue = CPU->Index_Y - 1;
					CPU->Index_Y = NewValue;

					CPU->SetFlag(Zero, (CPU->Index_Y == 0) ? 1 : 0);
					CPU->SetFlag(Negative, (CPU->Index_Y & 0x80) ? 1 : 0);
					CPU->ProgramCounter++;
				}}; //Decrement Index Y
		}

		//Flag Instructions
		{
			OpTable[0x18] = { { 'C', 'L', 'C', '\0'}, 0x18, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Carry, 0);
					CPU->ProgramCounter++;
				} }; //Clear Carry Flag
			OpTable[0xD8] = { { 'C', 'L', 'D', '\0'}, 0xD8, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Decimal, 0);
					CPU->ProgramCounter++;
				} }; //Clear Decimal Flag
			OpTable[0x58] = { { 'C', 'L', 'I', '\0'}, 0x58, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Interrupt, 0);
					CPU->ProgramCounter++;
				} }; //Clear Interrupt Flag
			OpTable[0xB8] = { { 'C', 'L', 'V', '\0'}, 0xB8, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Overflow, 0);
					CPU->ProgramCounter++;
				} }; //Clear Overflow Flag

			OpTable[0x38] = { { 'S', 'E', 'C', '\0'}, 0x38, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Carry, 1);
					CPU->ProgramCounter++;
				} }; //Set Carry Flag
			OpTable[0xF8] = { { 'S', 'E', 'D', '\0'}, 0xF8, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Decimal, 1);
					CPU->ProgramCounter++;
				} }; //Set Decimal Flag
			OpTable[0x78] = { { 'S', 'E', 'I', '\0'}, 0x78, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Interrupt, 1);
					CPU->ProgramCounter++;
				} }; //Set Disable Interrupt Flag
		}

		//Branch Instructions
		{
			OpTable[0x90] = { { 'B', 'C', 'C', '\0'}, 0x90, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Carry) == 0)
					{
						CPU->CycleRemain++;
						UInt8 Offset = CPU->ReadRAM(CPU->ProgramCounter + 1);
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
				} }; //Branch on Carry Clear (BCC)
			OpTable[0xB0] = { { 'B', 'C', 'S', '\0'}, 0xB0, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Carry) == 1)
					{
						CPU->CycleRemain++;
						UInt8 Offset = CPU->ReadRAM(CPU->ProgramCounter + 1);
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
				} }; //Branch on Carry Set (BCS)
			OpTable[0xF0] = { { 'B', 'E', 'Q', '\0'}, 0xF0, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Zero) == 1)
					{
						CPU->CycleRemain++;
						UInt8 Offset = CPU->ReadRAM(CPU->ProgramCounter + 1);
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
				} }; //Branch on Result Zero (BEQ)
			OpTable[0x30] = { { 'B', 'M', 'I', '\0'}, 0x30, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Negative) == 1)
					{
						CPU->CycleRemain++;
						UInt8 Offset = CPU->ReadRAM(CPU->ProgramCounter + 1);
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
				} }; //Branch on Results Minus (BMI)
			OpTable[0xD0] = { { 'B', 'N', 'E', '\0'}, 0xD0, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Zero) == 0)
					{
						CPU->CycleRemain++;
						UInt8 Offset = CPU->ReadRAM(CPU->ProgramCounter + 1);
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
				} }; //Branch on Result Not Zero (BNE)
			OpTable[0x10] = { { 'B', 'P', 'L', '\0'}, 0x10, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Negative) == 0)
					{
						CPU->CycleRemain++;
						UInt8 Offset = CPU->ReadRAM(CPU->ProgramCounter + 1);
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
				} }; //Branch on Result Plus (BPL)
			OpTable[0x50] = { { 'B', 'V', 'C', '\0'}, 0x50, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Overflow) == 0)
					{
						CPU->CycleRemain++;
						UInt8 Offset = CPU->ReadRAM(CPU->ProgramCounter + 1);
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
				} }; //Branch on Overflow Clear (BVC)
			OpTable[0x70] = { { 'B', 'V', 'S', '\0'}, 0x70, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Overflow) == 1)
					{
						CPU->CycleRemain++;
						UInt8 Offset = CPU->ReadRAM(CPU->ProgramCounter + 1);
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
				} }; //Branch on Overflow Set (BVS)
		}

		//Jump & Subroutines Instructions
		{
			OpTable[0x4C] = { { 'J', 'M', 'P', '\0'}, 0x4C, Absolute, 3, 3, [](NesCPU* CPU) 
				{
					CPU->ProgramCounter++;
					UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);

					UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
					CPU->ProgramCounter = MemoryAddress;
				} };
			OpTable[0x6C] = { { 'J', 'M', 'P', '\0'}, 0x6C, Indirect, 3, 5, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;

					if (LowAddressByte == 0x00FF) // Simulate page boundary hardware bug
					{
						CPU->ProgramCounter = (CPU->ReadRAM(MemoryAddress & 0xFF00) << 8) | CPU->ReadRAM(MemoryAddress + 0);
					}
					else
					{
						CPU->ProgramCounter = (CPU->ReadRAM(MemoryAddress + 1) << 8) | CPU->ReadRAM(MemoryAddress + 0);
					}
				} };
			OpTable[0x20] = { { 'J', 'S', 'R', '\0'}, 0x20, Absolute, 3, 6, [](NesCPU* CPU) 
				{
					CPU->ProgramCounter++;
					UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);

					CPU->ProgramCounter -= 2;
					CPU->WriteRAM(0x0100 + CPU->StackPointer, (CPU->ProgramCounter >> 8) & 0x00FF);
					CPU->StackPointer--;
					CPU->WriteRAM(0x0100 + CPU->StackPointer, CPU->ProgramCounter & 0x00FF);
					CPU->StackPointer--;

					UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
					CPU->ProgramCounter = MemoryAddress;
				} };
			OpTable[0x60] = { { 'R', 'T', 'S', '\0'}, 0x60, Implied, 1, 6, [](NesCPU* CPU) 
				{
					CPU->StackPointer++;
					UInt8 LowAddressByte = CPU->ReadRAM(0x0100 + CPU->StackPointer);
					CPU->StackPointer++;
					UInt8 HighAddressByte = CPU->ReadRAM(0x0100 + CPU->StackPointer);

					UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
					CPU->ProgramCounter = MemoryAddress;
					CPU->ProgramCounter++;
				} };
		}

		//Other Instructions
		{
			OpTable[0xEA] = { { 'N', 'O', 'P', '\0' }, 0xEA, Implied, 1, 2, [](NesCPU* CPU)
			{
				CPU->ProgramCounter++;
			}}; //No Operation

			OpTable[0x24] = { { 'B', 'I', 'T', '\0' }, 0x24, ZeroPage, 2, 3, [](NesCPU* CPU)
			{
				CPU->ProgramCounter++;

				UInt8 TestByte = CPU->ReadRAM(CPU->ProgramCounter);
				CPU->ProgramCounter++;

				CPU->SetFlag(Zero, (((CPU->Accumulator & TestByte) & 0x00FF) == 0)); //Store the results of our AND operation.
				CPU->SetFlag(Overflow, TestByte & (1 << 7));
				CPU->SetFlag(Negative, TestByte & (1 << 6));
			} }; //Bit Test (Zero Page)
			OpTable[0x2C] = { { 'B', 'I', 'T', '\0' }, 0x2C, Absolute, 3, 4, [](NesCPU* CPU)
			{
				CPU->ProgramCounter++;
				UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
				CPU->ProgramCounter++;
				UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

				UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
				UInt8 TestByte = CPU->ReadRAM(MemoryAddress);
				CPU->ProgramCounter++;

				CPU->SetFlag(Zero, (((CPU->Accumulator& TestByte) & 0x00FF) == 0)); //Store the results of our AND operation.
				CPU->SetFlag(Overflow, TestByte& (1 << 7));
				CPU->SetFlag(Negative, TestByte& (1 << 6));

			} }; //Bit Test (Absolute Address)
		}
	}
	NesCPU::~NesCPU()
	{

	}

	void NesCPU::Fetch()
	{
		CurrOp = &OpTable[ReadRAM(ProgramCounter)];
		CycleRemain = CurrOp->Cycles;
	}
	std::string NesCPU::GetAddressMode(AddressingMode Mode)
	{
		switch (Mode)
		{
			case AccumulatorAddressing:
			{
				return "Accumulator";
				break;
			}
			case Absolute:
			{
				return "Absolute";
				break;
			}
			case AbsoluteIndexedX:
			{
				return "Absolute( Index X)";
				break;
			}
			case AbsoluteIndexedY:
			{
				return "Absolute( Index Y)";
				break;
			}
			case Immediate:
			{
				return "Immediate";
				break;
			}
			case Implied:
			{
				return "Implied";
				break;
			}
			case Indirect:
			{
				return "Indirect";
				break;
			}
			case IndirectIndexedX:
			{
				return "Indirect IndexedX";
				break;
			}
			case IndirectIndexedY:
			{
				return "Indirect IndexedY";
				break;
			}
			case Relative:
			{
				return "Relative";
				break;
			}
			case ZeroPage:
			{
				return "ZeroPage";
				break;
			}
			case ZeroPageIndexX:
			{
				return "ZeroPage (Index X)";
				break;
			}
			case ZeroPageIndexY:
			{
				return "ZeroPage (Index Y)";
				break;
			}
			default:
			{
				return "Unknown";
				break;
			}
		}
	}

	void NesCPU::Connect(NesConsole* Console)
	{
		System = Console;
	}
	void NesCPU::WriteRAM(UInt16 Address, UInt8 Data)
	{
		System->WriteRAM(Address, Data);
	}
	UInt8 NesCPU::ReadRAM(UInt16 Address)
	{
		return System->ReadRAM(Address);
	}

	void NesCPU::Clock()
	{
		if (CycleRemain == 0)
		{
			Fetch();
			CurrOp->OpFunc(this); 

			SetFlag(Unsed, 1);
		}

		CycleRemain--;
		CycleCounter++;
	}
	void NesCPU::Reset()
	{
		//Get our address from the reset vector.
		UInt8 LowByte = ReadRAM(0xFFFC);
		UInt8 HighByte = ReadRAM(0xFFFD);

		//Initalize our Registers.
		SetFlag(Break, 1);
		SetFlag(Decimal, 0);
		SetFlag(Interrupt, 1);
		SetFlag(Unsed, 1);
		StackPointer = 0xFF; //Sets the stack pointer, testing only, this gets set in software.
		ProgramCounter = (HighByte << 8) | LowByte; //Start Location.

		//Set our cycles remaining.
		CycleRemain = 7;
	}
	void NesCPU::NMI()
	{

	}
	void NesCPU::IRQ()
	{

	}

	UInt8 NesCPU::GetFlag(StatusFlag Flag)
	{
		return ((StatusFlags & Flag) > 0) ? 1 : 0;
	}
	void NesCPU::SetFlag(StatusFlag Flag, bool v)
	{
		if (v)
		{
			StatusFlags |= Flag;
		}
		else
		{
			StatusFlags &= ~Flag;
		}
	}

	void NesCPU::DrawRegisters()
	{
		ImGui::Begin("CPU Registers");

		ImGui::Text("Accumulator: 0x%X (%d)", Accumulator, Accumulator);
		ImGui::Text("X Index: 0x%X (%d)", Index_X, Index_X);
		ImGui::Text("Y Index: 0x%X (%d)", Index_Y, Index_Y);

		ImGui::Separator();

		ImGui::Text("Stack Pointer: 0x%X (%d)", StackPointer, StackPointer);
		ImGui::Text("Program Counter: 0x%X (%d)", ProgramCounter, ProgramCounter);
		ImGui::Text("Status Flags: 0x%X (%d)", StatusFlags, StatusFlags);

		ImGui::Separator();

		ImGui::Text("- Carry: %d", GetFlag(Carry));
		ImGui::Text("- Zero: %d", GetFlag(Zero));
		ImGui::Text("- Interrupt: %d", GetFlag(Interrupt));
		ImGui::Text("- Decimal: %d", GetFlag(Decimal));
		ImGui::Text("- Break: %d", GetFlag(Break));
		ImGui::Text("- Unsed: %d", GetFlag(Unsed));
		ImGui::Text("- Overflow: %d", GetFlag(Overflow));
		ImGui::Text("- Negative: %d", GetFlag(Negative));

		ImGui::Separator();

		ImGui::Text("Current Instruction: %s (0x%X)", ((CurrOp != nullptr) ? CurrOp->Name : "None"), ((CurrOp != nullptr) ? CurrOp->Opcode : 0x00));
		ImGui::Text("Current Addressing Mode: %s", (CurrOp != nullptr) ? GetAddressMode(CurrOp->Mode).c_str() : "None");
		ImGui::Text("Current Clock Cycle: %d", CycleCounter);
		ImGui::Text("Cycles Remaining: %d", CycleRemain);

		ImGui::End();
	}
}