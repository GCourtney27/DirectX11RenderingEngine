#include <ie_pch.h>

#include "Model_Manager.h"

#include "Platform/DirectX12/Direct3D12_Context.h"
#include <fstream>

namespace Insight {

	ModelManager* ModelManager::s_Instance = nullptr;

	ModelManager::ModelManager()
	{
		IE_CORE_ASSERT(!s_Instance, "An instance of Model Manager already exists!");
		s_Instance = this;
	}

	ModelManager::~ModelManager()
	{
		FlushModelCache();
	}

	bool ModelManager::Init()
	{
		m_ConstantBufferUploadHeaps = &Direct3D12Context::Get().GetConstantBufferUploadHeap();
		m_pCommandList = &Direct3D12Context::Get().GetCommandList();
		//LoadAssetsFromFile("../Assets/EngineDEBUG/ModelCache.txt");
		return true;
	}

	bool ModelManager::LoadAssetsFromFile(const std::string& modelsPath)
	{
		std::ifstream inStream(modelsPath.c_str());
		std::string line = "";

		auto it = m_Models.begin();
		if (inStream.is_open()) {
			while (std::getline(inStream, line)) {
				StrongModelPtr mesh = make_shared<Model>(line);
				Transform t;

				//it->second = t;
				//m_Models.insert(mesh, push(t));
				//m_Models.insert(mesh, push(t));
			}
		}

		return true;
	}

	void ModelManager::Draw()
	{
		D3D12_GPU_VIRTUAL_ADDRESS cbvHandle(m_ConstantBufferUploadHeaps->GetGPUVirtualAddress());

		for (std::shared_ptr<Model>& model : m_Models)
		{
			for (unsigned int j = 0; j < model->GetNumChildMeshes(); j++)
			{
				m_pCommandList->SetGraphicsRootConstantBufferView(0, cbvHandle + (ConstantBufferPerObjectAlignedSize * j));

				model->GetMeshAtIndex(j)->Render();
			}
		}

	}

	// Update the Constant buffers in the gpu with the new data for each model. Does not draw models
	void ModelManager::UploadVertexDataToGPU()
	{
		UINT8* cbvGPUAddress = &Direct3D12Context::Get().GetConstantBufferViewGPUHeapAddress();
		UINT32 gpuAdressOffset = 0;
		for (unsigned int i = 0; i < m_Models.size(); i++)
		{

			for (unsigned int j = 0; j < m_Models[i]->GetNumChildMeshes(); j++)
			{
				// TODO: draw instanced if ref count is greater than one on shared pointer
				CB_VS_PerObject cbPerObject = m_Models[i]->GetMeshAtIndex(j)->GetConstantBuffer();

				memcpy(cbvGPUAddress + (ConstantBufferPerObjectAlignedSize * gpuAdressOffset++), &cbPerObject, sizeof(cbPerObject));
			}

		}

	}

	bool ModelManager::LoadMeshFromFile(const std::string& filePath, bool async)
	{
		std::shared_ptr<Model> mesh = std::make_shared<Model>();
		if (!mesh->Init(filePath))
		{
			IE_CORE_ERROR("MeshManager::LoadMeshFromFile Failed to load mesh from file: {0}", filePath);
			return false;
		}
		m_Models.push_back(mesh);
		return true;
	}

	void ModelManager::FlushModelCache()
	{
		for (std::shared_ptr<Model>& model : m_Models)
		{
			model->Destroy();
			model.reset();
		}
		m_Models.clear();
	}


}

