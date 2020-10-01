#include <Engine_pch.h>

#include "ADirectional_Light.h"

#include "Retina/Runtime/Components/Actor_Component.h"
#include "Renderer/Renderer.h"
#include "Retina/Runtime/ACamera.h"
#include "Retina/Core/Application.h"
#include "imgui.h"

namespace Retina {



	ADirectionalLight::ADirectionalLight(ActorId id, Runtime::ActorType type)
		: AActor(id, type)
	{
		Renderer::RegisterWorldDirectionalLight(this);

		m_pSceneComponent = CreateDefaultSubobject<Runtime::SceneComponent>();

		m_pSceneComponent->SetRotation(0.0f, -1.0f, -6.0f);

		m_ShaderCB.DiffuseColor = ieVector3(1.0f, 1.0f, 1.0f);
		m_ShaderCB.Direction = m_pSceneComponent->GetRotation();
		m_ShaderCB.Strength = 8.0f;
		m_ShaderCB.ShadowDarknessMultiplier = 0.6f;

		m_NearPlane = 1.0f;
		m_FarPlane = 210.0f;
	}

	ADirectionalLight::~ADirectionalLight()
	{
		Renderer::UnRegisterWorldDirectionalLight();
	}

	bool ADirectionalLight::LoadFromJson(const rapidjson::Value* jsonDirectionalLight)
	{
		AActor::LoadFromJson(jsonDirectionalLight);

		float DiffuseR, DiffuseG, DiffuseB, Strength, ShdowDarkness;
		const rapidjson::Value& emission = (*jsonDirectionalLight)["Emission"];
		json::get_float(emission[0], "diffuseR", DiffuseR);
		json::get_float(emission[0], "diffuseG", DiffuseG);
		json::get_float(emission[0], "diffuseB", DiffuseB);
		json::get_float(emission[0], "strength", Strength);
		json::get_float(emission[0], "shadowDarkness", ShdowDarkness);

		m_ShaderCB.DiffuseColor = XMFLOAT3(DiffuseR, DiffuseG, DiffuseB);
		//m_ShaderCB.Direction = AActor::GetTransformRef().GetRotationRef();
		m_ShaderCB.Strength = Strength;
		m_ShaderCB.ShadowDarknessMultiplier = ShdowDarkness;

		m_NearPlane = 1.0f;
		m_FarPlane = 210.0f;
		
		LightCamPositionOffset = XMFLOAT3(0.0f, 0.0f, 20.0f);

		//m_ShaderCB.Direction = SceneNode::GetTransformRef().GetRotation();

		//XMFLOAT3 LookAtPos = SceneNode::GetTransformRef().GetPosition();// (0.0f, 0.0f, 0.0f);
		//XMVECTOR LookAtPosVec = XMLoadFloat3(&LookAtPos);

		/*XMFLOAT3 Up(0.0f, 1.0f, 0.0f);
		XMVECTOR UpVec = XMLoadFloat3(&Up);
		XMFLOAT3 direction = m_ShaderCB.Direction;
		direction.x = -direction.x + LightCamPositionOffset.x;
		direction.y =  direction.y + LightCamPositionOffset.y;
		direction.z = -direction.z + LightCamPositionOffset.z;

		LightCamPositionVec = XMLoadFloat3(&direction);


		LightView = XMMatrixLookAtLH(LightCamPositionVec, LookAtPosVec, UpVec);
		LightProj = XMMatrixOrthographicLH(ViewWidth, ViewHeight, m_NearPlane, m_FarPlane);

		XMStoreFloat4x4(&LightViewFloat, XMMatrixTranspose(LightView));
		XMStoreFloat4x4(&LightProjFloat, XMMatrixTranspose(LightProj));

		m_ShaderCB.LightSpaceView = LightViewFloat;
		m_ShaderCB.LightSpaceProj = LightProjFloat;*/

		return true;
	}

	bool ADirectionalLight::WriteToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>* Writer)
	{
		Writer->StartObject(); // Start Write Actor
		{
			Writer->Key("Type");
			Writer->String("DirectionalLight");

			Writer->Key("DisplayName");
			Writer->String(SceneNode::GetDisplayName());

			// Directional Light Attributes
			Writer->Key("Emission");
			Writer->StartArray();
			{
				Writer->StartObject();
				Writer->Key("diffuseR");
				Writer->Double(m_ShaderCB.DiffuseColor.x);
				Writer->Key("diffuseG");
				Writer->Double(m_ShaderCB.DiffuseColor.y);
				Writer->Key("diffuseB");
				Writer->Double(m_ShaderCB.DiffuseColor.z);
				Writer->Key("strength");
				Writer->Double(m_ShaderCB.Strength);
				Writer->EndObject();
			}
			Writer->EndArray();

			Writer->Key("Subobjects");
			Writer->StartArray(); // Start Write SubObjects
			{
				for (size_t i = 0; i < m_NumComponents; ++i)
				{
					m_Components[i]->WriteToJson(*Writer);
				}
			}
			Writer->EndArray(); // End Write SubObjects
		}
		Writer->EndObject(); // End Write Actor
		return true;
	}

	bool ADirectionalLight::OnInit()
	{
		return true;
	}

	bool ADirectionalLight::OnPostInit()
	{
		return true;
	}

	void ADirectionalLight::OnUpdate(const float DeltaMs)
	{
		
	}

	void ADirectionalLight::OnPreRender(XMMATRIX parentMat)
	{
	}

	void ADirectionalLight::OnRender()
	{
	}

	void ADirectionalLight::Destroy()
	{
		Renderer::UnRegisterWorldDirectionalLight();
	}

	void ADirectionalLight::OnEvent(Event& e)
	{
		EventDispatcher Dispatcher(e);
		Dispatcher.Dispatch<TranslationEvent>(RN_BIND_EVENT_FN(ADirectionalLight::OnEventTranslation));
	}

	void ADirectionalLight::BeginPlay()
	{
	}

	void ADirectionalLight::Tick(const float DeltaMs)
	{
	}

	void ADirectionalLight::Exit()
	{
	}

	void ADirectionalLight::OnImGuiRender()
	{
		AActor::OnImGuiRender();

		ImGui::Spacing();
		ImGui::Spacing();

		/*ImGui::DragFloat3("light cam pos offset: ", &LightCamPositionOffset.x, 1.0f, 180.0f, 180.0f);
		ImGui::DragFloat("light cam near z: ", &m_NearPlane, 1.0f, 0.0f, 180.0f);
		ImGui::DragFloat("light cam far z: ", &m_FarPlane, 1.0f, 0.0f, 1000.0f);
		ImGui::DragFloat("light view width: ", &ViewWidth, 1.0f, 0.0f, 1000.0f);
		ImGui::DragFloat("light view height: ", &ViewHeight, 1.0f, 0.0f, 1000.0f);
		*/
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Emission", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGuiColorEditFlags colorWheelFlags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_PickerHueWheel;
			// Imgui will edit the color values in a normalized 0 to 1 space. 
			// In the shaders we transform the color values back into 0 to 255 space.
			ImGui::ColorEdit3("Diffuse", &m_ShaderCB.DiffuseColor.x, colorWheelFlags);
			ImGui::DragFloat("Strength", &m_ShaderCB.Strength, 0.01f, 0.0f, 10.0f);

			ImGui::Text("Shadows");
			ImGui::DragFloat("Shadow Darkness Multiplier: ", &m_ShaderCB.ShadowDarknessMultiplier, 0.05f, 0.0f, 1.0f);
		}

	}

	bool ADirectionalLight::OnEventTranslation(TranslationEvent& e)
	{
		m_ShaderCB.Direction = m_pSceneComponent->GetRotation();

		XMFLOAT3 LookAtPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMVECTOR LookAtPosVec = XMLoadFloat3(&LookAtPos);

		XMFLOAT3 Up(0.0f, 1.0f, 0.0f);
		XMVECTOR UpVec = XMLoadFloat3(&Up);
		XMFLOAT3 direction = m_ShaderCB.Direction;
		direction.x = -direction.x + LightCamPositionOffset.x;
		direction.y = direction.y + LightCamPositionOffset.y;
		direction.z = -direction.z + LightCamPositionOffset.z;
		LightCamPositionVec = XMLoadFloat3(&direction);

		LightView = XMMatrixLookAtLH(LightCamPositionVec, LookAtPosVec, UpVec);
		LightProj = XMMatrixOrthographicLH(ViewWidth, ViewHeight, m_NearPlane, m_FarPlane);

		XMStoreFloat4x4(&LightViewFloat, XMMatrixTranspose(LightView));
		XMStoreFloat4x4(&LightProjFloat, XMMatrixTranspose(LightProj));

		m_ShaderCB.LightSpaceView = LightViewFloat;
		m_ShaderCB.LightSpaceProj = LightProjFloat;
		return false;
	}

}