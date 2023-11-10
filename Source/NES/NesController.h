#pragma once

namespace NesEmulator
{
	class NesController
	{
		public:
			enum ControllerType
			{
				StandardController = 0x00 //The standard controller, one included with every NES.
			};

		private:

			ControllerType Type;

		public:

			//Constructor & Destructor
			NesController();
			~NesController();

			ControllerType GetType();
	};
}