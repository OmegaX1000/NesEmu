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
			}
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

				CPU->SetFlag(Zero, !(CPU->Accumulator & TestByte)); //Store the results of our AND operation.
				CPU->SetFlag(Overflow, ((TestByte >> 6) & 0x01));
				CPU->SetFlag(Negative, ((TestByte >> 7) & 0x01));
			} }; //Bit Test (Zero Page)
			OpTable[0x2C] = { { 'B', 'I', 'T', '\0' }, 0x2C, Absolute, 3, 4, [](NesCPU* CPU)
			{
				CPU->ProgramCounter++;
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