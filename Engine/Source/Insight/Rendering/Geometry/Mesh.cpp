#include <ie_pch.h>

#include "Mesh.h"

#include "Insight/Core/Application.h"
#include "Insight/Rendering/Renderer.h"

#include "Platform/Windows/DirectX_12/D3D12_Vertex_Buffer.h"
#include "Platform/Windows/DirectX_12/D3D12_Index_Buffer.h"


#include "imgui.h"

// TEMP
#include "Platform/Windows/DirectX_12/Direct3D12_Context.h"

namespace Insight {


	Mesh::Mesh(Verticies Verticies, Indices Indices)
	{
		Init(Verticies, Indices);
	}

	Mesh::~Mesh()
	{
		Destroy();
	}

	void Mesh::Destroy()
	{
		delete m_pVertexBuffer;
		delete m_pIndexBuffer;
	}

	void Mesh::Init(Verticies& Verticies, Indices& Indices)
	{
		CreateBuffers(Verticies, Indices);
	}

	void Mesh::PreRender(const XMMATRIX& parentMat)
	{
		m_Transform.SetWorldMatrix(XMMatrixMultiply(parentMat, m_Transform.GetLocalMatrix()));

		XMMATRIX worldMatTransposed = XMMatrixTranspose(m_Transform.GetWorldMatrixRef());
		XMFLOAT4X4 worldFloat;
		XMStoreFloat4x4(&worldFloat, worldMatTransposed);

		m_ConstantBufferPerObject.world = worldFloat;
	}

	CB_VS_PerObject Mesh::GetConstantBuffer()
	{
		return m_ConstantBufferPerObject;
	}

	void Mesh::Render(ID3D12GraphicsCommandList*& pCommandList)
	{
		Renderer::SetVertexBuffers(0, 1, m_pVertexBuffer);
		Renderer::SetIndexBuffer(m_pIndexBuffer);
		Renderer::DrawIndexedInstanced(m_pIndexBuffer->GetNumIndices(), 1, 0, 0, 0);
	}

	void Mesh::OnImGuiRender()
	{
	}

	void Mesh::CreateBuffers(Verticies& Verticies, Indices& Indices)
	{
		switch (Renderer::GetAPI()) {
		case Renderer::eRenderingAPI::D3D_11:
		{
			// TODO
			break;
		}
		case Renderer::eRenderingAPI::D3D_12:
		{
			m_pVertexBuffer = new D3D12VertexBuffer(Verticies);
			m_pIndexBuffer = new D3D12IndexBuffer(Indices);
			break;
		}
		case Renderer::eRenderingAPI::INVALID:
		{
			IE_CORE_FATAL(L"Mesh trying to be created before the renderer has been initialized.");
			break;
		}
		default:
		{
			IE_CORE_ERROR("Failed to determine vertex buffer type for mesh.");
			break;
		}
		}
	}
}

