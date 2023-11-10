#include "Application.h"

namespace NesEmulator
{
	Application* Application::Instance = nullptr;

	Application::Application() : MainWindow(nullptr), Config(nullptr)
	{
		if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
		{
			//Get our configuration setings.
			std::ifstream SettingsFile;
			SettingsFile.open("settings.json", std::ifstream::in);	

			if (!SettingsFile.is_open())
			{
				SettingsFile.open("settings.json", std::ifstream::in | std::ifstream::out | std::ifstream::app);
				Config = new ConfigData
				{
					.WorkingDirectory = SDL_GetBasePath()
				};
				SettingsFile.close();
			}
			else
			{
				LoadConfigSettings(&SettingsFile);
				SettingsFile.close();
			}

			//Create our window.
			this->Instance = this;
			MainWindow = SDL_CreateWindow(WindowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WindowWidth, WindowHeight, WindowFlags);

			CORE_TRACE("SDL Initalized!");
			CORE_TRACE("Window Created! \n		Title: {}\n		Width: {}\n		Height: {}", WindowTitle, WindowWidth, WindowHeight);

			if (NFD_Init() != NFD_OKAY) 
			{
				CORE_TRACE("NativeFileDialog failed to Initalized!");
			}

			GraphicsSystem.InitalizeRenderer(MainWindow, Diligent::RENDER_DEVICE_TYPE_D3D12);
			GuiLayer.ImGuiCreate(GraphicsSystem.GetDevice(), GraphicsSystem.GetSwapChain());
			NesMachine.InsertCartridge("F:/OmegaGamingHunters Folder/TestNES Emulator/Assets/Programs/nestest.nes");
		}
	}
	Application::~Application()
	{
		GuiLayer.ImGuiDestroy();
		NFD_Quit();
		SDL_Quit();
	}

	void Application::LoadConfigSettings(std::istream* File)
	{
		//nlohmann::json Settings = nlohmann::json::parse(File);
	}

	Application& Application::Get()
	{
		return *Instance;
	}

	void Application::Run()
	{
		while (ProgramLoop)
		{
			//Setup.
			HandleEvents();
			GraphicsSystem.ClearScreen();
			GuiLayer.BeginFrame(MainWindow, GraphicsSystem.GetDevice(), GraphicsSystem.GetSwapChain());

			//Run the Emulator
			while (!NesMachine.GetPPU()->FrameComplete)
			{
				NesMachine.Clock();
			}

			NesMachine.UpdateVideoOutput(GraphicsSystem.GetDevice(), GraphicsSystem.GetContext());
			NesMachine.GetPPU()->FrameComplete = false;
			NesMachine.GetPPU()->UpdateDebugPatternTable(GraphicsSystem.GetDevice());

			//Render the GUI
			UpdateUI();

			//Push results to the screen.
			GuiLayer.EndFrame();
			GraphicsSystem.RenderImGui(GuiLayer.GetRenderData(), ImGui::GetDrawData());
			GraphicsSystem.Present();
		}
	}
	void Application::UpdateUI()
	{
		//Main Menu Bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem(ICON_FA_FILE " Open"))
				{
					nfdchar_t* outPath;
					nfdfilteritem_t filterItem[1] = { { "iNES Rom", "nes" } };
					nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);

					if (result == NFD_OKAY)
					{
						NesMachine.InsertCartridge(outPath);
						NFD_FreePath(outPath);
					}
				}

				if (ImGui::MenuItem(ICON_FA_TIMES_CIRCLE " Close"))
				{

				}

				ImGui::Separator();
				
				if (ImGui::MenuItem("Exit"))
				{
					ProgramLoop = false;
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("NES"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Settings"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About"))
				{
					ImGui::ShowAboutWindow();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		//Draw our output from the emulator.
		NesMachine.DrawVideo();

		//Draw everything else.
		NesMachine.GetPPU()->DrawRegisters();
		NesMachine.GetPPU()->DrawPatternTable();
		NesMachine.GetCPU()->DrawRegisters();
	}
	void Application::HandleEvents()
	{
		SDL_Event Event;

		while (SDL_PollEvent(&Event))
		{
			GuiLayer.ProcessEvents(&Event);

			switch (Event.type)
			{
				case SDL_KEYDOWN:
				{
					if (Event.key.keysym.scancode == SDL_GetScancodeFromKey(SDLK_z))
					{
						
					}
					else if (Event.key.keysym.scancode == SDL_GetScancodeFromKey(SDLK_x))
					{
						
					}

					break;
				}
				case SDL_WINDOWEVENT:
				{
					if (Event.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						GraphicsSystem.GetSwapChain()->Resize(Event.window.data1, Event.window.data2);
					}

					break;
				}
				case SDL_QUIT:
				{
					ProgramLoop = false;
					break;
				}
			}
		}
	}
}