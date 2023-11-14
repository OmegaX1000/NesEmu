#include "ImGuiLayer.h"
#include "ImGuiShaders.h"

#include "optick.h"

namespace NesEmulator
{
	void ImGuiLayer::ImGuiCreate(Diligent::IRenderDevice* Device, Diligent::ISwapChain* SwapChain)
	{
		OPTICK_EVENT();
		Context = ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(640.0f, 480.0f);
		io.DeltaTime = 1.0f / 60.0f;
		io.IniFilename = NULL;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		
		RenderData.BaseVertexSupported = Device->GetAdapterInfo().DrawCommand.CapFlags & Diligent::DRAW_COMMAND_CAP_FLAG_BASE_VERTEX;
		if (RenderData.BaseVertexSupported)
		{
			io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
		}

		//Fonts
		io.Fonts->AddFontDefault();

		ImFontConfig config;
		config.MergeMode = true;
		config.GlyphMinAdvanceX = 10.0f;
		static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		std::string Font = FONT_ICON_FILE_NAME_FAS;
		std::string Path = SDL_GetBasePath();
		std::string::size_type pos = Path.find_first_of('\\');

		while (pos < Path.length())
		{
			Path = Path.replace(pos, 1, "/");
			pos = Path.find_first_of('\\');
		}

		Path.append("Assets/Fonts/" + Font);

		io.Fonts->AddFontFromFileTTF(Path.c_str(), 10.0f, &config, icon_ranges);

		CreateDeviceObjects(Device, SwapChain);
	}
	void ImGuiLayer::ImGuiDestroy()
	{
		OPTICK_EVENT();
		ImGui::DestroyContext(Context);
	}

	void ImGuiLayer::ProcessEvents(SDL_Event* Event)
	{
		OPTICK_EVENT();
		ImGuiIO& io = ImGui::GetIO();

		switch (Event->type)
		{
			case SDL_MOUSEMOTION:
			{
				ImVec2 mouse_pos((float)Event->motion.x, (float)Event->motion.y);
				io.AddMouseSourceEvent(Event->motion.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
				io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
				break;
			}
			case SDL_MOUSEWHEEL:
			{
				float wheel_x = -Event->wheel.preciseX;
				float wheel_y = Event->wheel.preciseY;

				io.AddMouseSourceEvent(Event->wheel.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
				io.AddMouseWheelEvent(wheel_x, wheel_y);
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				int mouse_button = -1;
				if (Event->button.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
				if (Event->button.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
				if (Event->button.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
				if (Event->button.button == SDL_BUTTON_X1) { mouse_button = 3; }
				if (Event->button.button == SDL_BUTTON_X2) { mouse_button = 4; }
				if (mouse_button == -1)
					break;
				io.AddMouseSourceEvent(Event->button.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
				io.AddMouseButtonEvent(mouse_button, (Event->type == SDL_MOUSEBUTTONDOWN));
				break;
			}
			case SDL_TEXTINPUT:
			{
				io.AddInputCharactersUTF8(Event->text.text);
				break;
			}
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				io.AddKeyEvent(ImGuiMod_Ctrl, (Event->key.keysym.mod & KMOD_CTRL) != 0);
				io.AddKeyEvent(ImGuiMod_Shift, (Event->key.keysym.mod & KMOD_SHIFT) != 0);
				io.AddKeyEvent(ImGuiMod_Alt, (Event->key.keysym.mod & KMOD_ALT) != 0);
				io.AddKeyEvent(ImGuiMod_Super, (Event->key.keysym.mod & KMOD_GUI) != 0);

				ImGuiKey key = SDL_KeycodeToImGuiKey(Event->key.keysym.sym);
				io.AddKeyEvent(key, (Event->type == SDL_KEYDOWN));
				break;
			}
			case SDL_WINDOWEVENT:
			{
				if (Event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
				{
					io.AddFocusEvent(true);
				}
				else if (Event->window.event == SDL_WINDOWEVENT_FOCUS_LOST)
				{
					io.AddFocusEvent(false);
				}

				break;
			}
		}
	}
	void ImGuiLayer::BeginFrame(SDL_Window* WindowHandle, Diligent::IRenderDevice* Device, Diligent::ISwapChain* SwapChain)
	{
		OPTICK_EVENT();
		if (!RenderData.Pipeline)
		{
			CreateDeviceObjects(Device, SwapChain);
		}

		ImGuiIO& io = ImGui::GetIO();

		RenderData.RenderWidth = SwapChain->GetDesc().Width;
		RenderData.RenderHeight = SwapChain->GetDesc().Height;

		io.DisplaySize = ImVec2((Float32)SwapChain->GetDesc().Width, (Float32)SwapChain->GetDesc().Height);
		io.DeltaTime = 1.0f / 60.0f;

		ImGui::NewFrame();
	}
	void ImGuiLayer::EndFrame()
	{
		OPTICK_EVENT();
		ImGui::EndFrame();
		ImGui::Render();
	}

	void ImGuiLayer::CreateDeviceObjects(Diligent::IRenderDevice* Device, Diligent::ISwapChain* SwapChain)
	{
		OPTICK_EVENT();
		//Release our resources so we can create them again.
		ReleaseDeviceObjects();

		//Begin creating our pipeline object.
		Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;
		PSOCreateInfo.PSODesc.Name = "ImGUI PSO";
		PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

		PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
		PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = SwapChain->GetDesc().ColorBufferFormat;
		PSOCreateInfo.GraphicsPipeline.DSVFormat = SwapChain->GetDesc().DepthBufferFormat;
		PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
		PSOCreateInfo.GraphicsPipeline.RasterizerDesc.ScissorEnable = true;
		PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;

		PSOCreateInfo.GraphicsPipeline.BlendDesc.AlphaToCoverageEnable = false;
		PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
		PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_ONE;
		PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = Diligent::BLEND_OPERATION_ADD;
		PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = Diligent::BLEND_FACTOR_ZERO;
		PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
		PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;

		//Vertex Layout
		Diligent::LayoutElement VSInputs[]
		{
			{0, 0, 2, Diligent::VT_FLOAT32},    // pos
			{1, 0, 2, Diligent::VT_FLOAT32},    // uv
			{2, 0, 4, Diligent::VT_UINT8, true} // col
		};
		PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(VSInputs);
		PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = VSInputs;

		//Create our Shaders.
		Diligent::ShaderCreateInfo ShaderCI;
		ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_DEFAULT;
		Diligent::RENDER_DEVICE_TYPE DeviceType = Device->GetDeviceInfo().Type;

		Diligent::ShaderMacro Macros[] =
		{
			{"GAMMA_TO_LINEAR(Gamma)", GAMMA_TO_LINEAR},
			{"SRGBA_TO_LINEAR(col)", SRGBA_TO_LINEAR},
		};
		ShaderCI.Macros = { Macros, _countof(Macros) };

		Diligent::RefCntAutoPtr<Diligent::IShader> VertexShader;
		{
			ShaderCI.Desc = { "Imgui VS", Diligent::SHADER_TYPE_VERTEX, true };

			switch (DeviceType)
			{
				case Diligent::RENDER_DEVICE_TYPE_VULKAN:
				{
					ShaderCI.ByteCode = VertexShader_SPIRV;
					ShaderCI.ByteCodeSize = sizeof(VertexShader_SPIRV);
					break;
				}
				case Diligent::RENDER_DEVICE_TYPE_D3D11:
				case Diligent::RENDER_DEVICE_TYPE_D3D12:
				{
					ShaderCI.Source = VertexShaderHLSL;
					break;
				}
				case Diligent::RENDER_DEVICE_TYPE_GL:
				case Diligent::RENDER_DEVICE_TYPE_GLES:
				{
					ShaderCI.Source = VertexShaderGLSL;
					break;
				}
				case Diligent::RENDER_DEVICE_TYPE_METAL:
				{
					ShaderCI.Source = ShadersMSL;
					ShaderCI.EntryPoint = "vs_main";
					break;
				}
			}

			Device->CreateShader(ShaderCI, &VertexShader);
		}

		Diligent::RefCntAutoPtr<Diligent::IShader> FragmentShader;
		{
			ShaderCI.Desc = { "Imgui PS", Diligent::SHADER_TYPE_PIXEL, true };

			switch (DeviceType)
			{
				case Diligent::RENDER_DEVICE_TYPE_VULKAN:
				{
					ShaderCI.ByteCode = FragmentShader_Gamma_SPIRV;
					ShaderCI.ByteCodeSize = sizeof(FragmentShader_Gamma_SPIRV);

					break;
				}
				case Diligent::RENDER_DEVICE_TYPE_D3D11:
				case Diligent::RENDER_DEVICE_TYPE_D3D12:
				{
					ShaderCI.Source = PixelShaderHLSL;
					break;
				}
				case Diligent::RENDER_DEVICE_TYPE_GL:
				case Diligent::RENDER_DEVICE_TYPE_GLES:
				{
					ShaderCI.Source = PixelShaderGLSL;
					break;
				}
				case Diligent::RENDER_DEVICE_TYPE_METAL:
				{
					ShaderCI.Source = ShadersMSL;
					ShaderCI.EntryPoint = "ps_main";
					break;
				}
			}

			Device->CreateShader(ShaderCI, &FragmentShader);
		}

		PSOCreateInfo.pVS = VertexShader;
		PSOCreateInfo.pPS = FragmentShader;

		//Texture samplers and Shader variables.
		Diligent::ShaderResourceVariableDesc Variables[] =
		{
			{ Diligent::SHADER_TYPE_PIXEL, "Texture",  Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}
		};
		PSOCreateInfo.PSODesc.ResourceLayout.Variables = Variables;
		PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Variables);

		Diligent::SamplerDesc SamLinearWrap;
		SamLinearWrap.MinFilter = Diligent::FILTER_TYPE_POINT;
		SamLinearWrap.MagFilter = Diligent::FILTER_TYPE_POINT;
		SamLinearWrap.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
		SamLinearWrap.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
		SamLinearWrap.AddressW = Diligent::TEXTURE_ADDRESS_WRAP;
		Diligent::ImmutableSamplerDesc ImtblSamplers[] =
		{
			{ Diligent::SHADER_TYPE_PIXEL, "Texture", SamLinearWrap}
		};
		PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
		PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

		RenderData.RenderWidth = SwapChain->GetDesc().Width;
		RenderData.RenderHeight = SwapChain->GetDesc().Height;

		Device->CreateGraphicsPipelineState(PSOCreateInfo, &RenderData.Pipeline);

		Diligent::BufferDesc BuffDesc;
		BuffDesc.Size = sizeof(Diligent::float4x4);
		BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
		BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		Device->CreateBuffer(BuffDesc, nullptr, &RenderData.ConstantData);
		RenderData.Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(RenderData.ConstantData);

		CreateFontTextures(Device);
	}
	void ImGuiLayer::CreateFontTextures(Diligent::IRenderDevice* Device)
	{
		OPTICK_EVENT();
		//Build texture atlas
		ImGuiIO& IO = ImGui::GetIO();

		unsigned char* TexData = nullptr;
		int            Width = 0;
		int            Weight = 0;
		IO.Fonts->GetTexDataAsRGBA32(&TexData, &Width, &Weight);

		Diligent::TextureDesc FontTexDesc;
		FontTexDesc.Name = "Imgui Font Texture";
		FontTexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		FontTexDesc.Width = static_cast<UInt32>(Width);
		FontTexDesc.Height = static_cast<UInt32>(Weight);
		FontTexDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
		FontTexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		FontTexDesc.Usage = Diligent::USAGE_IMMUTABLE;

		Diligent::TextureSubResData Mip0Data[] = { {TexData, 4 * UInt64{FontTexDesc.Width}} };
		Diligent::TextureData       InitData(Mip0Data, _countof(Mip0Data));

		Diligent::RefCntAutoPtr<Diligent::ITexture> pFontTex;
		Device->CreateTexture(FontTexDesc, &InitData, &pFontTex);
		RenderData.TextureView = pFontTex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

		RenderData.ShaderVarBinding.Release();
		RenderData.Pipeline->CreateShaderResourceBinding(&RenderData.ShaderVarBinding, true);
		RenderData.TextureShaderVariable = RenderData.ShaderVarBinding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "Texture");
		VERIFY_EXPR(RenderData.TextureShaderVariable != nullptr);

		//Store our identifier
		IO.Fonts->TexID = (ImTextureID)RenderData.TextureView;
	}
	void ImGuiLayer::ReleaseDeviceObjects()
	{
		OPTICK_EVENT();
		RenderData.ShaderVarBinding.Release();
		RenderData.TextureView.Release();
		RenderData.ConstantData.Release();
		RenderData.VertexData.Release();
		RenderData.IndexData.Release();
		RenderData.Pipeline.Release();
	}
	ImGuiRenderData* ImGuiLayer::GetRenderData()
	{
		return &RenderData;
	}
	ImGuiKey ImGuiLayer::SDL_KeycodeToImGuiKey(int keycode)
	{
		switch (keycode)
		{
			case SDLK_TAB: return ImGuiKey_Tab;
			case SDLK_LEFT: return ImGuiKey_LeftArrow;
			case SDLK_RIGHT: return ImGuiKey_RightArrow;
			case SDLK_UP: return ImGuiKey_UpArrow;
			case SDLK_DOWN: return ImGuiKey_DownArrow;
			case SDLK_PAGEUP: return ImGuiKey_PageUp;
			case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
			case SDLK_HOME: return ImGuiKey_Home;
			case SDLK_END: return ImGuiKey_End;
			case SDLK_INSERT: return ImGuiKey_Insert;
			case SDLK_DELETE: return ImGuiKey_Delete;
			case SDLK_BACKSPACE: return ImGuiKey_Backspace;
			case SDLK_SPACE: return ImGuiKey_Space;
			case SDLK_RETURN: return ImGuiKey_Enter;
			case SDLK_ESCAPE: return ImGuiKey_Escape;
			case SDLK_QUOTE: return ImGuiKey_Apostrophe;
			case SDLK_COMMA: return ImGuiKey_Comma;
			case SDLK_MINUS: return ImGuiKey_Minus;
			case SDLK_PERIOD: return ImGuiKey_Period;
			case SDLK_SLASH: return ImGuiKey_Slash;
			case SDLK_SEMICOLON: return ImGuiKey_Semicolon;
			case SDLK_EQUALS: return ImGuiKey_Equal;
			case SDLK_LEFTBRACKET: return ImGuiKey_LeftBracket;
			case SDLK_BACKSLASH: return ImGuiKey_Backslash;
			case SDLK_RIGHTBRACKET: return ImGuiKey_RightBracket;
			case SDLK_BACKQUOTE: return ImGuiKey_GraveAccent;
			case SDLK_CAPSLOCK: return ImGuiKey_CapsLock;
			case SDLK_SCROLLLOCK: return ImGuiKey_ScrollLock;
			case SDLK_NUMLOCKCLEAR: return ImGuiKey_NumLock;
			case SDLK_PRINTSCREEN: return ImGuiKey_PrintScreen;
			case SDLK_PAUSE: return ImGuiKey_Pause;
			case SDLK_KP_0: return ImGuiKey_Keypad0;
			case SDLK_KP_1: return ImGuiKey_Keypad1;
			case SDLK_KP_2: return ImGuiKey_Keypad2;
			case SDLK_KP_3: return ImGuiKey_Keypad3;
			case SDLK_KP_4: return ImGuiKey_Keypad4;
			case SDLK_KP_5: return ImGuiKey_Keypad5;
			case SDLK_KP_6: return ImGuiKey_Keypad6;
			case SDLK_KP_7: return ImGuiKey_Keypad7;
			case SDLK_KP_8: return ImGuiKey_Keypad8;
			case SDLK_KP_9: return ImGuiKey_Keypad9;
			case SDLK_KP_PERIOD: return ImGuiKey_KeypadDecimal;
			case SDLK_KP_DIVIDE: return ImGuiKey_KeypadDivide;
			case SDLK_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
			case SDLK_KP_MINUS: return ImGuiKey_KeypadSubtract;
			case SDLK_KP_PLUS: return ImGuiKey_KeypadAdd;
			case SDLK_KP_ENTER: return ImGuiKey_KeypadEnter;
			case SDLK_KP_EQUALS: return ImGuiKey_KeypadEqual;
			case SDLK_LCTRL: return ImGuiKey_LeftCtrl;
			case SDLK_LSHIFT: return ImGuiKey_LeftShift;
			case SDLK_LALT: return ImGuiKey_LeftAlt;
			case SDLK_LGUI: return ImGuiKey_LeftSuper;
			case SDLK_RCTRL: return ImGuiKey_RightCtrl;
			case SDLK_RSHIFT: return ImGuiKey_RightShift;
			case SDLK_RALT: return ImGuiKey_RightAlt;
			case SDLK_RGUI: return ImGuiKey_RightSuper;
			case SDLK_APPLICATION: return ImGuiKey_Menu;
			case SDLK_0: return ImGuiKey_0;
			case SDLK_1: return ImGuiKey_1;
			case SDLK_2: return ImGuiKey_2;
			case SDLK_3: return ImGuiKey_3;
			case SDLK_4: return ImGuiKey_4;
			case SDLK_5: return ImGuiKey_5;
			case SDLK_6: return ImGuiKey_6;
			case SDLK_7: return ImGuiKey_7;
			case SDLK_8: return ImGuiKey_8;
			case SDLK_9: return ImGuiKey_9;
			case SDLK_a: return ImGuiKey_A;
			case SDLK_b: return ImGuiKey_B;
			case SDLK_c: return ImGuiKey_C;
			case SDLK_d: return ImGuiKey_D;
			case SDLK_e: return ImGuiKey_E;
			case SDLK_f: return ImGuiKey_F;
			case SDLK_g: return ImGuiKey_G;
			case SDLK_h: return ImGuiKey_H;
			case SDLK_i: return ImGuiKey_I;
			case SDLK_j: return ImGuiKey_J;
			case SDLK_k: return ImGuiKey_K;
			case SDLK_l: return ImGuiKey_L;
			case SDLK_m: return ImGuiKey_M;
			case SDLK_n: return ImGuiKey_N;
			case SDLK_o: return ImGuiKey_O;
			case SDLK_p: return ImGuiKey_P;
			case SDLK_q: return ImGuiKey_Q;
			case SDLK_r: return ImGuiKey_R;
			case SDLK_s: return ImGuiKey_S;
			case SDLK_t: return ImGuiKey_T;
			case SDLK_u: return ImGuiKey_U;
			case SDLK_v: return ImGuiKey_V;
			case SDLK_w: return ImGuiKey_W;
			case SDLK_x: return ImGuiKey_X;
			case SDLK_y: return ImGuiKey_Y;
			case SDLK_z: return ImGuiKey_Z;
			case SDLK_F1: return ImGuiKey_F1;
			case SDLK_F2: return ImGuiKey_F2;
			case SDLK_F3: return ImGuiKey_F3;
			case SDLK_F4: return ImGuiKey_F4;
			case SDLK_F5: return ImGuiKey_F5;
			case SDLK_F6: return ImGuiKey_F6;
			case SDLK_F7: return ImGuiKey_F7;
			case SDLK_F8: return ImGuiKey_F8;
			case SDLK_F9: return ImGuiKey_F9;
			case SDLK_F10: return ImGuiKey_F10;
			case SDLK_F11: return ImGuiKey_F11;
			case SDLK_F12: return ImGuiKey_F12;
		}

		return ImGuiKey_None;
	}
}