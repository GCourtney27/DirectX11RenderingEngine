#include <Engine_pch.h>

#include "APost_Fx.h"

#include "Insight/Runtime/Components/Actor_Component.h"
#include "Insight/Core/Application.h"

#include "Insight/UI/UI_Lib.h"


namespace Insight {




	APostFx::APostFx(ActorId id, Runtime::ActorType type)
		: AActor(id, type)
	{
		Renderer::AddPostFxActor(this);
		m_ShaderCB.blBrightnessThreshold = XMFLOAT3{ 0.2126f, 0.7152f, 0.0722f };
	}

	APostFx::~APostFx()
	{
	}

	bool APostFx::LoadFromJson(const rapidjson::Value* JsonPostFx)
	{
		AActor::LoadFromJson(JsonPostFx);

		bool vnEnabled, caEnabled, fgEnabled, blEnabled;

		const rapidjson::Value& postFx = (*JsonPostFx)["PostFx"];

		const rapidjson::Value& vignette = postFx[0];
		json::get_float(vignette, "vnInnerRadius", m_TempInnerRadius);
		json::get_float(vignette, "vnOuterRadius", m_TempOuterRadius);
		json::get_float(vignette, "vnOpacity", m_ShaderCB.vnOpacity);
		json::get_bool(vignette, "vnEnabled", vnEnabled);

		const rapidjson::Value& filmGrain = postFx[1];
		json::get_float(filmGrain, "fgStrength", m_ShaderCB.fgStrength);
		json::get_bool(filmGrain, "fgEnabled", fgEnabled);

		const rapidjson::Value& chromAb = postFx[2];
		json::get_float(chromAb, "caIntensity", m_ShaderCB.caIntensity);
		json::get_bool(chromAb, "caEnabled", caEnabled);

		const rapidjson::Value& Bloom = postFx[3];
		json::get_float(Bloom, "blIntensity", m_ShaderCB.blCombineCoefficient);
		json::get_bool(Bloom, "blEnabled", blEnabled);

		m_ShaderCB.vnEnabled = static_cast<int>(vnEnabled);
		m_ShaderCB.fgEnabled = static_cast<int>(fgEnabled);
		m_ShaderCB.caEnabled = static_cast<int>(caEnabled);
		m_ShaderCB.blEnabled = static_cast<int>(blEnabled);
		m_ShaderCB.vnInnerRadius = m_TempInnerRadius;
		m_ShaderCB.vnOuterRadius = m_TempOuterRadius;
		
		return true;
	}

	bool APostFx::WriteToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>* Writer)
	{
		// TODO this work should be done in the base Actor class

		Writer->StartObject(); // Start Write Actor
		{
			Writer->Key("Type");
			Writer->String("PostFxVolume");

			Writer->Key("DisplayName");
			Writer->String(SceneNode::GetDisplayName());

			Writer->Key("Transform");
			Writer->StartArray(); // Start Write Transform
			{
				//ieTransform& Transform = SceneNode::GetTransformRef();
				//ieVector3 Pos = Transform.GetPosition();
				//ieVector3 Rot = Transform.GetRotation();
				//ieVector3 Sca = Transform.GetScale();

				//Writer->StartObject();
				//// Position
				//Writer->Key("posX");
				//Writer->Double(Pos.x);
				//Writer->Key("posY");
				//Writer->Double(Pos.y);
				//Writer->Key("posZ");
				//Writer->Double(Pos.z);
				//// Rotation
				//Writer->Key("rotX");
				//Writer->Double(Rot.x);
				//Writer->Key("rotY");
				//Writer->Double(Rot.y);
				//Writer->Key("rotZ");
				//Writer->Double(Rot.z);
				//// Scale
				//Writer->Key("scaX");
				//Writer->Double(Sca.x);
				//Writer->Key("scaY");
				//Writer->Double(Sca.y);
				//Writer->Key("scaZ");
				//Writer->Double(Sca.z);

				Writer->EndObject();
			}
			Writer->EndArray(); // End Write Transform

			// Post-Fx Volume Attributes
			Writer->Key("PostFx");
			Writer->StartArray();
			{
				Writer->StartObject(); // Start Vignette
				{
					Writer->Key("vnInnerRadius");
					Writer->Double(m_ShaderCB.vnInnerRadius);
					Writer->Key("vnOuterRadius");
					Writer->Double(m_ShaderCB.vnOuterRadius);
					Writer->Key("vnOpacity");
					Writer->Double(m_ShaderCB.vnOpacity);
					Writer->Key("vnEnabled");
					Writer->Bool(m_ShaderCB.vnEnabled);
				}
				Writer->EndObject(); // End Vignette
				Writer->StartObject(); // Start Film Grain
				{
					Writer->Key("fgStrength");
					Writer->Double(m_ShaderCB.fgStrength);
					Writer->Key("fgEnabled");
					Writer->Bool(m_ShaderCB.fgEnabled);
				}
				Writer->EndObject(); // End Film Grain
				Writer->StartObject(); // Start Chromatic Abberation
				{
					Writer->Key("caIntensity");
					Writer->Double(m_ShaderCB.caIntensity);
					Writer->Key("caEnabled");
					Writer->Bool(m_ShaderCB.caEnabled);
				}
				Writer->EndObject(); // End Chromatic Abberation
				Writer->StartObject(); // Start Chromatic Abberation
				{
					Writer->Key("blIntensity");
					Writer->Double(m_ShaderCB.blCombineCoefficient);
					Writer->Key("blEnabled");
					Writer->Bool(m_ShaderCB.blEnabled);
				}
				Writer->EndObject(); // End Chromatic Abberation
			}
			Writer->EndArray();

			Writer->Key("Subobjects");
			Writer->StartArray(); // Start Write SubObjects
			{
				for (size_t i = 0; i < m_NumComponents; ++i)
				{
					AActor::m_Components[i]->WriteToJson(*Writer);
				}
			}
			Writer->EndArray(); // End Write SubObjects
		}
		Writer->EndObject(); // End Write Actor
		return true;
	}

	bool APostFx::OnInit()
	{
		return true;
	}

	bool APostFx::OnPostInit()
	{
		return true;
	}

	void APostFx::OnUpdate(const float DeltaMs)
	{

	}

	void APostFx::OnPreRender(XMMATRIX parentMat)
	{
	}

	void APostFx::OnRender()
	{
	}

	void APostFx::Destroy()
	{
	}

	void APostFx::OnEvent(Event& e)
	{
	}

	void APostFx::BeginPlay()
	{
	}

	void APostFx::Tick(const float DeltaMs)
	{
	}

	void APostFx::Exit()
	{
	}

	void APostFx::OnImGuiRender()
	{
		AActor::OnImGuiRender();

		UI::Text("Vignette");
		UI::Spacing();
		UI::Checkbox("vnEnabled", (bool*)&m_ShaderCB.vnEnabled);
		UI::DragFloat("Inner Radius", &m_TempInnerRadius, 0.1f, 0.0f, 50.0f);
		UI::DragFloat("Outer Radius", &m_TempOuterRadius, 0.1f, 0.0f, 50.0f);
		UI::DragFloat("Opacity", &m_ShaderCB.vnOpacity, 0.15f, 0.0f, 10.0f);
		if (m_TempInnerRadius > m_TempOuterRadius) {
			m_TempInnerRadius = m_TempOuterRadius;
		}
		m_ShaderCB.vnInnerRadius = m_TempInnerRadius;
		m_ShaderCB.vnOuterRadius = m_TempOuterRadius;
		UI::Spacing();

		UI::Text("Bloom");
		UI::Checkbox("blEnabled", (bool*)&m_ShaderCB.blEnabled);
		UI::DragFloat("blStrength", &m_ShaderCB.blCombineCoefficient, 0.1f, 0.0f, 80.0f);

		UI::Text("Film Grain");
		UI::Checkbox("fgEnabled", (bool*)&m_ShaderCB.fgEnabled);
		UI::DragFloat("Strength", &m_ShaderCB.fgStrength, 0.1f, 0.0f, 80.0f);
		UI::Spacing();

		UI::Text("Chromatic Aberration");
		UI::Checkbox("caEnabled", (bool*)&m_ShaderCB.caEnabled);
		UI::DragFloat("Intensity", &m_ShaderCB.caIntensity, 0.1f, 0.0f, 80.0f);
	}

}