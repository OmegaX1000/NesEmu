#include "NesCPU.h"
#include "NesConsole.h"

#include "Log.h"

namespace NesEmulator
{
	NesCPU::NesCPU()
	{
		//Define our instructions for our CPU. (note: probably wouldn't do this ...)

		//Load and Store Instructions
		{
			//LAS - "AND" Memory with Stack Pointer
			OpTable[0xBB] = { {'L', 'A', 'S', '\0'}, 0xBB, AbsoluteIndexedY, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

					UInt8 Result = CPU->StackPointer & MemoryValue;
					CPU->Accumulator = Result;
					CPU->Index_X = Result;
					CPU->StackPointer = Result;

					CPU->SetFlag(Negative, (Result & 0x80) ? 1 : 0);
					CPU->SetFlag(Zero, (Result == 0) ? 1 : 0);
					CPU->ProgramCounter++;
				} };

			//LAX - Load Accumulator and Index Register X From Memory
			{
				OpTable[0xAB] = { {'L', 'A', 'X', '\0'}, 0xAB, Immediate, 2, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);

					CPU->Accumulator = MemoryValue;
					CPU->Index_X = MemoryValue;

					CPU->SetFlag(Negative, (MemoryValue & 0x80) ? 1 : 0);
					CPU->SetFlag(Zero, (MemoryValue == 0) ? 1 : 0);
					CPU->ProgramCounter++;
				} };
				OpTable[0xAF] = { {'L', 'A', 'X', '\0'}, 0xAF, Absolute, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

					CPU->Accumulator = MemoryValue;
					CPU->Index_X = MemoryValue;

					CPU->SetFlag(Negative, (MemoryValue & 0x80) ? 1 : 0);
					CPU->SetFlag(Zero, (MemoryValue == 0) ? 1 : 0);
					CPU->ProgramCounter++;
				} };
				OpTable[0xBF] = { {'L', 'A', 'X', '\0'}, 0xBF, AbsoluteIndexedY, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

					if ((MemoryAddress & 0xFF00) != (HighAddresByte << 8))
					{
						CPU->CycleRemain++;
					}

					CPU->Accumulator = MemoryValue;
					CPU->Index_X = MemoryValue;

					CPU->SetFlag(Negative, (MemoryValue & 0x80) ? 1 : 0);
					CPU->SetFlag(Zero, (MemoryValue == 0) ? 1 : 0);
					CPU->ProgramCounter++;
				} };
				OpTable[0xA7] = { {'L', 'A', 'X', '\0'}, 0xA7, ZeroPage, 2, 3, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = AddresByte;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

					CPU->Accumulator = MemoryValue;
					CPU->Index_X = MemoryValue;

					CPU->SetFlag(Negative, (MemoryValue & 0x80) ? 1 : 0);
					CPU->SetFlag(Zero, (MemoryValue == 0) ? 1 : 0);
					CPU->ProgramCounter++;
				} };
				OpTable[0xB7] = { {'L', 'A', 'X', '\0'}, 0xB7, ZeroPageIndexY, 2, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
					UInt16 MemoryAddress = AddresByte;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

					CPU->Accumulator = MemoryValue;
					CPU->Index_X = MemoryValue;

					CPU->SetFlag(Negative, (MemoryValue & 0x80) ? 1 : 0);
					CPU->SetFlag(Zero, (MemoryValue == 0) ? 1 : 0);
					CPU->ProgramCounter++;
				} };
				OpTable[0xA3] = { {'L', 'A', 'X', '\0'}, 0xA3, IndirectIndexedX, 2, 6, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
					UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
					UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
					UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
					UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);

					CPU->Accumulator = MemoryValue;
					CPU->Index_X = MemoryValue;

					CPU->SetFlag(Negative, (MemoryValue & 0x80) ? 1 : 0);
					CPU->SetFlag(Zero, (MemoryValue == 0) ? 1 : 0);
					CPU->ProgramCounter++;
				} };
				OpTable[0xB3] = { {'L', 'A', 'X', '\0'}, 0xB3, IndirectIndexedY, 2, 5, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
					UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
					UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
					UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
					UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);

					CPU->Accumulator = MemoryValue;
					CPU->Index_X = MemoryValue;

					if ((MemoryAddress & 0xFF00) != (SecondAddressHigh << 8))
					{
						CPU->CycleRemain++;
					}

					CPU->SetFlag(Negative, (MemoryValue & 0x80) ? 1 : 0);
					CPU->SetFlag(Zero, (MemoryValue == 0) ? 1 : 0);
					CPU->ProgramCounter++;
				} };
			}
			
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

			//Load Index Y with Memory (LDY)	
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

			//SAX - Store Accumulator "AND" Index Register X in Memory
			{
				OpTable[0x8F] = { {'S', 'A', 'X', '\0'}, 0x8F, Absolute, 3, 4, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;

						UInt8 Result = CPU->Accumulator & CPU->Index_X;
						CPU->WriteRAM(MemoryAddress, Result);
						CPU->ProgramCounter++;
					} };
				OpTable[0x87] = { {'S', 'A', 'X', '\0'}, 0x87, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);

						UInt8 Result = CPU->Accumulator & CPU->Index_X;
						CPU->WriteRAM(MemoryAddress, Result);
						CPU->ProgramCounter++;
					} };
				OpTable[0x97] = { {'S', 'A', 'X', '\0'}, 0x97, ZeroPageIndexY, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;

						UInt8 Result = CPU->Accumulator & CPU->Index_X;
						CPU->WriteRAM(MemoryAddress, Result);
						CPU->ProgramCounter++;
					} };
				OpTable[0x83] = { {'S', 'A', 'X', '\0'}, 0x83, IndirectIndexedX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;

						UInt8 Result = CPU->Accumulator & CPU->Index_X;
						CPU->WriteRAM(FinalMemoryAddress, Result);
						CPU->ProgramCounter++;
					} };
			}

			//SHA - Store Accumulator "AND" Index Register X "AND" Value
			{
				OpTable[0x9F] = { {'S', 'H', 'A', '\0'}, 0x9F, AbsoluteIndexedY, 3, 5, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 Value = CPU->ReadRAM(CPU->ProgramCounter) + 1;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;

						UInt8 Result = CPU->Accumulator & CPU->Index_X & Value;
						CPU->WriteRAM(MemoryAddress, Result);
						CPU->ProgramCounter++;
					} };
				OpTable[0x93] = { {'S', 'H', 'A', '\0'}, 0x93, IndirectIndexedY, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddressNoY = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;

						UInt8 Value = CPU->ReadRAM(MemoryAddressNoY) + 1;

						UInt8 Result = CPU->Accumulator & CPU->Index_X & Value;
						CPU->WriteRAM(MemoryAddress, Result);
						CPU->ProgramCounter++;
					} };
			}

			OpTable[0x9E] = { {'S', 'H', 'X', '\0'}, 0x9E, AbsoluteIndexedY, 3, 5, [](NesCPU* CPU) 
				{
					CPU->ProgramCounter++;
					UInt8 LowByteAddrNoY = CPU->ReadRAM(CPU->ProgramCounter);
					UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
					CPU->ProgramCounter++;
					UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;

					UInt8 Value = LowByteAddrNoY + 1;
					UInt8 Result = CPU->Index_X & Value;
					CPU->WriteRAM(MemoryAddress, Result);
					CPU->ProgramCounter++;
				} };
			OpTable[0x9C] = { {'S', 'H', 'Y', '\0'}, 0x9C, AbsoluteIndexedX, 3, 5, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowByteAddrNoY = CPU->ReadRAM(CPU->ProgramCounter);
					UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
					CPU->ProgramCounter++;
					UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;

					UInt8 Value = LowByteAddrNoY + 1;
					UInt8 Result = CPU->Index_Y & Value;
					CPU->WriteRAM(MemoryAddress, Result);
					CPU->ProgramCounter++;
				} };

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
						CPU->ProgramCounter++;
					} };
				OpTable[0x9D] = { { 'S', 'T', 'A', '\0'}, 0x9D, AbsoluteIndexedX, 3, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						CPU->WriteRAM(MemoryAddress, CPU->Accumulator);
						CPU->ProgramCounter++;
					} };
				OpTable[0x99] = { { 'S', 'T', 'A', '\0'}, 0x99, AbsoluteIndexedY, 3, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						CPU->WriteRAM(MemoryAddress, CPU->Accumulator);
						CPU->ProgramCounter++;
					} };
				OpTable[0x85] = { { 'S', 'T', 'A', '\0'}, 0x85, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						CPU->WriteRAM(AddresByte, CPU->Accumulator);
						CPU->ProgramCounter++;
					} };
				OpTable[0x95] = { { 'S', 'T', 'A', '\0'}, 0x95, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;

						CPU->WriteRAM(AddresByte, CPU->Accumulator);
						CPU->ProgramCounter++;
					} };
				OpTable[0x81] = { { 'S', 'T', 'A', '\0'}, 0x81, IndirectIndexedX, 2, 6, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						CPU->WriteRAM(FinalMemoryAddress, CPU->Accumulator);
						CPU->ProgramCounter++;
					} };
				OpTable[0x91] = { { 'S', 'T', 'A', '\0'}, 0x91, IndirectIndexedY, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt16 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						CPU->WriteRAM(FinalMemoryAddress, CPU->Accumulator);
						CPU->ProgramCounter++;
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
						CPU->ProgramCounter++;
					} };
				OpTable[0x86] = { { 'S', 'T', 'X', '\0'}, 0x86, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						CPU->WriteRAM(AddresByte, CPU->Index_X);
						CPU->ProgramCounter++;
					} };
				OpTable[0x96] = { { 'S', 'T', 'X', '\0'}, 0x96, ZeroPageIndexY, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;

						CPU->WriteRAM(AddresByte, CPU->Index_X);
						CPU->ProgramCounter++;
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
						CPU->ProgramCounter++;
					} };
				OpTable[0x84] = { { 'S', 'T', 'Y', '\0'}, 0x84, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter);

						CPU->WriteRAM(AddresByte, CPU->Index_Y);
						CPU->ProgramCounter++;
					} };
				OpTable[0x94] = { { 'S', 'T', 'Y', '\0'}, 0x94, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 AddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;

						CPU->WriteRAM(AddresByte, CPU->Index_Y);
						CPU->ProgramCounter++;
					} };
			}
		}

		//Transfer Instructions
		{
			OpTable[0x9B] = { {'S', 'H', 'S', '\0'}, 0x9B, AbsoluteIndexedY, 3, 5, [](NesCPU* CPU) 
				{
					//AND operation and store it in the stack
					UInt8 FirstResult = CPU->Accumulator & CPU->Index_X;
					CPU->WriteRAM(0x0100 + CPU->StackPointer, FirstResult);
					CPU->StackPointer--;

					//And operation and store in in the given memory address.
					CPU->ProgramCounter++;
					UInt8 LowByteAddrNoY = CPU->ReadRAM(CPU->ProgramCounter);
					UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
					CPU->ProgramCounter++;
					UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
					UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
					CPU->ProgramCounter++;

					UInt8 SecondResult = CPU->StackPointer & (LowByteAddrNoY + 1);
					CPU->WriteRAM(MemoryAddress, SecondResult);

				} }; //(Unoffical) Transfer Accumulator "AND" Index Register X to Stack Pointer then Store Stack Pointer "AND" Hi-Byte In Memory
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
					CPU->StatusFlags = CPU->ReadRAM(0x0100 + CPU->StackPointer);
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
						} };		   //Decrement Memory (Zero Page)
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
						} }; //Increment Index X
				OpTable[0xC8] = { {'I', 'N', 'Y', '\0'}, 0xC8, Implied, 1, 2, [](NesCPU* CPU)
						{
							UInt8 NewValue = CPU->Index_Y + 1;
							CPU->Index_Y = NewValue;

							CPU->SetFlag(Zero, (CPU->Index_Y == 0) ? 1 : 0);
							CPU->SetFlag(Negative, (CPU->Index_Y & 0x80) ? 1 : 0);
							CPU->ProgramCounter++;
						} }; //Increment Index Y
				OpTable[0xCA] = { {'D', 'E', 'X', '\0'}, 0xCA, Implied, 1, 2, [](NesCPU* CPU)
						{
							UInt8 NewValue = CPU->Index_X - 1;
							CPU->Index_X = NewValue;

							CPU->SetFlag(Zero, (CPU->Index_X == 0) ? 1 : 0);
							CPU->SetFlag(Negative, (CPU->Index_X & 0x80) ? 1 : 0);
							CPU->ProgramCounter++;
						} }; //Decrement Index X
				OpTable[0x88] = { {'D', 'E', 'Y', '\0'}, 0x88, Implied, 1, 2, [](NesCPU* CPU)
						{
							UInt8 NewValue = CPU->Index_Y - 1;
							CPU->Index_Y = NewValue;

							CPU->SetFlag(Zero, (CPU->Index_Y == 0) ? 1 : 0);
							CPU->SetFlag(Negative, (CPU->Index_Y & 0x80) ? 1 : 0);
							CPU->ProgramCounter++;
						} }; //Decrement Index Y
		}

		//Arithmetic Instructions
		{
			//ADC - Add Memory to Accumulator with Carry
			{
				OpTable[0x69] = { {'A', 'D', 'C', '\0'}, 0x69, Immediate, 2, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 Result = (UInt16)CPU->Accumulator + (UInt16)MemoryValue + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result > 255);
						CPU->SetFlag(Zero, (Result & 0x00FF) == 0);
						CPU->SetFlag(Overflow, (~((uint16_t)CPU->Accumulator ^ (uint16_t)MemoryValue) & ((uint16_t)CPU->Accumulator ^ (uint16_t)Result)) & 0x0080);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0x6D] = { {'A', 'D', 'C', '\0'}, 0x6D, Absolute, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Result = (UInt16)CPU->Accumulator + (UInt16)MemoryValue + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result > 255);
						CPU->SetFlag(Zero, (Result & 0x00FF) == 0);
						CPU->SetFlag(Overflow, (~((uint16_t)CPU->Accumulator ^ (uint16_t)MemoryValue) & ((uint16_t)CPU->Accumulator ^ (uint16_t)Result)) & 0x0080);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0x7D] = { {'A', 'D', 'C', '\0'}, 0x7D, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Result = (UInt16)CPU->Accumulator + (UInt16)MemoryValue + (UInt16)CPU->GetFlag(Carry);

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Carry, Result > 255);
						CPU->SetFlag(Zero, (Result & 0x00FF) == 0);
						CPU->SetFlag(Overflow, (~((uint16_t)CPU->Accumulator ^ (uint16_t)MemoryValue) & ((uint16_t)CPU->Accumulator ^ (uint16_t)Result)) & 0x0080);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0x79] = { {'A', 'D', 'C', '\0'}, 0x79, AbsoluteIndexedY, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Result = (UInt16)CPU->Accumulator + (UInt16)MemoryValue + (UInt16)CPU->GetFlag(Carry);

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Carry, Result > 255);
						CPU->SetFlag(Zero, (Result & 0x00FF) == 0);
						CPU->SetFlag(Overflow, (~((uint16_t)CPU->Accumulator ^ (uint16_t)MemoryValue) & ((uint16_t)CPU->Accumulator ^ (uint16_t)Result)) & 0x0080);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0x65] = { {'A', 'D', 'C', '\0'}, 0x65, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Result = (UInt16)CPU->Accumulator + (UInt16)MemoryValue + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result > 255);
						CPU->SetFlag(Zero, (Result & 0x00FF) == 0);
						CPU->SetFlag(Overflow, (~((uint16_t)CPU->Accumulator ^ (uint16_t)MemoryValue) & ((uint16_t)CPU->Accumulator ^ (uint16_t)Result)) & 0x0080);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0x75] = { {'A', 'D', 'C', '\0'}, 0x75, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Result = (UInt16)CPU->Accumulator + (UInt16)MemoryValue + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result > 255);
						CPU->SetFlag(Zero, (Result & 0x00FF) == 0);
						CPU->SetFlag(Overflow, (~((uint16_t)CPU->Accumulator ^ (uint16_t)MemoryValue) & ((uint16_t)CPU->Accumulator ^ (uint16_t)Result)) & 0x0080);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0x61] = { {'A', 'D', 'C', '\0'}, 0x61, IndirectIndexedX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						UInt16 Result = (UInt16)CPU->Accumulator + (UInt16)MemoryValue + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result > 255);
						CPU->SetFlag(Zero, (Result & 0x00FF) == 0);
						CPU->SetFlag(Overflow, (~((uint16_t)CPU->Accumulator ^ (uint16_t)MemoryValue) & ((uint16_t)CPU->Accumulator ^ (uint16_t)Result)) & 0x0080);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0x71] = { {'A', 'D', 'C', '\0'}, 0x71, IndirectIndexedY, 2, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						UInt16 Result = (UInt16)CPU->Accumulator + (UInt16)MemoryValue + (UInt16)CPU->GetFlag(Carry);

						if ((FinalMemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Carry, Result > 255);
						CPU->SetFlag(Zero, (Result & 0x00FF) == 0);
						CPU->SetFlag(Overflow, (~((uint16_t)CPU->Accumulator ^ (uint16_t)MemoryValue) & ((uint16_t)CPU->Accumulator ^ (uint16_t)Result)) & 0x0080);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
			}

			//ANC - "AND" Memory with Accumulator then Move Negative Flag to Carry Flag
			{
				OpTable[0x0B] = { {'A', 'N', 'C', '\0'}, 0x0B, Immediate, 2, 2, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 Result = CPU->Accumulator & MemoryValue;
						CPU->Accumulator = Result;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0) ? 1 : 0);
						CPU->SetFlag(Negative, (CPU->Accumulator & 0x80) ? 1 : 0);
						CPU->SetFlag(Carry, (CPU->Accumulator & 0x80) ? 1 : 0);
						CPU->ProgramCounter++;
					} };
				OpTable[0x2B] = { {'A', 'N', 'C', '\0'}, 0x2B, Immediate, 2, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 Result = CPU->Accumulator & MemoryValue;
						CPU->Accumulator = Result;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0) ? 1 : 0);
						CPU->SetFlag(Negative, (CPU->Accumulator & 0x80) ? 1 : 0);
						CPU->SetFlag(Carry, (CPU->Accumulator & 0x80) ? 1 : 0);
						CPU->ProgramCounter++;
					} };
			}

			OpTable[0x6B] = { {'A', 'R', 'R', '\0'}, 0x6B, Immediate, 2, 2, [](NesCPU* CPU) 
				{
					CPU->ProgramCounter++;
					UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
					UInt8 Result = (CPU->GetFlag(Carry) << 7) | (MemoryValue >> 1);
					CPU->Accumulator = Result;

					CPU->SetFlag(Negative, (CPU->Accumulator & 0x80) ? 1 : 0);
					CPU->SetFlag(Zero, (CPU->Accumulator == 0) ? 1 : 0);
					CPU->SetFlag(Overflow, ((CPU->Accumulator & 0x40) != (CPU->Accumulator & 0x020)) ? 1 : 0);
					CPU->SetFlag(Carry, (CPU->Accumulator & 0x40) ? 1 : 0);
					CPU->ProgramCounter++;

				} }; //ARR - "AND" Accumulator then Rotate Right
			OpTable[0x4B] = { {'A', 'S', 'R', '\0'}, 0x4B, Immediate, 2, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
					UInt8 Value = (CPU->Accumulator & MemoryValue) >> 1;
					Value = Value & 0x80;
					CPU->Accumulator = Value;

					CPU->SetFlag(Negative, 0);
					CPU->SetFlag(Zero, (CPU->Accumulator == 0) ? 1 : 0);
					CPU->SetFlag(Carry, CPU->Accumulator & 0x01);
					CPU->ProgramCounter++;

				} }; //ASR - "AND" then Logical Shift Right

			//DCP - Decrement Memory By One then Compare with Accumulator
			{

			}

			//SBC - Add Memory to Accumulator with Carry
			{
				OpTable[0xE9] = { {'S', 'B', 'C', '\0'}, 0xE9, Immediate, 2, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 Value = ((UInt16)MemoryValue) ^ 0x00FF;
						UInt16 Result = (UInt16)CPU->Accumulator + Value + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result & 0xFF00);
						CPU->SetFlag(Zero, ((Result & 0x00FF) == 0));
						CPU->SetFlag(Overflow, (Result ^ (UInt16)CPU->Accumulator) & (Result ^ Value) & 0x80);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0xED] = { {'S', 'B', 'C', '\0'}, 0xED, Absolute, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Value = ((UInt16)MemoryValue) ^ 0x00FF;
						UInt16 Result = (UInt16)CPU->Accumulator + Value + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result & 0xFF00);
						CPU->SetFlag(Zero, ((Result & 0x00FF) == 0));
						CPU->SetFlag(Overflow, (Result ^ (UInt16)CPU->Accumulator) & (Result ^ Value) & 0x80);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0xFD] = { {'S', 'B', 'C', '\0'}, 0xFD, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Value = ((UInt16)MemoryValue) ^ 0x00FF;
						UInt16 Result = (UInt16)CPU->Accumulator + Value + (UInt16)CPU->GetFlag(Carry);

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Carry, Result & 0xFF00);
						CPU->SetFlag(Zero, ((Result & 0x00FF) == 0));
						CPU->SetFlag(Overflow, (Result ^ (UInt16)CPU->Accumulator) & (Result ^ Value) & 0x80);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0xF9] = { {'S', 'B', 'C', '\0'}, 0xF9, AbsoluteIndexedY, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Value = ((UInt16)MemoryValue) ^ 0x00FF;
						UInt16 Result = (UInt16)CPU->Accumulator + Value + (UInt16)CPU->GetFlag(Carry);

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Carry, Result & 0xFF00);
						CPU->SetFlag(Zero, ((Result & 0x00FF) == 0));
						CPU->SetFlag(Overflow, (Result ^ (UInt16)CPU->Accumulator) & (Result ^ Value) & 0x80);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0xE5] = { {'S', 'B', 'C', '\0'}, 0xE5, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Value = ((UInt16)MemoryValue) ^ 0x00FF;
						UInt16 Result = (UInt16)CPU->Accumulator + Value + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result & 0xFF00);
						CPU->SetFlag(Zero, ((Result & 0x00FF) == 0));
						CPU->SetFlag(Overflow, (Result ^ (UInt16)CPU->Accumulator) & (Result ^ Value) & 0x80);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0xF5] = { {'S', 'B', 'C', '\0'}, 0xF5, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						UInt16 Value = ((UInt16)MemoryValue) ^ 0x00FF;
						UInt16 Result = (UInt16)CPU->Accumulator + Value + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result & 0xFF00);
						CPU->SetFlag(Zero, ((Result & 0x00FF) == 0));
						CPU->SetFlag(Overflow, (Result ^ (UInt16)CPU->Accumulator) & (Result ^ Value) & 0x80);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0xE1] = { {'S', 'B', 'C', '\0'}, 0xE1, IndirectIndexedX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						UInt16 Value = ((UInt16)MemoryValue) ^ 0x00FF;
						UInt16 Result = (UInt16)CPU->Accumulator + Value + (UInt16)CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, Result & 0xFF00);
						CPU->SetFlag(Zero, ((Result & 0x00FF) == 0));
						CPU->SetFlag(Overflow, (Result ^ (UInt16)CPU->Accumulator) & (Result ^ Value) & 0x80);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
				OpTable[0xF1] = { {'S', 'B', 'C', '\0'}, 0xF1, IndirectIndexedY, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						UInt16 Value = ((UInt16)MemoryValue) ^ 0x00FF;
						UInt16 Result = (UInt16)CPU->Accumulator + Value + (UInt16)CPU->GetFlag(Carry);

						if ((FinalMemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Carry, Result & 0xFF00);
						CPU->SetFlag(Zero, ((Result & 0x00FF) == 0));
						CPU->SetFlag(Overflow, (Result ^ (UInt16)CPU->Accumulator) & (Result ^ Value) & 0x80);
						CPU->SetFlag(Negative, Result & 0x80);

						CPU->Accumulator = Result & 0x00FF;
						CPU->ProgramCounter++;
					} };
			}
		}

		//Logical Instructions
		{
			//"AND" Memory with Accumulator
			{
				OpTable[0x29] = { {'A', 'N', 'D', '\0'}, 0x29, Immediate, 2, 2, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator & MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x2D] = { {'A', 'N', 'D', '\0'}, 0x2D, Absolute, 3, 4, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator & MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator& (1 << 7));
					} };
				OpTable[0x3D] = { {'A', 'N', 'D', '\0'}, 0x3D, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->ProgramCounter++;

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->Accumulator = CPU->Accumulator & MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x39] = { {'A', 'N', 'D', '\0'}, 0x39, AbsoluteIndexedY, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->ProgramCounter++;

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->Accumulator = CPU->Accumulator & MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x25] = { {'A', 'N', 'D', '\0'}, 0x25, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						CPU->Accumulator = CPU->Accumulator & MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x35] = { {'A', 'N', 'D', '\0'}, 0x35, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						CPU->Accumulator = CPU->Accumulator & MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x21] = { {'A', 'N', 'D', '\0'}, 0x21, IndirectIndexedX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator & MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x31] = { {'A', 'N', 'D', '\0'}, 0x31, IndirectIndexedY, 2, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator & MemoryValue;

						if ((MemoryAddress & 0xFF00) != (SecondAddressHigh << 8))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
			}

			//EOR - "Exclusive OR" Memory with Accumulator
			{
				OpTable[0x49] = { {'E', 'O', 'R', '\0'}, 0x49, Immediate, 2, 2, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator ^ MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator& (1 << 7));
					} };
				OpTable[0x4D] = { {'E', 'O', 'R', '\0'}, 0x4D, Absolute, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator ^ MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x5D] = { {'E', 'O', 'R', '\0'}, 0x5D, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->ProgramCounter++;

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->Accumulator = CPU->Accumulator ^ MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x59] = { {'E', 'O', 'R', '\0'}, 0x59, AbsoluteIndexedY, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->ProgramCounter++;

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->Accumulator = CPU->Accumulator ^ MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x45] = { {'E', 'O', 'R', '\0'}, 0x45, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						CPU->Accumulator = CPU->Accumulator ^ MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x55] = { {'E', 'O', 'R', '\0'}, 0x55, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						CPU->Accumulator = CPU->Accumulator ^ MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x41] = { {'E', 'O', 'R', '\0'}, 0x41, IndirectIndexedX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator ^ MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x51] = { {'E', 'O', 'R', '\0'}, 0x51, IndirectIndexedY, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator ^ MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
			}

			//ORA - "OR" Memory with Accumulator
			{
				OpTable[0x09] = { {'O', 'R', 'A', '\0'}, 0x09, Immediate, 2, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator | MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator& (1 << 7));
					} };
				OpTable[0x0D] = { {'O', 'R', 'A', '\0'}, 0x0D, Absolute, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator | MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x1D] = { {'O', 'R', 'A', '\0'}, 0x1D, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator | MemoryValue;

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x19] = { {'O', 'R', 'A', '\0'}, 0x19, AbsoluteIndexedY, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowByteAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						CPU->ProgramCounter++;
						UInt8 HighByteAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighByteAddress << 8) | LowByteAddress;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator | MemoryValue;

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x05] = { {'O', 'R', 'A', '\0'}, 0x05, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						CPU->Accumulator = CPU->Accumulator | MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x15] = { {'O', 'R', 'A', '\0'}, 0x15, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						CPU->Accumulator = CPU->Accumulator | MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x01] = { {'O', 'R', 'A', '\0'}, 0x01, IndirectIndexedX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator | MemoryValue;

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
				OpTable[0x11] = { {'O', 'R', 'A', '\0'}, 0x11, IndirectIndexedY, 2, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						CPU->ProgramCounter++;

						CPU->Accumulator = CPU->Accumulator | MemoryValue;

						if ((MemoryAddress & 0xFF00) != (SecondAddressHigh << 8))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Zero, (CPU->Accumulator == 0x00) ? 1 : 0);
						CPU->SetFlag(Negative, CPU->Accumulator & (1 << 7));
					} };
			}
		}

		//Shift & Rotate Instructions
		{
			//ASL - Arithmetic Shift Left
			{
				OpTable[0x0A] = { {'A', 'S', 'L', '\0'}, 0x0A, AccumulatorAddressing, 1, 2, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 NewValue = (CPU->Accumulator << 1);
						CPU->SetFlag(Carry, (NewValue & 0xFF00) > 0);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->Accumulator = NewValue & 0x00FF;
					} };
				OpTable[0x0E] = { {'A', 'S', 'L', '\0'}, 0x0E, Absolute, 3, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue << 1);

						CPU->SetFlag(Carry, (NewValue & 0xFF00) > 0);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x1E] = { {'A', 'S', 'L', '\0'}, 0x1E, AbsoluteIndexedX, 3, 7, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue << 1);

						CPU->SetFlag(Carry, (NewValue & 0xFF00) > 0);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x06] = { {'A', 'S', 'L', '\0'}, 0x06, ZeroPage, 2, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue << 1);
						CPU->SetFlag(Carry, (NewValue & 0xFF00) > 0);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x16] = { {'A', 'S', 'L', '\0'}, 0x16, ZeroPageIndexX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue << 1);
						CPU->SetFlag(Carry, (NewValue & 0xFF00) > 0);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
			}

			//LSR - Logical Shift Right
			{
				OpTable[0x4A] = { {'L', 'S', 'R', '\0'}, 0x4A, AccumulatorAddressing, 1, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 NewValue = (CPU->Accumulator >> 1);
						CPU->SetFlag(Carry, NewValue & 0x0001);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->Accumulator = NewValue & 0x00FF;
					} };
				OpTable[0x4E] = { {'L', 'S', 'R', '\0'}, 0x4E, Absolute, 3, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue >> 1);

						CPU->SetFlag(Carry, NewValue & 0x0001);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x5E] = { {'L', 'S', 'R', '\0'}, 0x5E, AbsoluteIndexedX, 3, 7, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue >> 1);

						CPU->SetFlag(Carry, NewValue & 0x0001);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x46] = { {'L', 'S', 'R', '\0'}, 0x46, ZeroPage, 2, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue >> 1);
						CPU->SetFlag(Carry, NewValue & 0x0001);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x56] = { {'L', 'S', 'R', '\0'}, 0x56, ZeroPageIndexX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue >> 1);
						CPU->SetFlag(Carry, NewValue & 0x0001);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
			}

			//ROL - Rotate Left
			{
				OpTable[0x2A] = { {'R', 'O', 'L', '\0'}, 0x2A, AccumulatorAddressing, 1, 2, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 NewValue = (CPU->Accumulator << 1) | CPU->GetFlag(Carry);
						CPU->SetFlag(Carry, NewValue & 0xFF00);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->Accumulator = NewValue & 0x00FF;
					} };
				OpTable[0x2E] = { {'R', 'O', 'L', '\0'}, 0x2E, Absolute, 3, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (CPU->Accumulator << 1) | CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, NewValue & 0xFF00);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x3E] = { {'R', 'O', 'L', '\0'}, 0x3E, AbsoluteIndexedX, 3, 7, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (CPU->Accumulator << 1) | CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, NewValue & 0xFF00);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x26] = { {'R', 'O', 'L', '\0'}, 0x26, ZeroPage, 2, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue << 1) | CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, NewValue & 0xFF00);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x36] = { {'R', 'O', 'L', '\0'}, 0x36, ZeroPageIndexX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (OldValue << 1) | CPU->GetFlag(Carry);

						CPU->SetFlag(Carry, NewValue & 0xFF00);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
			}

			//ROR - Rotate Right
			{
				OpTable[0x6A] = { {'R', 'O', 'R', '\0'}, 0x6A, AccumulatorAddressing, 1, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 NewValue = (CPU->GetFlag(Carry) << 7) | (CPU->Accumulator >> 1);
						CPU->SetFlag(Carry, CPU->Accumulator & 0x01);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->Accumulator = NewValue & 0x00FF;
					} };
				OpTable[0x6E] = { {'R', 'O', 'R', '\0'}, 0x6E, Absolute, 3, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (CPU->GetFlag(Carry) << 7) | (OldValue >> 1);

						CPU->SetFlag(Carry, OldValue & 0x01);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x7E] = { {'R', 'O', 'R', '\0'}, 0x7E, AbsoluteIndexedX, 3, 7, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (CPU->GetFlag(Carry) << 7) | (OldValue >> 1);

						CPU->SetFlag(Carry, OldValue & 0x01);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x66] = { {'R', 'O', 'R', '\0'}, 0x66, ZeroPage, 2, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (CPU->GetFlag(Carry) << 7) | (OldValue >> 1);

						CPU->SetFlag(Carry, OldValue & 0x01);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
				OpTable[0x76] = { {'R', 'O', 'R', '\0'}, 0x76, ZeroPageIndexX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 OldValue = CPU->ReadRAM(MemoryAddress);
						UInt8 NewValue = (CPU->GetFlag(Carry) << 7) | (OldValue >> 1);

						CPU->SetFlag(Carry, OldValue & 0x01);
						CPU->SetFlag(Zero, (NewValue & 0x00FF) == 0x00);
						CPU->SetFlag(Negative, (NewValue & 0x80));

						CPU->WriteRAM(MemoryAddress, NewValue & 0x00FF);
						CPU->ProgramCounter++;
					} };
			}
		}

		//Flag Instructions
		{
			OpTable[0x18] = { {'C', 'L', 'C', '\0'}, 0x18, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Carry, 0);
					CPU->ProgramCounter++;
				} }; //Clear Carry Flag
			OpTable[0xD8] = { {'C', 'L', 'D', '\0'}, 0xD8, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Decimal, 0);
					CPU->ProgramCounter++;
				} }; //Clear Decimal Flag
			OpTable[0x58] = { {'C', 'L', 'I', '\0'}, 0x58, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Interrupt, 0);
					CPU->ProgramCounter++;
				} }; //Clear Interrupt Flag
			OpTable[0xB8] = { {'C', 'L', 'V', '\0'}, 0xB8, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Overflow, 0);
					CPU->ProgramCounter++;
				} }; //Clear Overflow Flag

			OpTable[0x38] = { {'S', 'E', 'C', '\0'}, 0x38, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Carry, 1);
					CPU->ProgramCounter++;
				} }; //Set Carry Flag
			OpTable[0xF8] = { {'S', 'E', 'D', '\0'}, 0xF8, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Decimal, 1);
					CPU->ProgramCounter++;
				} }; //Set Decimal Flag
			OpTable[0x78] = { {'S', 'E', 'I', '\0'}, 0x78, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->SetFlag(Interrupt, 1);
					CPU->ProgramCounter++;
				} }; //Set Disable Interrupt Flag
		}

		//Compare Instructions
		{
			//CMP - Compare Memory and Accumulator
			{
				OpTable[0xC9] = { {'C', 'M', 'P', '\0'}, 0xC9, Immediate, 2, 2, [](NesCPU* CPU) 
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);

						UInt8 Result = CPU->Accumulator - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Accumulator >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Accumulator == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
				OpTable[0xCD] = { {'C', 'M', 'P', '\0'}, 0xCD, Absolute, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);
						
						UInt8 Result = CPU->Accumulator - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Accumulator >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Accumulator == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
				OpTable[0xDD] = { {'C', 'M', 'P', '\0'}, 0xDD, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						CPU->ProgramCounter++;
						UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						UInt8 Result = CPU->Accumulator - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Accumulator >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Accumulator == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
				OpTable[0xD9] = { {'C', 'M', 'P', '\0'}, 0xD9, AbsoluteIndexedY, 3, 4, [](NesCPU* CPU)
						{
							CPU->ProgramCounter++;
							UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
							CPU->ProgramCounter++;
							UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
							UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
							UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

							if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
							{
								CPU->CycleRemain++;
							}

							UInt8 Result = CPU->Accumulator - MemoryValue;

							CPU->SetFlag(Carry, (CPU->Accumulator >= MemoryValue) ? 1 : 0);
							CPU->SetFlag(Zero, (CPU->Accumulator == MemoryValue) ? 1 : 0);
							CPU->SetFlag(Negative, Result & (1 << 7));
							CPU->ProgramCounter++;
						} };
				OpTable[0xC5] = { {'C', 'M', 'P', '\0'}, 0xC5, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						UInt8 Result = CPU->Accumulator - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Accumulator >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Accumulator == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
				OpTable[0xD5] = { {'C', 'M', 'P', '\0'}, 0xD5, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						UInt8 Result = CPU->Accumulator - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Accumulator >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Accumulator == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
				OpTable[0xC1] = { {'C', 'M', 'P', '\0'}, 0xC1, IndirectIndexedX, 2, 6, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_X;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						CPU->ProgramCounter++;

						UInt8 Result = CPU->Accumulator - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Accumulator >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Accumulator == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
					} };
				OpTable[0xD1] = { {'C', 'M', 'P', '\0'}, 0xD1, IndirectIndexedY, 2, 5, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter) + CPU->Index_Y;
						UInt8 SecondAddressLow = CPU->ReadRAM(MemoryAddress);
						UInt8 SecondAddressHigh = CPU->ReadRAM(MemoryAddress + 1);
						UInt16 FinalMemoryAddress = (SecondAddressHigh << 8) | SecondAddressLow;
						UInt8 MemoryValue = CPU->ReadRAM(FinalMemoryAddress);
						CPU->ProgramCounter++;

						UInt8 Result = CPU->Accumulator - MemoryValue;

						if ((MemoryAddress & 0xFF00) != (SecondAddressHigh << 8))
						{
							CPU->CycleRemain++;
						}

						CPU->SetFlag(Carry, (CPU->Accumulator >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Accumulator == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
					} };
			}

			//CXP - Compare Index Register X To Memory
			{
				OpTable[0xE0] = { {'C', 'X', 'P', '\0'}, 0xE0, Immediate, 2, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);

						UInt8 Result = CPU->Index_X - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Index_X >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Index_X == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result& (1 << 7));
						CPU->ProgramCounter++;
					} };
				OpTable[0xEC] = { {'C', 'X', 'P', '\0'}, 0xEC, Absolute, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						UInt8 Result = CPU->Index_X - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Index_X >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Index_X == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
				OpTable[0xE4] = { {'C', 'X', 'P', '\0'}, 0xE4, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						UInt8 Result = CPU->Index_X - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Index_X >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Index_X == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
			}

			//CYP - Compare Index Register Y To Memory
			{
				OpTable[0xC0] = { {'C', 'Y', 'P', '\0'}, 0xC0, Immediate, 2, 2, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryValue = CPU->ReadRAM(CPU->ProgramCounter);

						UInt8 Result = CPU->Index_Y - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Index_Y >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Index_Y == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
				OpTable[0xCC] = { {'C', 'Y', 'P', '\0'}, 0xCC, Absolute, 3, 4, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
						UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						UInt8 Result = CPU->Index_Y - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Index_Y >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Index_Y == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
				OpTable[0xC4] = { {'C', 'Y', 'P', '\0'}, 0xC4, ZeroPage, 2, 3, [](NesCPU* CPU)
					{
						CPU->ProgramCounter++;
						UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
						UInt8 MemoryValue = CPU->ReadRAM(MemoryAddress);

						UInt8 Result = CPU->Index_Y - MemoryValue;

						CPU->SetFlag(Carry, (CPU->Index_Y >= MemoryValue) ? 1 : 0);
						CPU->SetFlag(Zero, (CPU->Index_Y == MemoryValue) ? 1 : 0);
						CPU->SetFlag(Negative, Result & (1 << 7));
						CPU->ProgramCounter++;
					} };
			}
		}

		//Branch Instructions
		{
			OpTable[0x90] = { {'B', 'C', 'C', '\0'}, 0x90, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Carry) == 0)
					{
						CPU->CycleRemain++;
						CPU->ProgramCounter++;
						Int8 Offset = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
					else
					{
						CPU->ProgramCounter += 2;
					}
				} }; //Branch on Carry Clear (BCC)
			OpTable[0xB0] = { {'B', 'C', 'S', '\0'}, 0xB0, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Carry) == 1)
					{
						CPU->CycleRemain++;
						CPU->ProgramCounter++;
						Int8 Offset = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
					else
					{
						CPU->ProgramCounter += 2;
					}
				} }; //Branch on Carry Set (BCS)
			OpTable[0xF0] = { {'B', 'E', 'Q', '\0'}, 0xF0, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Zero) == 1)
					{
						CPU->CycleRemain++;
						CPU->ProgramCounter++;
						Int8 Offset = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
					else
					{
						CPU->ProgramCounter += 2;
					}
				} }; //Branch on Result Zero (BEQ)
			OpTable[0x30] = { {'B', 'M', 'I', '\0'}, 0x30, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Negative) == 1)
					{
						CPU->CycleRemain++;
						CPU->ProgramCounter++;
						Int8 Offset = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
					else
					{
						CPU->ProgramCounter += 2;
					}
				} }; //Branch on Results Minus (BMI)
			OpTable[0xD0] = { {'B', 'N', 'E', '\0'}, 0xD0, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Zero) == 0)
					{
						CPU->CycleRemain++;
						CPU->ProgramCounter++;
						Int8 Offset = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
					else
					{
						CPU->ProgramCounter += 2;
					}
				} }; //Branch on Result Not Zero (BNE)
			OpTable[0x10] = { {'B', 'P', 'L', '\0'}, 0x10, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Negative) == 0)
					{
						CPU->CycleRemain++;
						CPU->ProgramCounter++;
						Int8 Offset = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
					else
					{
						CPU->ProgramCounter += 2;
					}
				} }; //Branch on Result Plus (BPL)
			OpTable[0x50] = { {'B', 'V', 'C', '\0'}, 0x50, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Overflow) == 0)
					{
						CPU->CycleRemain++;
						CPU->ProgramCounter++;
						Int8 Offset = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
					else
					{
						CPU->ProgramCounter += 2;
					}
				} }; //Branch on Overflow Clear (BVC)
			OpTable[0x70] = { {'B', 'V', 'S', '\0'}, 0x70, Relative, 2, 2, [](NesCPU* CPU)
				{
					if (CPU->GetFlag(Overflow) == 1)
					{
						CPU->CycleRemain++;
						CPU->ProgramCounter++;
						Int8 Offset = CPU->ReadRAM(CPU->ProgramCounter);
						CPU->ProgramCounter++;
						UInt16 NewAddress = CPU->ProgramCounter + Offset;

						if ((NewAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
						{
							CPU->CycleRemain++;
						}

						CPU->ProgramCounter = NewAddress;
					}
					else
					{
						CPU->ProgramCounter += 2;
					}
				} }; //Branch on Overflow Set (BVS)
		}

		//Jump & Subroutines Instructions
		{
			OpTable[0x4C] = { {'J', 'M', 'P', '\0'}, 0x4C, Absolute, 3, 3, [](NesCPU* CPU) 
				{
					CPU->ProgramCounter++;
					UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);

					UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
					CPU->ProgramCounter = MemoryAddress;
				} };
			OpTable[0x6C] = { {'J', 'M', 'P', '\0'}, 0x6C, Indirect, 3, 5, [](NesCPU* CPU)
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
			OpTable[0x20] = { {'J', 'S', 'R', '\0'}, 0x20, Absolute, 3, 6, [](NesCPU* CPU) 
				{
					CPU->ProgramCounter++;
					UInt8 LowAddressByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddressByte = CPU->ReadRAM(CPU->ProgramCounter);

					CPU->WriteRAM(0x0100 + CPU->StackPointer, (CPU->ProgramCounter >> 8) & 0x00FF);
					CPU->StackPointer--;
					CPU->WriteRAM(0x0100 + CPU->StackPointer, CPU->ProgramCounter & 0x00FF);
					CPU->StackPointer--;

					UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
					CPU->ProgramCounter = MemoryAddress;
				} };
			OpTable[0x60] = { {'R', 'T', 'S', '\0'}, 0x60, Implied, 1, 6, [](NesCPU* CPU) 
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

		//Interrupt Instructions
		{
			OpTable[0x00] = { {'B', 'R', 'K', '\0'}, 0x00, Implied, 1, 7, [](NesCPU* CPU)
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
			} }; //Break Command (BRK)
			OpTable[0x40] = { {'R', 'T', 'I', '\0'}, 0x40, Implied, 1, 6, [](NesCPU* CPU) 
				{
					CPU->StackPointer++;
					CPU->StatusFlags = CPU->ReadRAM(0x0100 + CPU->StackPointer);

					CPU->StackPointer++;
					UInt8 LowAddressByte = CPU->ReadRAM(0x0100 + CPU->StackPointer);
					CPU->StackPointer++;
					UInt8 HighAddressByte = CPU->ReadRAM(0x0100 + CPU->StackPointer);

					UInt16 MemoryAddress = (HighAddressByte << 8) | LowAddressByte;
					CPU->ProgramCounter = MemoryAddress;
				} }; //Return From Interrupt (RTI)
		}

		//Other Instructions
		{
			//NOP - No Operation
			{
				OpTable[0xEA] = { {'N', 'O', 'P', '\0' }, 0xEA, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
				} }; // Official
				OpTable[0x1A] = { {'N', 'O', 'P', '\0' }, 0x1A, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0x3A] = { {'N', 'O', 'P', '\0' }, 0x3A, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0x5A] = { {'N', 'O', 'P', '\0' }, 0x5A, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0x7A] = { {'N', 'O', 'P', '\0' }, 0x7A, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0xDA] = { {'N', 'O', 'P', '\0' }, 0xDA, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0xFA] = { {'N', 'O', 'P', '\0' }, 0xFA, Implied, 1, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0x80] = { {'N', 'O', 'P', '\0' }, 0x80, Immediate, 2, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0x82] = { {'N', 'O', 'P', '\0' }, 0x82, Immediate, 2, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0x89] = { {'N', 'O', 'P', '\0' }, 0x89, Immediate, 2, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0xC2] = { {'N', 'O', 'P', '\0' }, 0xC2, Immediate, 2, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0xE2] = { {'N', 'O', 'P', '\0' }, 0xE2, Immediate, 2, 2, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0x0C] = { {'N', 'O', 'P', '\0' }, 0x0C, Absolute, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 3;
				} }; // Unoffical
				OpTable[0x1C] = { {'N', 'O', 'P', '\0' }, 0x1C, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;

					if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
					{
						CPU->CycleRemain++;
					}

					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0x3C] = { {'N', 'O', 'P', '\0' }, 0x3C, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;

					if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
					{
						CPU->CycleRemain++;
					}

					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0x5C] = { {'N', 'O', 'P', '\0' }, 0x5C, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;

					if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
					{
						CPU->CycleRemain++;
					}

					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0x7C] = { {'N', 'O', 'P', '\0' }, 0x7C, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;

					if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
					{
						CPU->CycleRemain++;
					}

					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0xDC] = { {'N', 'O', 'P', '\0' }, 0xDC, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;

					if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
					{
						CPU->CycleRemain++;
					}

					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0xFC] = { {'N', 'O', 'P', '\0' }, 0xFC, AbsoluteIndexedX, 3, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter++;
					UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					CPU->ProgramCounter++;
					UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
					UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;

					if ((MemoryAddress & 0xFF00) != (CPU->ProgramCounter & 0xFF00))
					{
						CPU->CycleRemain++;
					}

					CPU->ProgramCounter++;
				} }; // Unoffical
				OpTable[0x04] = { {'N', 'O', 'P', '\0' }, 0x04, ZeroPage, 2, 3, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0x44] = { {'N', 'O', 'P', '\0' }, 0x44, ZeroPage, 2, 3, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0x64] = { {'N', 'O', 'P', '\0' }, 0x64, ZeroPage, 2, 3, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0x14] = { {'N', 'O', 'P', '\0' }, 0x14, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0x34] = { {'N', 'O', 'P', '\0' }, 0x34, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0x54] = { {'N', 'O', 'P', '\0' }, 0x54, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0x74] = { {'N', 'O', 'P', '\0' }, 0x74, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0xD4] = { {'N', 'O', 'P', '\0' }, 0xD4, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
				OpTable[0xF4] = { {'N', 'O', 'P', '\0' }, 0xF4, ZeroPageIndexX, 2, 4, [](NesCPU* CPU)
				{
					CPU->ProgramCounter += 2;
				} }; // Unoffical
			}

			//JAM - Halt the CPU
			{
				OpTable[0x02] = { {'J', 'A', 'M', '\0'}, 0x02, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0x12] = { {'J', 'A', 'M', '\0'}, 0x12, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0x22] = { {'J', 'A', 'M', '\0'}, 0x22, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0x32] = { {'J', 'A', 'M', '\0'}, 0x32, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0x42] = { {'J', 'A', 'M', '\0'}, 0x42, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0x52] = { {'J', 'A', 'M', '\0'}, 0x52, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0x62] = { {'J', 'A', 'M', '\0'}, 0x62, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0x72] = { {'J', 'A', 'M', '\0'}, 0x72, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0x92] = { {'J', 'A', 'M', '\0'}, 0x92, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0xB2] = { {'J', 'A', 'M', '\0'}, 0xB2, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0xD2] = { {'J', 'A', 'M', '\0'}, 0xD2, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
				OpTable[0xF2] = { {'J', 'A', 'M', '\0'}, 0xF2, Implied, 1, 0, [](NesCPU* CPU)
					{
						CPU->Jam = true;
					} };
			}

			OpTable[0x24] = { {'B', 'I', 'T', '\0' }, 0x24, ZeroPage, 2, 3, [](NesCPU* CPU)
			{
				CPU->ProgramCounter++;

				UInt8 MemoryAddress = CPU->ReadRAM(CPU->ProgramCounter);
				UInt8 TestByte = CPU->ReadRAM(MemoryAddress);

				CPU->ProgramCounter++;

				CPU->SetFlag(Zero, (((CPU->Accumulator & TestByte) & 0x00FF) == 0)); //Store the results of our AND operation.
				CPU->SetFlag(Overflow, TestByte & (1 << 6));
				CPU->SetFlag(Negative, TestByte & (1 << 7));
			} }; //Bit Test (Zero Page)
			OpTable[0x2C] = { {'B', 'I', 'T', '\0' }, 0x2C, Absolute, 3, 4, [](NesCPU* CPU)
			{
				CPU->ProgramCounter++;
				UInt8 LowAddresByte = CPU->ReadRAM(CPU->ProgramCounter);
				CPU->ProgramCounter++;
				UInt8 HighAddresByte = CPU->ReadRAM(CPU->ProgramCounter);

				UInt16 MemoryAddress = (HighAddresByte << 8) | LowAddresByte;
				UInt8 TestByte = CPU->ReadRAM(MemoryAddress);
				CPU->ProgramCounter++;

				CPU->SetFlag(Zero, (((CPU->Accumulator& TestByte) & 0x00FF) == 0)); //Store the results of our AND operation.
				CPU->SetFlag(Overflow, TestByte& (1 << 6));
				CPU->SetFlag(Negative, TestByte& (1 << 7));

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
		System->CPUWrite(Address, Data);
	}
	UInt8 NesCPU::ReadRAM(UInt16 Address)
	{
		return System->CPURead(Address);
	}

	void NesCPU::Clock()
	{
		if (CycleRemain == 0 && Jam == false)
		{
			Fetch();

			if (CurrOp->OpFunc != nullptr)
			{
				CurrOp->OpFunc(this);
			}

			SetFlag(Unsed, 1);
		}

		if (Jam == false)
		{
			CycleRemain--;
			CycleCounter++;
		}
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
		StackPointer = 0xFD; //Sets the stack pointer, testing only, this gets set in software.
		ProgramCounter = (HighByte << 8) | LowByte; //Start Location.

		//Set our cycles remaining.
		CycleRemain = 7;
	}
	void NesCPU::NMI()
	{
		//Get our address from the reset vector.
		UInt8 LowByte = ReadRAM(0xFFFA);
		UInt8 HighByte = ReadRAM(0xFFFB);

		UInt8 temp = ReadRAM(ProgramCounter);
		ProgramCounter++;

		SetFlag(Interrupt, true);
		WriteRAM(0x0100 + StackPointer, (ProgramCounter >> 8) & 0x00FF);
		StackPointer--;
		WriteRAM(0x0100 + StackPointer, ProgramCounter & 0x00FF);
		StackPointer--;

		SetFlag(Unsed, true);
		WriteRAM(0x0100 + StackPointer, StatusFlags);
		StackPointer--;

		ProgramCounter = (HighByte << 8) | LowByte; //Start Location.
		CycleRemain = 8;
	}
	void NesCPU::IRQ()
	{
		if (GetFlag(Interrupt) == 0)
		{
			//Get our address from the reset vector.
			UInt8 LowByte = ReadRAM(0xFFFE);
			UInt8 HighByte = ReadRAM(0xFFFF);

			UInt8 temp = ReadRAM(ProgramCounter);
			ProgramCounter++;

			SetFlag(Interrupt, true);
			WriteRAM(0x0100 + StackPointer, (ProgramCounter >> 8) & 0x00FF);
			StackPointer--;
			WriteRAM(0x0100 + StackPointer, ProgramCounter & 0x00FF);
			StackPointer--;

			SetFlag(Unsed, true);
			SetFlag(Break, false);
			WriteRAM(0x0100 + StackPointer, StatusFlags);
			StackPointer--;

			ProgramCounter = (HighByte << 8) | LowByte; //Start Location.
			CycleRemain = 7;
		}
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