#include <Engine_pch.h>

#include "D3D12_Vertex_Buffer.h"

#include "Platform/DirectX_12/Direct3D12_Context.h"

namespace Insight {


	D3D12VertexBuffer::D3D12VertexBuffer(Verticies Verticies)
		: ieVertexBuffer(Verticies) 
	{
		m_NumVerticies = static_cast<uint32_t>(Verticies.size());
		m_BufferSize = m_NumVerticies * sizeof(Vertex3D);
		CreateResources();
	}

	void D3D12VertexBuffer::Destroy()
	{
	}

	bool D3D12VertexBuffer::CreateResources()
	{
		HRESULT hr;
		Direct3D12Context& RenderContext = Renderer::GetAs<Direct3D12Context>();

		hr = RenderContext.GetDeviceContext().CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_pVertexBuffer));
		if (FAILED(hr)) {
			IE_DEBUG_LOG(LogSeverity::Error, "Failed to upload vertex buffer resource heap");
			return false;
		}
		m_pVertexBuffer->SetName(L"Vertex Buffer Resource Heap");

		hr = RenderContext.GetDeviceContext().CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pVertexBufferUploadHeap));
		if (FAILED(hr)) {
			IE_DEBUG_LOG(LogSeverity::Error, "Failed to upload vertex buffer heap");
			return false;
		}
		m_pVertexBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(m_Verticies.data());
		vertexData.RowPitch = m_BufferSize;
		vertexData.SlicePitch = m_BufferSize;

		// TODO: This will fail in multithread mode because UpdateSubresources
		// modifies the command list and the command list is used elsewhere during this time
		// recording draw commands still, initializing other assets etc., causing a corruption.
		UpdateSubresources(&RenderContext.GetScenePassCommandList(), m_pVertexBuffer.Get(), m_pVertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);

		m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
		m_VertexBufferView.StrideInBytes = sizeof(Vertex3D);
		m_VertexBufferView.SizeInBytes = m_BufferSize;

		RenderContext.GetScenePassCommandList().ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

		return true;
	}

}
