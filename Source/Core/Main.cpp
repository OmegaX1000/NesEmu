#include "Application.h"

int main(int argc, char** argv)
{
	NesEmulator::Log::Init();

	NesEmulator::Application TestEmu;
	TestEmu.Run();

	NesEmulator::Log::Shutdown();
	return 0;
}