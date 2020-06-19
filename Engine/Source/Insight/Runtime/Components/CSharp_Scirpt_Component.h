#pragma once
#include <Insight/Core.h>

#include "Actor_Component.h"

#include <mono/metadata/debug-helpers.h>

namespace Insight {

	class MonoScriptManager;

	class INSIGHT_API CSharpScriptComponent : public ActorComponent
	{
	public:
		CSharpScriptComponent(StrongActorPtr pOwner);
		~CSharpScriptComponent();

		virtual bool LoadFromJson(const rapidjson::Value& jsonStaticMeshComponent) override;

		virtual void OnInit() override;
		virtual void OnPostInit();
		virtual void OnDestroy() override;
		virtual void OnUpdate(const float& deltaTime);
		virtual void OnChanged();
		virtual void OnImGuiRender() override;

		void BeginPlay();
		void Tick(const float& deltaMs);

		// Mono Interop Calls (C# --> C++)
		void Interop_Translate(float x, float y, float z);
		void Interop_Rotate(float x, float y, float z);
		void Interop_Scale(float x, float y, float z);
		void Interop_SetPosition(float x, float y, float z);
		void Interop_SetRotation(float x, float y, float z);
		void Interop_SetScale(float x, float y, float z);


		virtual void OnAttach() override;
		virtual void OnDetach() override;

	private:
		MonoScriptManager* m_pMonoScriptManager = nullptr;
		MonoClass* m_pClass = nullptr;
		MonoObject* m_pObject = nullptr;
		MonoMethod* m_pBeginPlayMethod = nullptr;
		MonoMethod* m_pUpdateMethod = nullptr;

		std::string m_ModuleName;
		bool m_CanBeTicked = true;
		bool m_CanBeCalledOnBeginPlay = true;
	};

}
