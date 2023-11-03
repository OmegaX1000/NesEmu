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
		public:
			RenderSystem GraphicsSystem;
			ImGuiLayer GuiLayer;

		private:

			static Application* Instance;

			//Application Properties.
			NesConsole NesMachine;
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

			static Application& Get();

			void Run();
			void UpdateUI();
			void HandleEvents();
	};
}