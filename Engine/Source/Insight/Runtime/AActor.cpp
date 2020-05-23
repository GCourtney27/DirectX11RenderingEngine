#include <ie_pch.h>

#include "Insight/Runtime/Components/Actor_Component.h"
#include "AActor.h"
#include "imgui.h"

namespace Insight {



	AActor::AActor(ActorId id, ActorName actorName)
		: m_id(id)
	{
		SceneNode::SetDisplayName(actorName);
	}

	AActor::~AActor()
	{
	}

	// Draw the heirarchy of the actor and its children to ImGui
	void AActor::RenderSceneHeirarchy()
	{
		if (ImGui::TreeNode(SceneNode::GetDisplayName()))
		{
			SceneNode::RenderSceneHeirarchy();
			size_t numComponents = m_Components.size();
			for (size_t i = 0; i < numComponents; ++i)
			{
				m_Components[i]->RenderSceneHeirarchy();
			}
			ImGui::TreePop();
			ImGui::Spacing();
		}
	}

	bool AActor::OnInit()
	{
		SceneNode::OnInit();
		return true;
	}

	bool AActor::OnPostInit()
	{
		SceneNode::OnPostInit();

		return true;
	}

	void AActor::OnUpdate(const float& deltaMs)
	{
		SceneNode::OnUpdate(deltaMs);


		size_t numComponents = m_Components.size();
		for (size_t i = 0; i < numComponents; ++i)
		{
			m_Components[i]->OnUpdate(deltaMs);
		}
	}

	void AActor::OnPreRender(XMMATRIX& parentMatrix)
	{
		if (m_Parent) {
			GetTransformRef().SetWorldMatrix(XMMatrixMultiply(m_Parent->GetTransform().GetWorldMatrix(), GetTransformRef().GetLocalMatrix()));
		}
		else {
			GetTransformRef().SetWorldMatrix(GetTransformRef().GetLocalMatrix());
		}

		// Render Children
		SceneNode::OnPreRender(GetTransformRef().GetWorldMatrixRef());
		// Render Components
		size_t numComponents = m_Components.size();
		for (size_t i = 0; i < numComponents; ++i) {
			m_Components[i]->OnPreRender(GetTransformRef().GetWorldMatrixRef());
		}
	}

	void AActor::OnRender()
	{
		// Render Children
		SceneNode::OnRender();

		// Render Components
		for (size_t i = 0; i < m_NumComponents; ++i) {
			m_Components[i]->OnRender();
		}
	}

	void AActor::BeginPlay()
	{
		SceneNode::BeginPlay();

	}

	void AActor::Tick(const float& deltaMs)
	{
		SceneNode::Tick(deltaMs);

	}

	void AActor::Exit()
	{
		SceneNode::Exit();

	}

	void AActor::Destroy()
	{
		SceneNode::Destroy();

	}

	void AActor::AddComponent(StrongActorComponentPtr pComponent)
	{
		IE_CORE_ASSERT(pComponent, "Attempting to add component to actor that is NULL");
		IE_CORE_ASSERT(std::find(m_Components.begin(), m_Components.end(), pComponent) == m_Components.end(), "Failed to add component to Actor");

		pComponent->OnAttach();
		m_Components.push_back(pComponent);
		m_NumComponents++;
	}

	void AActor::RemoveComponent(StrongActorComponentPtr component)
	{
		IE_CORE_ASSERT(std::find(m_Components.begin(), m_Components.end(), component) != m_Components.end(), "Could not find Component in Actor list while attempting to delete");

		auto iter = std::find(m_Components.begin(), m_Components.end(), component);
		(*iter)->OnDetach();
		(*iter)->OnDestroy();
		(*iter).reset();
		m_Components.erase(iter);
		m_NumComponents--;
	}

}

