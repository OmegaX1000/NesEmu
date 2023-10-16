#pragma once
#include "catch_amalgamated.hpp"
#include "NesConsole.h"

TEST_CASE("6502 CPU Instructions")
{
	NesEmulator::NesConsole TestConsole;

	for (int i = 0; i < 7; i++)
	{
		TestConsole.GetCPU()->Clock();
	}

	SECTION("Instruction: NOP")
	{
		//Setup
		UInt16 StartIP = TestConsole.GetCPU()->ProgramCounter;
		TestConsole.WriteRAM(0x8000, 0xEA);

		//Execute
		TestConsole.GetCPU()->Clock();

		//Results
		REQUIRE(!(StartIP == TestConsole.GetCPU()->ProgramCounter));
		TestConsole.GetCPU()->ProgramCounter = 0x8000;
		TestConsole.GetCPU()->CycleRemain = 0;
	};
	SECTION("Instruction: CLC")
	{
		//Setup
		UInt16 StartIP = TestConsole.GetCPU()->ProgramCounter;
		TestConsole.WriteRAM(0x8000, 0x18);

		//Execute
		TestConsole.GetCPU()->Clock();

		//Results
		REQUIRE(TestConsole.GetCPU()->GetFlag(TestConsole.GetCPU()->Carry) == 0);
		TestConsole.GetCPU()->ProgramCounter = 0x8000;
		TestConsole.GetCPU()->CycleRemain = 0;
	};
	SECTION("Instruction: CLD")
	{
		//Setup
		UInt16 StartIP = TestConsole.GetCPU()->ProgramCounter;
		TestConsole.WriteRAM(0x8000, 0xD8);

		//Execute
		TestConsole.GetCPU()->Clock();

		//Results
		REQUIRE(TestConsole.GetCPU()->GetFlag(TestConsole.GetCPU()->Decimal) == 0);
		TestConsole.GetCPU()->ProgramCounter = 0x8000;
		TestConsole.GetCPU()->CycleRemain = 0;
	};
	SECTION("Instruction: CLI")
	{
		//Setup
		UInt16 StartIP = TestConsole.GetCPU()->ProgramCounter;
		TestConsole.WriteRAM(0x8000, 0x58);

		//Execute
		TestConsole.GetCPU()->Clock();

		//Results
		REQUIRE(TestConsole.GetCPU()->GetFlag(TestConsole.GetCPU()->Interrupt) == 0);
		TestConsole.GetCPU()->ProgramCounter = 0x8000;
		TestConsole.GetCPU()->CycleRemain = 0;
	};
	SECTION("Instruction: CLV")
	{
		//Setup
		UInt16 StartIP = TestConsole.GetCPU()->ProgramCounter;
		TestConsole.WriteRAM(0x8000, 0xB8);

		//Execute
		TestConsole.GetCPU()->Clock();

		//Results
		REQUIRE(TestConsole.GetCPU()->GetFlag(TestConsole.GetCPU()->Overflow) == 0);
		TestConsole.GetCPU()->ProgramCounter = 0x8000;
		TestConsole.GetCPU()->CycleRemain = 0;
	};
	SECTION("Instruction: SEC")
	{
		//Setup
		UInt16 StartIP = TestConsole.GetCPU()->ProgramCounter;
		TestConsole.WriteRAM(0x8000, 0x38);

		//Execute
		TestConsole.GetCPU()->Clock();

		//Results
		REQUIRE(TestConsole.GetCPU()->GetFlag(TestConsole.GetCPU()->Carry) == 1);
		TestConsole.GetCPU()->ProgramCounter = 0x8000;
		TestConsole.GetCPU()->CycleRemain = 0;
	};
	SECTION("Instruction: SED")
	{
		//Setup
		UInt16 StartIP = TestConsole.GetCPU()->ProgramCounter;
		TestConsole.WriteRAM(0x8000, 0xF8);

		//Execute
		TestConsole.GetCPU()->Clock();

		//Results
		REQUIRE(TestConsole.GetCPU()->GetFlag(TestConsole.GetCPU()->Decimal) == 1);
		TestConsole.GetCPU()->ProgramCounter = 0x8000;
		TestConsole.GetCPU()->CycleRemain = 0;
	};
	SECTION("Instruction: SEI")
	{
		//Setup
		UInt16 StartIP = TestConsole.GetCPU()->ProgramCounter;
		TestConsole.WriteRAM(0x8000, 0x78);

		//Execute
		TestConsole.GetCPU()->Clock();

		//Results
		REQUIRE(TestConsole.GetCPU()->GetFlag(TestConsole.GetCPU()->Interrupt) == 1);
		TestConsole.GetCPU()->ProgramCounter = 0x8000;
		TestConsole.GetCPU()->CycleRemain = 0;
	};

	TestConsole.GetCPU()->StatusFlags = 0;
}