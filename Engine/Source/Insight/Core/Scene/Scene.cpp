#include <ie_pch.h>

#include "Scene.h"
#include "Insight/Core/Application.h"
#include "Insight/Runtime/Components/Static_Mesh_Component.h"
#include "imgui.h"

namespace Insight {


	Scene::Scene()
		: m_pRoot(new SceneNode("Scene Root"))
	{

	}

	Scene::~Scene()
	{
		Destroy();
	}

	bool Scene::Init()
	{
		// TODO: Init from file
		m_Renderer = Application::Get().GetWindow().GetRenderContext();
		m_ModelManager.Init();
		//m_ModelManager.LoadMeshFromFile("../Assets/Models/nanosuit/nanosuit.obj");

		m_pTestActor = new AActor(0, "Test actor 1");// TODO: make the id be its index in the scene
		m_pTestActor->CreateDefaultSubobject<StaticMeshComponent>();
		m_pTestActor2 = new AActor(1, "Test actor 2");// TODO: make the id be its index in the scene
		m_pTestActor2->CreateDefaultSubobject<StaticMeshComponent>();

		//m_pModel = new Model();
		//m_pModel->Init("../Assets/Objects/nanosuit/nanosuit.obj");
		//m_pTestActor->AddChild(m_pTestActor2);

		//m_pRoot->AddChild(m_pModel);
		m_pRoot->AddChild(m_pTestActor);
		m_pRoot->AddChild(m_pTestActor2);
		
		reinterpret_cast<Direct3D12Context*>(m_Renderer.get())->CloseCommandListAndSignalCommandQueue();// Very uber doober temp
		return true;
	}

	void Scene::OnUpdate(const float& deltaMs)
	{
		m_Renderer->OnUpdate(deltaMs);
		if (m_pRoot)
		{
			m_pRoot->OnUpdate(deltaMs);
		}
	}

	void Scene::OnImGuiRender()
	{
		RenderSceneHeirarchy();
		RenderInspector();
	}

	void Scene::RenderSceneHeirarchy()
	{
		ImGui::Begin("Scene Heirarchy");
		{
			if (ImGui::CollapsingHeader(m_pRoot->GetDisplayName(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				m_pRoot->RenderSceneHeirarchy();
			}
		}
		ImGui::End();
		
	}

	void Scene::RenderInspector()
	{
		ImGui::Begin("Inspector");
		{
			float xOffset = 0.0;
			ImGui::Text("TestActor1");
			ImGui::DragFloat("position", &xOffset, 0.01f, -100.0f, 100.0f);
			m_pTestActor->GetTransformRef().Translate(xOffset, 0.0f, 0.0f);
			//ImGui::Text("TestActor2");
			//ImGui::DragFloat3("position", &m_pTestActor2->GetTransformRef().GetPositionRef().x, 0.01f, -100.0f, 100.0f);
			// TODO: If an object is selected in the scene heirarchy graph
			//		or through object ray cast picking, display its info in this panel
		}
		ImGui::End();
	}

	void Scene::OnPreRender()
	{
		m_Renderer->OnPreFrameRender();
		m_pRoot->OnPreRender(XMMatrixIdentity());
		m_ModelManager.UploadVertexDataToGPU();
	}

	void Scene::OnRender()
	{
		m_Renderer->OnRender();
		m_ModelManager.Draw();
		//m_pRoot->OnRender();
	}

	void Scene::OnPostRender()
	{
		m_Renderer->ExecuteDraw();
		m_Renderer->SwapBuffers();
	}

	void Scene::Destroy()
	{
		delete m_pRoot;
	}


}
