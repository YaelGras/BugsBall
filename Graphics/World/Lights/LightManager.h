#pragma once


#include <optional>
#include <string>
#include <vector>

#include "imgui.h"
#include "Lights.h"


class LightManager 
{
public:


	// -- Helper methods to go from c++ lights to directX-usable format
	static hlsl_GenericLight spotToGeneric(const SpotLight& sl);
	static hlsl_GenericLight pointToGeneric(const PointLight& pl);
	static hlsl_GenericLight dirToGeneric(const DirectionalLight& dl);


public:


	[[maybe_unused]] void addDirlight(DirectionalLight&& pl) {}
	void addSpotlight(SpotLight&& sl)	{ m_spots.push_back(sl); }
	void addPointlight(PointLight&& pl)	{ m_point.push_back(pl); }



	std::vector<hlsl_GenericLight> convertLights() const noexcept;
	std::vector<std::string> getAllLightsDebugName() const;

public:

	std::optional<hlsl_GenericLight> showDebugWindow()
	{
		ImGui::Begin("Lights");
		if(ImGui::Button("Add new point light"))
		{

			m_point.push_back(PointLight{ 3 ,{}, {1,1,1}, {1,1,1}, 1, true });
		}
		if (ImGui::Button("Add new spot light"))
		{
			SpotLight sl;
			sl.insideAngle = 1;
			sl.direction = { 0,-1,0 };
			sl.position = {};
			sl.outsideAngle = 1.f;
			sl.strength = 1.F;
			sl.ambiant = { 1,1,1 };
			sl.diffuse= { 1,1,1 };
			sl.specularFactor = 1.F;
			m_spots.push_back(sl);
		}



		static std::vector<std::string> names;
		static std::string lastName = "No light selected";
		names = getAllLightsDebugName();
		names.insert(names.begin() + 0, "None");
		static const char* current_item = nullptr;
		static int currentItemIndex = -1;
		current_item = lastName.c_str();

		if (names.empty())
		{
			ImGui::End();
			return std::nullopt;
		}

		if (ImGui::BeginCombo("All lights", current_item, ImGuiComboFlags_NoArrowButton ))
		{
			for (int n = 0; n < names.size(); n++)
			{
				bool is_selected = (currentItemIndex == n); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(names[n].c_str(), is_selected))
				{					
					currentItemIndex = n;
					lastName = names[n];
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}

		std::optional<hlsl_GenericLight> returnLight = std::nullopt;
		if (currentItemIndex >= 0 && !names.empty())
		{			
			ImGui::BeginChild("Current Selected");

			switch (names[currentItemIndex][0])
			{
				case 'N' :
					break;
				case 'P':
					{						
						size_t tmpIndex = currentItemIndex - 1;
						ImGui::DragFloat3(("Position ##" + std::to_string(tmpIndex)).c_str(), &m_point[tmpIndex].position.vector4_f32[0]);
						if (ImGui::DragInt(("Distance ##" + std::to_string(tmpIndex)).c_str(), &m_point[tmpIndex].distance, 1, 0, 11))
						{
							m_point[tmpIndex].range = computeRangeFromDistance(m_point[tmpIndex].distance);
						
						}
						ImGui::BeginDisabled();
						ImGui::DragFloat4(("Range ##" + std::to_string(tmpIndex)).c_str(), m_point[tmpIndex].range.vector4_f32);
						ImGui::EndDisabled();
						ImGui::DragFloat(("specularFactor ##" + std::to_string(tmpIndex)).c_str(), &m_point[tmpIndex].specularFactor);
						ImGui::DragFloat3(("Ambiant ##" + std::to_string(tmpIndex)).c_str(), &m_point[tmpIndex].ambiant.vector4_f32[0]);
						ImGui::DragFloat3(("Diffuse ##" + std::to_string(tmpIndex)).c_str(), &m_point[tmpIndex].diffuse.vector4_f32[0]);
						ImGui::Checkbox(("IsOn ##" + std::to_string(tmpIndex)).c_str(), &m_point[tmpIndex].isOn);
						returnLight = pointToGeneric(m_point[tmpIndex]);
					break;
					}
				case 'S':
					{
						
					size_t tmpIndex = currentItemIndex - m_point.size() -1 ;
					SpotLight& sl = m_spots[tmpIndex];
					
						

					ImGui::DragFloat3(("Position ##" + std::to_string(currentItemIndex)).c_str(), &sl.position.vector4_f32[0]);
					if (ImGui::DragFloat3(("Direction ##" + std::to_string(currentItemIndex)).c_str(), &sl.direction.vector4_f32[0], 0.01f, -1.f, 1.f))
					{
						sl.direction = XMVector3Normalize(sl.direction);
					}
					ImGui::DragFloat3(("Ambiant ##" + std::to_string(currentItemIndex)).c_str(), &sl.ambiant.vector4_f32[0]);
					ImGui::DragFloat3(("Diffuse ##" + std::to_string(currentItemIndex)).c_str(), &sl.diffuse.vector4_f32[0]);

					ImGui::DragFloat(("specularFactor ##" + std::to_string(currentItemIndex)).c_str(), &sl.specularFactor);
					ImGui::DragFloat(("Inside Angle (radius) ##" + std::to_string(currentItemIndex)).c_str(), &sl.insideAngle, 0.01f, 0.F, XM_PIDIV2);					
					ImGui::DragFloat(("Outside angle (falloff) ##" + std::to_string(currentItemIndex)).c_str(), &sl.outsideAngle, 0.01f, 0.F, XM_PIDIV2);
					ImGui::DragFloat(("Strength ##" + std::to_string(currentItemIndex)).c_str(), &sl.strength);

					ImGui::Checkbox(("IsOn ##" + std::to_string(currentItemIndex)).c_str(), &m_spots[tmpIndex].isOn);

					returnLight = spotToGeneric(sl);

					break;

					}
				case 'D':
					break;
			default:
				break;
			}

			ImGui::EndChild();
		}

		ImGui::End();
		return returnLight;
	}

	auto& getAllSpotLights() { return m_spots; }
	LightManager() = default;
	~LightManager() = default;


private:

	std::vector<DirectionalLight> m_dirs; // unused, sun is hard coded in shader
	std::vector<PointLight> m_point;
	std::vector<SpotLight> m_spots;


};