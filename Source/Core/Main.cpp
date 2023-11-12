#include "Application.h"
#include "optick.h"

int main(int argc, char** argv)
{
	OPTICK_START_CAPTURE();
	NesEmulator::Log::Init();

	NesEmulator::Application TestEmu;
	TestEmu.Run();

	NesEmulator::Log::Shutdown();
	OPTICK_STOP_CAPTURE();
	OPTICK_SAVE_CAPTURE("Caputre.opt");
	return 0;
}