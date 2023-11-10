#include "NesController.h"

namespace NesEmulator
{
	NesController::NesController()
	{

	}
	NesController::~NesController()
	{

	}

	NesController::ControllerType NesController::GetType()
	{
		return Type;
	}
}