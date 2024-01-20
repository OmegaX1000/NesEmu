#include "NesController.h"

namespace NesEmulator
{
	NesController::NesController() : Type(ControllerType::NoController), Device(InputType::NoInput)
	{
		StandardControllerKeyboard[0] = ImGuiKey_Z;
		StandardControllerKeyboard[1] = ImGuiKey_X;
		StandardControllerKeyboard[2] = ImGuiKey_A;
		StandardControllerKeyboard[3] = ImGuiKey_S;
		StandardControllerKeyboard[4] = ImGuiKey_UpArrow;
		StandardControllerKeyboard[5] = ImGuiKey_DownArrow;
		StandardControllerKeyboard[6] = ImGuiKey_LeftArrow;
		StandardControllerKeyboard[7] = ImGuiKey_RightArrow;

		std::fill_n(StandardControllerGamepad, 8, ImGuiKey_None);
	}
	NesController::~NesController()
	{

	}

	NesController::ControllerType NesController::GetType()
	{
		return Type;
	}
	std::string_view NesController::GetTypeName()
	{
		switch (Type)
		{
			case NoController: return "None"; break;
			case StandardController: return "Standard Controller"; break;
			default: return "Unknown Type"; break;
		}
	}
	std::string_view NesController::GetTypeName(ControllerType ID)
	{
		switch (ID)
		{
			case NoController: return "None"; break;
			case StandardController: return "Standard Controller"; break;
			default: return "Unknown Type"; break;
		}
	}
	NesController::InputType NesController::GetInputDevice()
	{
		return Device;
	}
	std::string_view NesController::GetInputName()
	{
		switch (Device)
		{
			case NoInput: return "None"; break;
			case Keyboard: return "Keyboard"; break;
			case Gamepad: return "Gamepad"; break;
			default: return "Unknown Type"; break;
		}
	}
	std::string_view NesController::GetInputName(InputType ID)
	{
		switch (ID)
		{
			case NoInput: return "None"; break;
			case Keyboard: return "Keyboard"; break;
			case Gamepad: return "Gamepad"; break;
			default: return "Unknown Type"; break;
		}
	}
	NesController::ControllerLatch* NesController::GetWriteBuffer()
	{
		return &WriteBuffer;
	}
	NesController::DataLine* NesController::GetReadBuffer()
	{
		return &ReadBuffer;
	}

	std::string_view NesController::GetButtonName(UInt8 ButtonID)
	{
		switch (Type)
		{
			case StandardController:
			{
				switch (ButtonID)
				{
					case 0: return "A"; break;
					case 1: return "B"; break;
					case 2: return "Select"; break;
					case 3: return "Start"; break;
					case 4: return "Up"; break;
					case 5: return "Down"; break;
					case 6: return "Left"; break;
					case 7: return "Right"; break;
				}

				break;
			}
		}

		return "";
	}

	void NesController::SwitchController(ControllerType NewGamepad)
	{
		Type = NewGamepad;
	}
	void NesController::SwitchInputDevice(InputType NewInput)
	{
		Device = NewInput;
	}
	void NesController::ControllerWrite(UInt8 Data)
	{
		switch (Type)
		{
			case StandardController:
			{
				WriteBuffer.Register = Data;
				WriteBuffer.ControlPort = (Data & 0x01) ? 1 : 0;
				break;
			}
			default:
			{
				break;
			}
		}
	}
	void NesController::ControllerRead(UInt8& Data)
	{
		switch (Type)
		{
			case StandardController:
			{
				Data = StandardControllerButtons & 0x01;
				StandardControllerButtons >>= 1;
				break;
			}
			default:
			{
				break;
			}
		}
	}
}