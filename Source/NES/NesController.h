#pragma once
#include "Definitions.h"
#include <string>

namespace NesEmulator
{
	class NesController
	{
		public:
			enum ControllerType
			{
				NoController = 0x00,
				StandardController = 0x01, //The standard controller, one included with every NES.
				NumOfControllers
			};
			enum InputType
			{
				NoInput = 0x00,
				Keyboard = 0x01,
				Gamepad = 0x02,
				NumOfInputDevices
			};

		private:

			union ControllerLatch
			{
				struct
				{
					UInt8 Unused : 5;
					UInt8 ControlPort : 1;
					UInt8 ExpansionPort : 2;
				};

				UInt8 Register = 0x00;
			};
			union DataLine
			{
				struct
				{
					UInt8 Data : 5;
					UInt8 Unused : 3;
				};

				UInt8 Register = 0x00;
			};

			//General Variables
			ControllerType Type;
			InputType Device;
			ControllerLatch WriteBuffer;
			DataLine ReadBuffer;

		public:

			//Constructor & Destructor
			NesController();
			~NesController();

			//Controller Specific variables
			UInt8 StandardControllerButtons = 0x00;

			ControllerType GetType();
			std::string_view GetTypeName();
			std::string_view GetTypeName(ControllerType ID);
			InputType GetInputDevice();
			std::string_view GetInputName();
			std::string_view GetInputName(InputType ID);
			ControllerLatch* GetWriteBuffer();
			DataLine* GetReadBuffer();

			void SwitchController(ControllerType NewGamepad);
			void SwitchInputDevice(InputType NewInput);
			void ControllerWrite(UInt8 Data);
			void ControllerRead(UInt8 &Data);
	};
}