#include "Application.h"
#include "optick.h"

namespace NesEmulator
{
	Application* Application::Instance = nullptr;

	Application::Application() : MainWindow(nullptr), Config(nullptr)
	{
		OPTICK_EVENT("Application Init");

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
		else
		{
			SDL_Quit();
		}
	}
	Application::~Application()
	{
		OPTICK_EVENT("Application Exit");
		GuiLayer.ImGuiDestroy();

		NFD_Quit();
		SDL_Quit();
	}

	void Application::LoadConfigSettings(std::istream* File)
	{
		OPTICK_EVENT();
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
			OPTICK_FRAME("MainThread");

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
		OPTICK_EVENT();

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
				if (ImGui::MenuItem("Input"))
				{
					ShowInputConfig = true;
				}

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
		InputConfiguration();
		
		NesMachine.GetPPU()->DrawRegisters();
		NesMachine.GetPPU()->DrawPatternTable();
		NesMachine.GetCPU()->DrawRegisters();
	}
	void Application::HandleEvents()
	{
		OPTICK_EVENT();
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
						for (int i = 0; i < 79662; i++)
						{
							//NesMachine.Clock();
						}
					}
					else if (Event.key.keysym.scancode == SDL_GetScancodeFromKey(SDLK_x))
					{
						for (int i = 0; i < 100; i++)
						{
							//NesMachine.Clock();
						}
					}
					else if (Event.key.keysym.scancode == SDL_GetScancodeFromKey(SDLK_c))
					{
						//NesMachine.Clock();
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
					SDL_DestroyWindow(MainWindow);
					ProgramLoop = false;
					break;
				}
			}
		}
	}

	void Application::InputConfiguration()
	{
		if (ShowInputConfig)
		{
			ImGui::Begin("Input Configuration", &ShowInputConfig);
			NesMachine.TurnoffPolling();

			ImGui::BeginGroup();
			ImGui::Text("NES Controller Port #1");	
			ImGui::Text("Controller Type: ");
			ImGui::SetNextItemWidth(200.0f);
			if (ImGui::BeginCombo("##ControllerOne", NesMachine.GetControllerOne()->GetTypeName().data()))
			{
				for (int i = 0; i < NesController::ControllerType::NumOfControllers; i++)
				{
					if (ImGui::Selectable(NesMachine.GetControllerOne()->GetTypeName((NesController::ControllerType)i).data()))
					{
						NesMachine.GetControllerOne()->SwitchController((NesController::ControllerType)i);
					}
				}
				ImGui::EndCombo();
			}
			ImGui::Text("Input Device: ");
			ImGui::SetNextItemWidth(200.0f);
			if (ImGui::BeginCombo("##InputOne", NesMachine.GetControllerOne()->GetInputName().data()))
			{
				for (int i = 0; i < NesController::InputType::NumOfInputDevices; i++)
				{
					if (ImGui::Selectable(NesMachine.GetControllerOne()->GetInputName((NesController::InputType)i).data()))
					{
						NesMachine.GetControllerOne()->SwitchInputDevice((NesController::InputType)i);
					}
				}

				ImGui::EndCombo();
			}
			
			//Button Mapping
			switch (NesMachine.GetControllerOne()->GetType())
			{
				case NesController::StandardController:
				{
					ImGui::BeginGroup();
					ImGui::Text("Button: ");
					ImGui::SetNextItemWidth(100.0f);

					static UInt8 StandardButtonPreview = 0;
					static bool KeyLookingForInput = false;

					if (ImGui::BeginCombo("##StandardButtons", NesMachine.GetControllerOne()->GetButtonName(StandardButtonPreview).data()))
					{
						for (int i = 0; i < 8; i++)
						{
							if (ImGui::Selectable(NesMachine.GetControllerOne()->GetButtonName(i).data()))
							{
								StandardButtonPreview = i;
								KeyLookingForInput = false;
							}
						}

						ImGui::EndCombo();
					}

					ImGui::EndGroup();
					ImGui::SameLine();
					ImGui::BeginGroup();

					if (NesMachine.GetControllerOne()->GetInputDevice() == NesController::Keyboard)
					{
						ImGui::Text("Key: ");
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
						ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.1f, 0.5f));
						
						if (ImGui::Selectable(ImGui::GetKeyName(NesMachine.GetControllerOne()->StandardControllerKeyboard[StandardButtonPreview]), KeyLookingForInput, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100.0f, 18.0f)))
						{
							KeyLookingForInput = !KeyLookingForInput;
						}

						if (KeyLookingForInput)
						{
							for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
							{
								if (ImGui::IsKeyDown(key) && ImGui::IsKeyboardKey(key))
								{
									NesMachine.GetControllerOne()->StandardControllerKeyboard[StandardButtonPreview] = key;
									KeyLookingForInput = false;
									break;
								}
							}
						}

						ImGui::PopStyleVar(2);
					}
					else if (NesMachine.GetControllerOne()->GetInputDevice() == NesController::Gamepad)
					{
						ImGui::Text("Button: ");
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
						ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.1f, 0.5f));

						if (ImGui::Selectable(ImGui::GetKeyName(NesMachine.GetControllerOne()->StandardControllerGamepad[StandardButtonPreview]), KeyLookingForInput, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100.0f, 18.0f)))
						{
							KeyLookingForInput = !KeyLookingForInput;
						}

						if (KeyLookingForInput)
						{
							for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
							{
								if (ImGui::IsKeyDown(key) && ImGui::IsGamepadKey(key))
								{
									NesMachine.GetControllerOne()->StandardControllerGamepad[StandardButtonPreview] = key;
									KeyLookingForInput = false;
									break;
								}
							}
						}

						ImGui::PopStyleVar(2);
					}
					else
					{
						ImGui::Text("Binding: ");
						ImGui::SetNextItemWidth(100.0f);
						
						if (ImGui::BeginCombo("##NullBindings", nullptr))
						{
							ImGui::EndCombo();
						}
					}

					ImGui::EndGroup();
					break;
				}
				default:
				{
					ImGui::BeginGroup();
					ImGui::Text("Button: ");
					ImGui::SetNextItemWidth(100.0f);

					if (ImGui::BeginCombo("##NullButtons", nullptr))
					{
						ImGui::EndCombo();
					}

					ImGui::EndGroup();
					ImGui::SameLine();
					ImGui::BeginGroup();

					ImGui::Text("Binding: ");
					ImGui::SetNextItemWidth(100.0f);

					if (ImGui::BeginCombo("##NullBindings", nullptr))
					{
						ImGui::EndCombo();
					}

					ImGui::EndGroup();
					break;
				}
			}
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::Text("NES Controller Port #2");
			ImGui::Text("Controller Type: ");
			ImGui::SetNextItemWidth(200.0f);
			if (ImGui::BeginCombo("##ControllerTwo", NesMachine.GetControllerTwo()->GetTypeName().data()))
			{
				for (int i = 0; i < NesController::ControllerType::NumOfControllers; i++)
				{
					if (ImGui::Selectable(NesMachine.GetControllerTwo()->GetTypeName((NesController::ControllerType)i).data()))
					{
						NesMachine.GetControllerTwo()->SwitchController((NesController::ControllerType)i);
					}
				}
				ImGui::EndCombo();
			}
			ImGui::Text("Input Device: ");
			ImGui::SetNextItemWidth(200.0f);
			if (ImGui::BeginCombo("##InputTwo", NesMachine.GetControllerTwo()->GetInputName().data()))
			{
				for (int i = 0; i < NesController::InputType::NumOfInputDevices; i++)
				{
					if (ImGui::Selectable(NesMachine.GetControllerTwo()->GetInputName((NesController::InputType)i).data()))
					{
						NesMachine.GetControllerTwo()->SwitchInputDevice((NesController::InputType)i);
					}
				}

				ImGui::EndCombo();
			}

			//Button Mapping
			switch (NesMachine.GetControllerTwo()->GetType())
			{
				case NesController::StandardController:
				{
					ImGui::BeginGroup();
					ImGui::Text("Button: ");
					ImGui::SetNextItemWidth(100.0f);

					static UInt8 StandardButtonPreviewTwo = 0;
					static bool KeyLookingForInputTwo = false;

					if (ImGui::BeginCombo("##StandardButtonsTwo", NesMachine.GetControllerTwo()->GetButtonName(StandardButtonPreviewTwo).data()))
					{
						for (int i = 0; i < 8; i++)
						{
							if (ImGui::Selectable(NesMachine.GetControllerTwo()->GetButtonName(i).data()))
							{
								StandardButtonPreviewTwo = i;
								KeyLookingForInputTwo = false;
							}
						}

						ImGui::EndCombo();
					}

					ImGui::EndGroup();
					ImGui::SameLine();
					ImGui::BeginGroup();

					if (NesMachine.GetControllerTwo()->GetInputDevice() == NesController::Keyboard)
					{
						ImGui::Text("Key: ");
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
						ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.1f, 0.5f));

						if (ImGui::Selectable(ImGui::GetKeyName(NesMachine.GetControllerTwo()->StandardControllerKeyboard[StandardButtonPreviewTwo]), KeyLookingForInputTwo, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100.0f, 18.0f)))
						{
							KeyLookingForInputTwo = !KeyLookingForInputTwo;
						}

						if (KeyLookingForInputTwo)
						{
							for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
							{
								if (ImGui::IsKeyDown(key) && ImGui::IsKeyboardKey(key))
								{
									NesMachine.GetControllerTwo()->StandardControllerKeyboard[StandardButtonPreviewTwo] = key;
									KeyLookingForInputTwo = false;
									break;
								}
							}
						}

						ImGui::PopStyleVar(2);
					}
					else if (NesMachine.GetControllerTwo()->GetInputDevice() == NesController::Gamepad)
					{
						ImGui::Text("Button: ");
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
						ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.1f, 0.5f));

						if (ImGui::Selectable(ImGui::GetKeyName(NesMachine.GetControllerTwo()->StandardControllerGamepad[StandardButtonPreviewTwo]), KeyLookingForInputTwo, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100.0f, 18.0f)))
						{
							KeyLookingForInputTwo = !KeyLookingForInputTwo;
						}

						if (KeyLookingForInputTwo)
						{
							for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
							{
								if (ImGui::IsKeyDown(key) && ImGui::IsGamepadKey(key))
								{
									NesMachine.GetControllerTwo()->StandardControllerGamepad[StandardButtonPreviewTwo] = key;
									KeyLookingForInputTwo = false;
									break;
								}
							}
						}

						ImGui::PopStyleVar(2);
					}
					else
					{
						ImGui::Text("Binding: ");
						ImGui::SetNextItemWidth(100.0f);

						if (ImGui::BeginCombo("##NullBindingsTwo", nullptr))
						{
							ImGui::EndCombo();
						}
					}

					ImGui::EndGroup();
					break;
				}
				default:
				{
					ImGui::BeginGroup();
					ImGui::Text("Button: ");
					ImGui::SetNextItemWidth(100.0f);

					if (ImGui::BeginCombo("##NullButtonsTwo", nullptr))
					{
						ImGui::EndCombo();
					}

					ImGui::EndGroup();
					ImGui::SameLine();
					ImGui::BeginGroup();

					ImGui::Text("Binding: ");
					ImGui::SetNextItemWidth(100.0f);

					if (ImGui::BeginCombo("##NullBindingsTwo", nullptr))
					{
						ImGui::EndCombo();
					}

					ImGui::EndGroup();
					break;
				}
			}
			ImGui::EndGroup();

			ImGui::End();
		}
	}
}