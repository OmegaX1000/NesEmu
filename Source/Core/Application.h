#pragma once
#include "Definitions.h"
#include "RenderSystem.h"
#include "SDL.h"
#include "Log.h"
#include "NesConsole.h"

#include "nfd.h"

namespace NesEmulator
{
	class Application
	{
		private:
			//Application Properties.
			NesConsole NesMachine;
			RenderSystem GraphicsSystem;
			ImGuiLayer GuiLayer;
			bool ProgramLoop = true;

			//Window Properties.
			SDL_Window* MainWindow;
			std::string WindowTitle = "NES Emulator";
			int WindowWidth = 640;
			int WindowHeight = 480;
			UInt32 WindowFlags = SDL_WINDOW_RESIZABLE;

		public:

			//Constructor & Destructor.
			Application();
			~Application();

			void Run();
			void UpdateUI();
			void HandleEvents();
	};
}