#pragma once
#include "Definitions.h"
#include "ImGuiLayer.h"
#include "SDL.h"
#include "SDL_syswm.h"

#define D3D12_SUPPORTED 1
#define GL_SUPPORTED 1
#define VULKAN_SUPPORTED 1

#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

namespace NesEmulator
{
	class RenderSystem
	{
		private:
			Diligent::RENDER_DEVICE_TYPE DeviceType;

			Diligent::RefCntAutoPtr<Diligent::IRenderDevice> RenderDevice;
			Diligent::RefCntAutoPtr<Diligent::IDeviceContext> DeviceContext;
			Diligent::RefCntAutoPtr<Diligent::ISwapChain> SwapChain;

		public:

			void InitalizeRenderer(SDL_Window* WindowHandle, Diligent::RENDER_DEVICE_TYPE RendererBackend);

			void ClearScreen();
			void RenderImGui(ImGuiRenderData* RenderBatch, ImDrawData* ImData);
			void Present();

			Diligent::IRenderDevice* GetDevice();
			Diligent::IDeviceContext* GetContext();
			Diligent::ISwapChain* GetSwapChain();
	};
}