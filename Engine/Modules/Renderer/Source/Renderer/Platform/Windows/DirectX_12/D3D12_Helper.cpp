#include <Renderer_pch.h>

#include "D3D12_Helper.h"

#include "Platform/Windows/DirectX_12/Direct3D12_Context.h"
#include "Platform/Windows/Windows_Window.h"

namespace Insight {



	D3D12Helper::D3D12Helper()
	{
		
	}

	D3D12Helper::~D3D12Helper()
	{
	}

	bool D3D12Helper::Init(Direct3D12Context* pRendererContext)
	{
		m_pRenderContextRef = pRendererContext;
		CreateDXGIFactory();
		CreateDevice();
		CreateCommandQueues();
		CreateSwapChain();
		CreateViewport();
		CreateScissorRect();

		CreateFenceEvent();

		return true;
	}

	void D3D12Helper::ResizeResources()
	{

		uint32_t WindowWidth = m_pRenderContextRef->GetWindowRef().GetWidth();
		uint32_t WindowHeight = m_pRenderContextRef->GetWindowRef().GetHeight();

		// Resize the swapchain
		{
			HRESULT hr;
			DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
			m_pSwapChain->GetDesc(&SwapChainDesc);
			hr = m_pSwapChain->ResizeBuffers(m_FrameBufferCount, WindowWidth, WindowHeight, SwapChainDesc.BufferDesc.Format, SwapChainDesc.Flags);
			ThrowIfFailed(hr, "Failed to resize swap chain buffers for D3D 12 context.");
		}
		
		// Update View and Scissor rects
		{
			m_Client_ViewPort.TopLeftX = 0.0f;
			m_Client_ViewPort.TopLeftY = 0.0f;
			m_Client_ViewPort.Width = static_cast<FLOAT>(WindowWidth);
			m_Client_ViewPort.Height = static_cast<FLOAT>(WindowHeight);
			  
			m_Client_ScissorRect.left = static_cast<LONG>(m_Client_ViewPort.TopLeftX);
			m_Client_ScissorRect.right = static_cast<LONG>(m_Client_ViewPort.TopLeftX + m_Client_ViewPort.Width);
			m_Client_ScissorRect.top = static_cast<LONG>(m_Client_ViewPort.TopLeftY);
			m_Client_ScissorRect.bottom = static_cast<LONG>(m_Client_ViewPort.TopLeftX + m_Client_ViewPort.Height);
		}
	}

	void D3D12Helper::CleanUp()
	{
		// Close the fence handle on the GPU
		if (!CloseHandle(m_FenceEvent)) {
			IE_CORE_ERROR("Failed to close GPU handle while cleaning up the D3D 12 context.");
		}
	}

	void D3D12Helper::CreateDXGIFactory()
	{
		UINT DxgiFactoryFlags = 0u;

		// Enable debug layers if in debug builds
#if defined IE_DEBUG
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
				DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif

		HRESULT hr = CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(&m_pDxgiFactory));
		ThrowIfFailed(hr, "Failed to create DXGI Factory.");
	}

	void D3D12Helper::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
	{
		ComPtr<IDXGIAdapter1> pAdapter;
		*ppAdapter = nullptr;
		UINT currentVideoCardMemory = 0;
		DXGI_ADAPTER_DESC1 Desc;

		auto CheckRayTracingSupport = [](ID3D12Device* pDevice) {

			D3D12_FEATURE_DATA_D3D12_OPTIONS5 Options5 = {};
			ThrowIfFailed(pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &Options5, sizeof(Options5)), "Failed to query feature support for ray trace with device.");
			if (Options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) {
				IE_CORE_WARN("Ray tracing not supported on this device.");
				return false;
			}
			return true;
		};

		for (UINT AdapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(AdapterIndex, &pAdapter); ++AdapterIndex)
		{
			Desc = {};
			pAdapter->GetDesc1(&Desc);

			// Make sure we get the video card that is not a software adapter
			// and it has the most video memory
			if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE || Desc.DedicatedVideoMemory < currentVideoCardMemory) continue;

			// Check if we can support ray tracing with the device
			if (m_pRenderContextRef->GetIsRayTraceEnabled()) {

				ComPtr<ID3D12Device5> TempDevice;
				if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device5), &TempDevice))) {
					if (CheckRayTracingSupport(TempDevice.Get())) {

						currentVideoCardMemory = static_cast<UINT>(Desc.DedicatedVideoMemory);
						if (*ppAdapter != nullptr) {
							(*ppAdapter)->Release();
						}
						*ppAdapter = pAdapter.Detach();

						m_pRenderContextRef->SetIsRayTraceSupported(true);

						IE_CORE_WARN("Found suitable Direct3D 12 graphics hardware that can support ray tracing: {0}", StringHelper::WideToString(std::wstring{ Desc.Description }));
						continue;
					}
				}
			}

			// If we cannot support ray tracing, just see if D3D 12 is supported and create a default device
			if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {

				currentVideoCardMemory = static_cast<UINT>(Desc.DedicatedVideoMemory);
				if (*ppAdapter != nullptr) {
					(*ppAdapter)->Release();
				}
				*ppAdapter = pAdapter.Detach();

				IE_CORE_WARN("Found suitable Direct3D 12 graphics hardware: {0}", StringHelper::WideToString(std::wstring{ Desc.Description }));
			}
		}
		Desc = {};
		(*ppAdapter)->GetDesc1(&Desc);
		IE_CORE_WARN("\"{0}\" selected as Direct3D 12 graphics hardware.", StringHelper::WideToString(Desc.Description));
	}

	void D3D12Helper::CreateDevice()
	{
		GetHardwareAdapter(m_pDxgiFactory.Get(), &m_pAdapter);

		// If ray tracing is supported create the device DXR needs
		if (m_pRenderContextRef->m_IsRayTraceSupported) {
			ComPtr<ID3D12Device5> TempDevice;
			HRESULT hr = D3D12CreateDevice(m_pAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&TempDevice));
			ThrowIfFailed(hr, "Failed to create logical device for ray tracing.");
			m_pDevice = TempDevice.Detach();
		}
		else {
			HRESULT hr = D3D12CreateDevice(m_pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
			ThrowIfFailed(hr, "Failed to create logical device.");
		}
	}

	void D3D12Helper::CreateCommandQueues()
	{
		// Create Graphics Command Queue
		D3D12_COMMAND_QUEUE_DESC GraphicsCommandQueueDesc = {};
		GraphicsCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		GraphicsCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		HRESULT hr = m_pDevice->CreateCommandQueue(&GraphicsCommandQueueDesc, IID_PPV_ARGS(&m_pGraphicsCommandQueue));
		ThrowIfFailed(hr, "Failed to Create Command Queue");
		
		// Create Compute Command Queue
		D3D12_COMMAND_QUEUE_DESC ComputeCommandQueueDesc = {};
		ComputeCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ComputeCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

		hr = m_pDevice->CreateCommandQueue(&ComputeCommandQueueDesc, IID_PPV_ARGS(&m_pComputeCommandQueue));
		ThrowIfFailed(hr, "Failed to Create Command Queue");
	}

	void D3D12Helper::CreateSwapChain()
	{
		HRESULT hr;

		uint32_t WindowWidth = m_pRenderContextRef->GetWindowRef().GetWidth() / 2u;
		uint32_t WindowHeight = m_pRenderContextRef->GetWindowRef().GetHeight() / 2u;

		DXGI_MODE_DESC backBufferDesc = {};
		backBufferDesc.Width = WindowWidth;
		backBufferDesc.Height = WindowHeight;

		m_SampleDesc = {};
		m_SampleDesc.Count = 1;

		DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
		SwapChainDesc.BufferCount = m_FrameBufferCount;
		SwapChainDesc.Width = WindowWidth;
		SwapChainDesc.Height = WindowHeight;
		SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Dont save the contents of the back buffer after presented
		SwapChainDesc.Flags	= m_pRenderContextRef->m_AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		SwapChainDesc.SampleDesc = m_SampleDesc;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> TempSwapChain = {};
		hr = m_pDxgiFactory->CreateSwapChainForHwnd(&m_pRenderContextRef->GetCommandQueue(), m_pRenderContextRef->GetWindowRef().GetWindowHandleRef(), &SwapChainDesc, nullptr, nullptr, &TempSwapChain);
		ThrowIfFailed(hr, "Failed to Create Swap Chain");

		if (m_pRenderContextRef->m_AllowTearing) {
			ThrowIfFailed(m_pDxgiFactory->MakeWindowAssociation(m_pRenderContextRef->GetWindowRef().GetWindowHandleRef(), DXGI_MWA_NO_ALT_ENTER),
				"Failed to Make Window Association");
		}

		ThrowIfFailed(TempSwapChain.As(&m_pSwapChain), "Failed to cast SwapChain ComPtr");

		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		DXGI_SWAP_CHAIN_DESC1 Desc = {};
		m_pSwapChain->GetDesc1(&Desc);
		m_SwapChainBackBufferFormat = Desc.Format;
	}

	void D3D12Helper::CreateViewport()
	{
		uint32_t WindowWidth = m_pRenderContextRef->GetWindowRef().GetWidth() / 2u;
		uint32_t WindowHeight = m_pRenderContextRef->GetWindowRef().GetHeight() / 2u;

		m_Client_ViewPort.TopLeftX = 0;
		m_Client_ViewPort.TopLeftY = 0;
		m_Client_ViewPort.Width = static_cast<FLOAT>(WindowWidth);
		m_Client_ViewPort.Height = static_cast<FLOAT>(WindowHeight);
		m_Client_ViewPort.MinDepth = 0.0f;
		m_Client_ViewPort.MaxDepth = 1.0f;
	}

	void D3D12Helper::CreateScissorRect()
	{
		uint32_t WindowWidth = m_pRenderContextRef->GetWindowRef().GetWidth() / 2u;
		uint32_t WindowHeight = m_pRenderContextRef->GetWindowRef().GetHeight() / 2u;

		m_Client_ScissorRect.left = 0;
		m_Client_ScissorRect.top = 0;
		m_Client_ScissorRect.right = WindowWidth;
		m_Client_ScissorRect.bottom = WindowHeight;
	}

	void D3D12Helper::CreateFenceEvent()
	{
		HRESULT hr;
		hr = m_pDevice->CreateFence(m_FenceValues[m_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
		ThrowIfFailed(hr, "Failed to create Fence Event");
		m_FenceValues[m_FrameIndex]++;

		m_FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (m_FenceEvent == nullptr)
			THROW_COM_ERROR("Fence Event was nullptr");
	}

	void D3D12Helper::MoveToNextFrame()
	{
		HRESULT hr;

		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = m_FenceValues[m_FrameIndex];
		hr = m_pGraphicsCommandQueue->Signal(m_pFence.Get(), currentFenceValue);
		ThrowIfFailed(hr, "Failed to signal fence on Command Queue");

		// Advance the frame index.
		m_FrameIndex = (m_FrameIndex + 1) % m_FrameBufferCount;

		// Check to see if the next frame is ready to start.
		if (m_pFence->GetCompletedValue() < m_FenceValues[m_FrameIndex])
		{
			hr = m_pFence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent);
			ThrowIfFailed(hr, "Failed to set completion event on fence");
			WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		m_FenceValues[m_FrameIndex] = currentFenceValue + 1;
	}

	void D3D12Helper::WaitForGPU()
	{
		HRESULT hr;
		// Schedule a Signal command in the queue.
		hr = m_pGraphicsCommandQueue->Signal(m_pFence.Get(), m_FenceValues[m_FrameIndex]);
		ThrowIfFailed(hr, "Fialed to signal command queue while waiting for GPU");

		// Wait until the fence has been processed.
		hr = m_pFence.Get()->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent);
		ThrowIfFailed(hr, "Fialed to set fence on completion event while waiting for GPU");

		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

		// Increment the fence value for the current frame.
		m_FenceValues[m_FrameIndex]++;
	}

}