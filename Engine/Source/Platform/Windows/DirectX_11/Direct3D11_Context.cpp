#include <ie_pch.h>

#include "Direct3D11_Context.h"

#include "Insight/Core/Application.h"
#include "Platform/Windows/Windows_Window.h"
#include "Platform/Windows/DirectX_11/Geometry/D3D11_Index_Buffer.h"
#include "Platform/Windows/DirectX_11/Geometry/D3D11_Vertex_Buffer.h"

#include "Insight/Runtime/APlayer_Character.h"
#include "Insight/Systems/Managers/Geometry_Manager.h"

#include "Insight/Rendering/APost_Fx.h"
#include "Insight/Rendering/ASky_Light.h"
#include "Insight/Rendering/ASky_Sphere.h"
#include "Insight/Rendering/Lighting/ASpot_Light.h"
#include "Insight/Rendering/Lighting/APoint_Light.h"
#include "Insight/Rendering/Lighting/ADirectional_Light.h"

namespace Insight {




	Direct3D11Context::Direct3D11Context(WindowsWindow* WindowHandle) 
		:	m_pWindowHandle(&WindowHandle->GetWindowHandleReference()),
			m_pWindow(WindowHandle),
			Renderer(WindowHandle->GetWidth(), WindowHandle->GetHeight(), false)
	{
	}

	Direct3D11Context::~Direct3D11Context()
	{
	}

	bool Direct3D11Context::InitImpl()
	{
		IE_CORE_INFO("Renderer: D3D 11");

		CreateDXGIFactory();
		CreateDeviceAndSwapChain();
		CreateRTVs();
		InitShadersLayout();
		CreateViewports();
		LoadAssets();

		return true;
	}

	void Direct3D11Context::SetVertexBuffersImpl(uint32_t StartSlot, uint32_t NumBuffers, ieVertexBuffer* pBuffers)
	{
		m_pDeviceContext->IASetVertexBuffers(StartSlot, NumBuffers, reinterpret_cast<D3D11VertexBuffer*>(pBuffers)->GetBufferPtr(), reinterpret_cast<D3D11VertexBuffer*>(pBuffers)->GetStridePtr(), reinterpret_cast<D3D11VertexBuffer*>(pBuffers)->GetBufferOffset());
	}

	void Direct3D11Context::SetIndexBufferImpl(ieIndexBuffer* pBuffer)
	{
		m_pDeviceContext->IASetIndexBuffer(reinterpret_cast<D3D11IndexBuffer*>(pBuffer)->GetBufferPtr(), reinterpret_cast<D3D11IndexBuffer*>(pBuffer)->GetFormat(), reinterpret_cast<D3D11IndexBuffer*>(pBuffer)->GetBufferOffset());
	}

	void Direct3D11Context::DrawIndexedInstancedImpl(uint32_t IndexCountPerInstance, uint32_t NumInstances, uint32_t StartIndexLocation, uint32_t BaseVertexLoaction, uint32_t StartInstanceLocation)
	{
	}

	void Direct3D11Context::RenderSkySphereImpl()
	{
	}

	bool Direct3D11Context::CreateSkyboxImpl()
	{
		return false;
	}

	void Direct3D11Context::DestroySkyboxImpl()
	{
	}

	void Direct3D11Context::DestroyImpl()
	{
	}

	bool Direct3D11Context::PostInitImpl()
	{
		return false;
	}

	void Direct3D11Context::OnUpdateImpl(const float DeltaMs)
	{
	}

	void Direct3D11Context::OnPreFrameRenderImpl()
	{
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
		m_pDeviceContext->RSSetViewports(1, &m_ScenePassViewport);
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), m_ClearColor);
	}

	void Direct3D11Context::OnRenderImpl()
	{
		m_pDeviceContext->IASetInputLayout(m_VertexShader.GetInputLayout());
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

		m_pDeviceContext->VSSetShader(m_VertexShader.GetShader(), nullptr, 0);
		m_pDeviceContext->PSSetShader(m_PixelShader.GetShader(), nullptr, 0);

		UINT Stride = sizeof(ScreenSpaceVertex);
		UINT Offset = 0U;
		m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &Stride, &Offset);

		m_pDeviceContext->Draw(3, 0);

	}

	void Direct3D11Context::OnMidFrameRenderImpl()
	{
	}

	void Direct3D11Context::ExecuteDrawImpl()
	{
	}

	void Direct3D11Context::SwapBuffersImpl()
	{
		UINT PresentFlags = (m_AllowTearing && m_WindowedMode) ? DXGI_PRESENT_ALLOW_TEARING : 0;
		HRESULT hr = m_pSwapChain->Present(m_VSyncEnabled, PresentFlags);
		ThrowIfFailed(hr, "Failed to present frame for D3D 11 context.");
	}

	void Direct3D11Context::OnWindowResizeImpl()
	{
	}

	void Direct3D11Context::OnWindowFullScreenImpl()
	{
	}



	// Initializaion
	// --------------

	void Direct3D11Context::CreateDXGIFactory()
	{
		HRESULT hr = ::CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(m_pDxgiFactory.GetAddressOf()));
		ThrowIfFailed(hr, "Failed to create Dxgi Factor for DirectX 11.");
	}

	void Direct3D11Context::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter** ppAdapter)
	{
		HRESULT hr;

		ComPtr<IDXGIAdapter> pAdapter;
		*ppAdapter = nullptr;
		uint32_t CurrentVideoCardMemory = 0;
		DXGI_ADAPTER_DESC Desc;

		for (uint32_t AdapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters(AdapterIndex, &pAdapter); AdapterIndex++)
		{
			Desc = {};
			pAdapter->GetDesc(&Desc);
			
			// Make sure we get the video card that is not a software adapter
			// and it has the most video memory. Likly a software adapter if 
			// the device has no system or dedicated video memory.
			bool IsSoftwareAdapter = ((Desc.DedicatedVideoMemory | Desc.DedicatedSystemMemory) == 0);
			if ((Desc.DedicatedVideoMemory < CurrentVideoCardMemory) || IsSoftwareAdapter) {
				continue;
			}
			hr = ::D3D11CreateDevice(pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, NULL, nullptr, 0, D3D11_SDK_VERSION, NULL, NULL, NULL);
			if (SUCCEEDED(hr)) {

				CurrentVideoCardMemory = static_cast<uint32_t>(Desc.DedicatedSystemMemory);
				if (*ppAdapter != nullptr) {
					(*ppAdapter)->Release();
				}
				*ppAdapter = pAdapter.Detach();

				IE_CORE_WARN("Found suitable Direct3D 11 graphics hardware: {0}", StringHelper::WideToString(Desc.Description));
			}
		}
		Desc = {};
		(*ppAdapter)->GetDesc(&Desc);
		IE_CORE_WARN("\"{0}\" selected as Direct3D 11 graphics hardware.", StringHelper::WideToString(Desc.Description));
	}

	void Direct3D11Context::CreateDeviceAndSwapChain()
	{
		HRESULT hr;
		GetHardwareAdapter(m_pDxgiFactory.Get(), &m_pAdapter);

		UINT DeviceCreateFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined IE_DEBUG
		DeviceCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
		if (m_DeviceMaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_1) {
			DeviceCreateFlags |= D3D11_CREATE_DEVICE_DEBUGGABLE;
		}
#endif
		m_SampleDesc = {};
		m_SampleDesc.Count = 1;
		m_SampleDesc.Quality = 0;
		
		// TODO Query for HDR support
		DXGI_SWAP_CHAIN_DESC SwapChainDesc = { };
		SwapChainDesc.BufferDesc.Width = m_WindowWidth;
		SwapChainDesc.BufferDesc.Height = m_WindowHeight;
		SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		SwapChainDesc.SampleDesc = m_SampleDesc;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.BufferCount = static_cast<UINT>(m_FrameBufferCount);
		SwapChainDesc.OutputWindow = *m_pWindowHandle;
		SwapChainDesc.Windowed = TRUE;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		SwapChainDesc.Flags = m_AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		hr = ::D3D11CreateDeviceAndSwapChain(
			m_pAdapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN, 
			NULL, 
			DeviceCreateFlags,
			NULL, 
			0, 
			D3D11_SDK_VERSION, 
			&SwapChainDesc,
			&m_pSwapChain, 
			&m_pDevice, 
			NULL, 
			&m_pDeviceContext
		);
		ThrowIfFailed(hr, "Failed to create swapchain for D3D 11 context");

		if (m_AllowTearing) {
			ThrowIfFailed(m_pDxgiFactory->MakeWindowAssociation(*m_pWindowHandle, DXGI_MWA_NO_ALT_ENTER), "Failed to make window association for D3D 11 context.");
		}
	}

	void Direct3D11Context::CreateRTVs()
	{
		HRESULT hr;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(m_pBackBuffer.GetAddressOf()));
		ThrowIfFailed(hr, "Failed to get the back buffer from the swapchain for D3D 11 context.");

		hr = m_pDevice->CreateRenderTargetView(m_pBackBuffer.Get(), NULL, m_pRenderTargetView.GetAddressOf());
		ThrowIfFailed(hr, "Failed to create render target view for D3D 11 context.");
	}

	void Direct3D11Context::CreateViewports()
	{
		m_ScenePassViewport = {};
		m_ScenePassViewport.TopLeftX = 0.0f;
		m_ScenePassViewport.TopLeftY = 0.0f;
		m_ScenePassViewport.Width = static_cast<FLOAT>(m_pWindow->GetWidth());
		m_ScenePassViewport.Height = static_cast<FLOAT>(m_pWindow->GetHeight());
		m_ScenePassViewport.MinDepth = 0.0f;
		m_ScenePassViewport.MaxDepth = 1.0f;
	}

	void Direct3D11Context::LoadAssets()
	{
		ScreenSpaceVertex Verticies[] =
		{
			{ ieFloat3{-0.5f, 0.0f, 0.0f}, ieFloat2{0.0f, 0.0f} },
			{ ieFloat3{0.5f, 0.0f, 0.0f}, ieFloat2{0.0f, 0.0f} },
			{ ieFloat3{0.0f, 0.5f, 0.0f}, ieFloat2{0.0f, 0.0f} }
		};
		D3D11_BUFFER_DESC VertexBufferDesc = {};
		VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		VertexBufferDesc.ByteWidth = sizeof(ScreenSpaceVertex) * ARRAYSIZE(Verticies);
		VertexBufferDesc.CPUAccessFlags = 0;
		VertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA VertexBufferData = {};
		VertexBufferData.pSysMem = Verticies;

		HRESULT hr = m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexBufferData, m_pVertexBuffer.GetAddressOf());
		ThrowIfFailed(hr, "Failed to create vertex buffer for D3D11 context.");
	}

	void Direct3D11Context::InitShadersLayout()
	{
#if defined IE_DEBUG
		LPCWSTR VertexShaderFolder = L"../Bin/Debug-windows-x86_64/Engine/SimpleShader.vertex.cso";
		LPCWSTR PixelShaderFolder = L"../Bin/Debug-windows-x86_64/Engine/SimpleShader.pixel.cso";
#elif defined IE_RELEASE || defined IE_GAME_DIST || defined IE_ENGINE_DIST
		LPCWSTR VertexShaderFolder = L"SimpleShader.vertex.cso";
		LPCWSTR PixelShaderFolder = L"SimpleShader.pixel.cso";
#endif 

		D3D11_INPUT_ELEMENT_DESC InputLayout[] = 
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		m_VertexShader.Init(m_pDevice, VertexShaderFolder, InputLayout, ARRAYSIZE(InputLayout));
		m_PixelShader.Init(m_pDevice, PixelShaderFolder);

	}


}