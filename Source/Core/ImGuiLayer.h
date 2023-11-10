#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <SDL.h>
#include "Definitions.h"
#include "IconsFontAwesome5.h"

#include "Primitives/interface/BasicTypes.h"
#include "Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "Graphics/GraphicsAccessories/interface/GraphicsAccessories.hpp"

namespace NesEmulator
{
	struct ImGuiRenderData
	{
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> Pipeline;

		Diligent::RefCntAutoPtr<Diligent::IBuffer>				  VertexData;
		Diligent::RefCntAutoPtr<Diligent::IBuffer>				  IndexData;
		Diligent::RefCntAutoPtr<Diligent::IBuffer>				  ConstantData;
		Diligent::RefCntAutoPtr<Diligent::ITextureView>           TextureView;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> ShaderVarBinding;
		Diligent::IShaderResourceVariable* TextureShaderVariable = nullptr;

		UInt32 RenderWidth = 0;
		UInt32 RenderHeight = 0;
		bool BaseVertexSupported = false;
	};

	class ImGuiLayer
	{
		private:
			ImGuiContext* Context;
			ImGuiRenderData RenderData;

		public:

			void ImGuiCreate(Diligent::IRenderDevice* Device, Diligent::ISwapChain* SwapChain);
			void ImGuiDestroy();

			void ProcessEvents(SDL_Event* Event);
			void BeginFrame(SDL_Window* WindowHandle, Diligent::IRenderDevice* Device, Diligent::ISwapChain* SwapChain);
			void EndFrame();

			void CreateDeviceObjects(Diligent::IRenderDevice* Device, Diligent::ISwapChain* SwapChain);
			void CreateFontTextures(Diligent::IRenderDevice* Device);
			void ReleaseDeviceObjects();
			ImGuiRenderData* GetRenderData();
			ImGuiKey SDL_KeycodeToImGuiKey(int keycode);
	};
}