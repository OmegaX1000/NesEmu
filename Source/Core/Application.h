#pragma once
#include "Definitions.h"
#include "RenderSystem.h"
#include "SDL.h"
#include "Log.h"
#include "NesConsole.h"

#include "nfd.h"
#include "nlohmann/json.hpp"

namespace NesEmulator
{
	//Holds configuration data for the app like directories and settings.
	struct ConfigData
	{
		//Directories
		std::string WorkingDirectory;
		std::string FontDirectory;
		std::string PaletteDirectory;
		std::string RomDirectory;
		std::string SaveDirectories;
		std::string ScreenshotDirectory;
	};

	//Holds the entire application.
	class Application
	{
		public:
			RenderSystem GraphicsSystem;
			ImGuiLayer GuiLayer;
			ConfigData* Config;

		private:

			static Application* Instance;

			//Application Properties.
			NesConsole NesMachine;
			bool ProgramLoop = true;

			void LoadConfigSettings(std::istream* FileString);

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