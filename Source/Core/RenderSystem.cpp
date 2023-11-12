#include "RenderSystem.h"

#include "optick.h"

namespace NesEmulator
{
	void RenderSystem::InitalizeRenderer(SDL_Window* WindowHandle, Diligent::RENDER_DEVICE_TYPE RendererBackend)
	{
		OPTICK_EVENT();
		Diligent::SwapChainDesc SwapDesc;
		SDL_SysWMinfo WindowInfo;
		SDL_VERSION(&WindowInfo.version);
		SDL_GetWindowWMInfo(WindowHandle, &WindowInfo);
		DeviceType = RendererBackend;

		switch (RendererBackend)
		{
			case Diligent::RENDER_DEVICE_TYPE_D3D12:
			{
				Diligent::EngineD3D12CreateInfo EngineCI;

				auto* FactoryD3D12 = Diligent::GetEngineFactoryD3D12();
				FactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &RenderDevice, &DeviceContext);
				Diligent::Win32NativeWindow Handle{ WindowInfo.info.win.window };
				FactoryD3D12->CreateSwapChainD3D12(RenderDevice, DeviceContext, SwapDesc, Diligent::FullScreenModeDesc{}, Handle, &SwapChain);

				break;
			}
			case Diligent::RENDER_DEVICE_TYPE_GL:
			{
				Diligent::EngineGLCreateInfo EngineCI;
				auto* pFactoryOpenGL = Diligent::GetEngineFactoryOpenGL();
				EngineCI.Window.hWnd = WindowInfo.info.win.window;
				pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &RenderDevice, &DeviceContext, SwapDesc, &SwapChain);

				break;
			}
			case Diligent::RENDER_DEVICE_TYPE_VULKAN:
			{
				Diligent::EngineVkCreateInfo EngineCI;
				auto* pFactoryVk = Diligent::GetEngineFactoryVk();
				pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &RenderDevice, &DeviceContext);
				Diligent::Win32NativeWindow Window{ WindowInfo.info.win.window };
				pFactoryVk->CreateSwapChainVk(RenderDevice, DeviceContext, SwapDesc, Window, &SwapChain);

				break;
			}
			case Diligent::RENDER_DEVICE_TYPE_METAL:
			{
				break;
			}
		}
	}

	void RenderSystem::ClearScreen()
	{
		OPTICK_EVENT();
		auto* pRTV = SwapChain->GetCurrentBackBufferRTV();
		auto* pDSV = SwapChain->GetDepthBufferDSV();
		const float ClearColor[] = { 0.000f, 0.000f, 0.000f, 1.0f };

		DeviceContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		DeviceContext->ClearRenderTarget(pRTV, ClearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		DeviceContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
	void RenderSystem::RenderImGui(ImGuiRenderData* RenderBatch, ImDrawData* ImData)
	{
		OPTICK_EVENT();
		//Create our vertex buffer and index buffer
		UInt32 VertexBufferSize = 1024;
		UInt32 IndexBufferSize = 2048;

		if (!RenderBatch->VertexData || VertexBufferSize < ImData->TotalVtxCount)
		{
			RenderBatch->VertexData.Release();

			while (VertexBufferSize < ImData->TotalVtxCount)
			{
				VertexBufferSize *= 2;
			}

			Diligent::BufferDesc VBDesc;
			VBDesc.Name = "Imgui Vertex Buffer";
			VBDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			VBDesc.Size = VertexBufferSize * sizeof(ImDrawVert);
			VBDesc.Usage = Diligent::USAGE_DYNAMIC;
			VBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			RenderDevice->CreateBuffer(VBDesc, nullptr, &RenderBatch->VertexData);
		}

		if (!RenderBatch->IndexData || IndexBufferSize < ImData->TotalIdxCount)
		{
			RenderBatch->IndexData.Release();

			while (IndexBufferSize < ImData->TotalIdxCount)
			{
				IndexBufferSize *= 2;
			}

			Diligent::BufferDesc IBDesc;
			IBDesc.Name = "Imgui Index Buffer";
			IBDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			IBDesc.Size = IndexBufferSize * sizeof(ImDrawIdx);
			IBDesc.Usage = Diligent::USAGE_DYNAMIC;
			IBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			RenderDevice->CreateBuffer(IBDesc, nullptr, &RenderBatch->IndexData);
		}

		//Fill in our vertex and index buffer.
		{
			Diligent::MapHelper<ImDrawVert> Verices(DeviceContext, RenderBatch->VertexData, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			Diligent::MapHelper<ImDrawIdx>  Indices(DeviceContext, RenderBatch->IndexData, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			ImDrawVert* pVtxDst = Verices;
			ImDrawIdx* pIdxDst = Indices;

			for (Int32 CmdListID = 0; CmdListID < ImData->CmdListsCount; CmdListID++)
			{
				const ImDrawList* pCmdList = ImData->CmdLists[CmdListID];
				memcpy(pVtxDst, pCmdList->VtxBuffer.Data, pCmdList->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(pIdxDst, pCmdList->IdxBuffer.Data, pCmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
				pVtxDst += pCmdList->VtxBuffer.Size;
				pIdxDst += pCmdList->IdxBuffer.Size;
			}
		}

		//Setup our constant buffer (ortho matrix)
		float L = ImData->DisplayPos.x;
		float R = ImData->DisplayPos.x + ImData->DisplaySize.x;
		float T = ImData->DisplayPos.y;
		float B = ImData->DisplayPos.y + ImData->DisplaySize.y;

		Diligent::float4x4 Projection
		{
			2.0f / (R - L),                  0.0f,   0.0f,   0.0f,
			0.0f,                  2.0f / (T - B),   0.0f,   0.0f,
			0.0f,                            0.0f,   0.5f,   0.0f,
			(R + L) / (L - R),  (T + B) / (B - T),   0.5f,   1.0f
		};

		Diligent::MapHelper<Diligent::float4x4> CBData(DeviceContext, RenderBatch->ConstantData, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		*CBData = Projection;

		//Setup our Render State.
		Diligent::IBuffer* pVBs[] = { RenderBatch->VertexData };
		DeviceContext->SetVertexBuffers(0, 1, pVBs, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
		DeviceContext->SetIndexBuffer(RenderBatch->IndexData, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		DeviceContext->SetPipelineState(RenderBatch->Pipeline);

		const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
		DeviceContext->SetBlendFactors(blend_factor);

		Diligent::Viewport vp;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.Width = static_cast<float>(RenderBatch->RenderWidth);
		vp.Height = static_cast<float>(RenderBatch->RenderHeight);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		DeviceContext->SetViewports(1, &vp, RenderBatch->RenderWidth, RenderBatch->RenderHeight);

		//Render Command Lists.
		ImVec2 clip_off = ImData->DisplayPos;
		UInt32 GlobalIdxOffset = 0;
		UInt32 GlobalVtxOffset = 0;
		Diligent::ITextureView* LastTextureView = nullptr;

		for (int n = 0; n < ImData->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = ImData->CmdLists[n];
			const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
			const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

				if (pcmd->UserCallback)
				{
					if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					{
						//Setup our Render State.
						Diligent::IBuffer* pVBs[] = { RenderBatch->VertexData };
						DeviceContext->SetVertexBuffers(0, 1, pVBs, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
						DeviceContext->SetIndexBuffer(RenderBatch->IndexData, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
						DeviceContext->SetPipelineState(RenderBatch->Pipeline);

						const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
						DeviceContext->SetBlendFactors(blend_factor);

						Diligent::Viewport vp;
						vp.TopLeftX = 0;
						vp.TopLeftY = 0;
						vp.Width = static_cast<float>(RenderBatch->RenderWidth);
						vp.Height = static_cast<float>(RenderBatch->RenderHeight);
						vp.MinDepth = 0.0f;
						vp.MaxDepth = 1.0f;
						DeviceContext->SetViewports(1, &vp, RenderBatch->RenderWidth, RenderBatch->RenderHeight);
					}
					else
					{
						pcmd->UserCallback(cmd_list, pcmd);
					}
				}
				else
				{
					ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
					ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
					if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
						continue;

					Diligent::Rect Scissor
					{
						static_cast<Int32>(clip_min.x),
						static_cast<Int32>(clip_min.y),
						static_cast<Int32>(clip_max.x),
						static_cast<Int32>(clip_max.y)
					};

					DeviceContext->SetScissorRects(1, &Scissor, RenderBatch->RenderWidth, RenderBatch->RenderHeight);

					//Bind texture
					auto* pTextureView = reinterpret_cast<Diligent::ITextureView*>(pcmd->TextureId);
					VERIFY_EXPR(pTextureView);

					if (pTextureView != LastTextureView)
					{
						LastTextureView = pTextureView;
						RenderBatch->TextureShaderVariable->Set(pTextureView);
						DeviceContext->CommitShaderResources(RenderBatch->ShaderVarBinding, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
					}

					Diligent::DrawIndexedAttribs DrawAttrs;
					DrawAttrs.NumIndices = pcmd->ElemCount;
					DrawAttrs.IndexType = sizeof(ImDrawIdx) == sizeof(UInt16) ? Diligent::VT_UINT16 : Diligent::VT_UINT32;
					DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_STATES;
					DrawAttrs.FirstIndexLocation = pcmd->IdxOffset + GlobalIdxOffset;

					if (RenderBatch->BaseVertexSupported)
					{
						DrawAttrs.BaseVertex = pcmd->VtxOffset + GlobalVtxOffset;
					}
					else
					{
						Diligent::IBuffer* pVBs[] = { RenderBatch->VertexData };
						Uint64   VtxOffsets[] = { sizeof(ImDrawVert) * (size_t{pcmd->VtxOffset} + size_t{GlobalVtxOffset}) };
						DeviceContext->SetVertexBuffers(0, 1, pVBs, VtxOffsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);
					}

					DeviceContext->DrawIndexed(DrawAttrs);
				}
			}

			GlobalIdxOffset += cmd_list->IdxBuffer.Size;
			GlobalVtxOffset += cmd_list->VtxBuffer.Size;
		}
	}
	void RenderSystem::Present()
	{
		OPTICK_EVENT();
		SwapChain->Present();
	}

	Diligent::IRenderDevice* RenderSystem::GetDevice()
	{
		return RenderDevice;
	}
	Diligent::IDeviceContext* RenderSystem::GetContext()
	{
		return DeviceContext;
	}
	Diligent::ISwapChain* RenderSystem::GetSwapChain()
	{
		return SwapChain;
	}
}