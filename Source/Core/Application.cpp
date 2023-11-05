#include "Application.h"

namespace NesEmulator
{
	Application* Application::Instance = nullptr;

	Application::Application() : MainWindow(nullptr)
	{
		if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
		{
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

	Application& Application::Get()
	{
		return *Instance;
	}

	void Application::Run()
	{
		while (ProgramLoop)
		{
			HandleEvents();
			GuiLayer.BeginFrame(MainWindow, GraphicsSystem.GetDevice(), GraphicsSystem.GetSwapChain());

			//for (int i = 0; i < 100; i++)
			{
				//NesMachine.Clock(GraphicsSystem.GetDevice());
			}

			UpdateUI();

			GuiLayer.EndFrame();
			GraphicsSystem.RenderImGui(GuiLayer.GetRenderData(), ImGui::GetDrawData());
			GraphicsSystem.Present();
		}
	}
	void Application::UpdateUI()
	{
		ImGui::DockSpaceOverViewport();

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

				ImGui::Separator();
				
				if (ImGui::MenuItem("Exit"))
				{
					ProgramLoop = false;
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About"))
				{

				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		NesMachine.GetPPU()->DrawVideo();
		NesMachine.GetPPU()->DrawRegisters();
		NesMachine.GetPPU()->DrawPatternTable();
		NesMachine.GetPPU()->DrawNametable();
		NesMachine.GetPPU()->DrawPalettes();
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
						for (int i = 0; i < 341; i++)
						{
							NesMachine.Clock(GraphicsSystem.GetDevice());
						}

						NesMachine.GetPPU()->UpdateDebugPatternTable(GraphicsSystem.GetDevice());
						//NesMachine.GetCPU()->Clock();
					}
					else if (Event.key.keysym.scancode == SDL_GetScancodeFromKey(SDLK_x))
					{
						NesMachine.Clock(GraphicsSystem.GetDevice());
						NesMachine.GetPPU()->UpdateDebugPatternTable(GraphicsSystem.GetDevice());
						//NesMachine.GetCPU()->Clock();
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