#include "JsonParser.h"

#include "World/Lights/Lights.h"


void JsonParser::openFile()
{
	m_file.open(m_fileName);
	m_file >> m_json;
	m_file.close();
}

void JsonParser::updateList()
{
	if (m_json.is_array())
	{
		for (auto& item : m_json)
		{
			FormatJson obj;
			obj.nameObj = item["j_Objet"];
			obj.pathObj = item["j_path"];
			obj.positionObj = DirectX::XMVectorSet(item["j_position"]["x"], item["j_position"]["y"],
			                                       item["j_position"]["z"], item["j_position"]["n"]);
			obj.forwardObj = DirectX::XMVectorSet(item["j_forward"]["x"], item["j_forward"]["y"],
			                                      item["j_forward"]["z"], item["j_forward"]["n"]);
			obj.scaleObj = DirectX::XMVectorSet(item["j_scale"]["x"], item["j_scale"]["y"], item["j_scale"]["z"],
			                                    item["j_scale"]["n"]);

			if (item.contains("j_typeshape"))
			{
				int type = item["j_typeshape"];

				if (item.contains("j_scaleshape"))
				{
					switch (type)
					{
					case 0:
					case 3:
						obj.scaleShape = DirectX::XMVectorSetX(
							DirectX::XMVectorSetY(
								DirectX::XMVectorSetZ(obj.scaleShape,
								                      item["j_scaleshape"]["z"]),
								item["j_scaleshape"]["y"]),
							item["j_scaleshape"]["x"]);
						break;
					case 1:
						obj.scaleShape = DirectX::XMVectorSetX(obj.scaleShape, item["j_scaleshape"]["j_radius"]);
						break;
					case 2:
						obj.scaleShape = DirectX::XMVectorSetX(
							DirectX::XMVectorSetY(obj.scaleShape,
							                      item["j_scaleshape"]["j_radius"]),
							item["j_scaleshape"]["j_halfHeight"]);
						break;
					}
				}
				else
				{
					//std::cout << "Object " << obj.nameObj << " doesn't contain scaleShape" << std::endl;
					obj.scaleShape = obj.scaleObj;
				}
				if (item.contains("j_matshape"))
				{
					if (item["j_matshape"].contains("staticFriction"))
						obj.matShape = DirectX::XMVectorSetX(obj.matShape, item["j_matshape"]["staticFriction"]);
					if (item["j_matshape"].contains("dynamicFriction"))
						obj.matShape = DirectX::XMVectorSetY(obj.matShape, item["j_matshape"]["dynamicFriction"]);
					if (item["j_matshape"].contains("restitution"))
						obj.matShape = DirectX::XMVectorSetZ(obj.matShape, item["j_matshape"]["restitution"]);
				}
				obj.scaleShape = DirectX::XMVectorSetW(obj.scaleShape, static_cast<float>(type));
				obj.matShape = DirectX::XMVectorSetW(obj.matShape, static_cast<float>(type));
			}
			else
			{
				obj.scaleShape = DirectX::XMVectorSetW(obj.scaleShape, static_cast<float>(-1));
				obj.matShape = DirectX::XMVectorSetW(obj.matShape, static_cast<float>(-1));
			}


			objs.push_back(obj);
		}
	}
	//else
		//std::cout << "Error: Json file is not an array" << std::endl;
}

void JsonParser::getCheckpoints(std::vector<FormatJson>& checkpointInfos)
{
	if (m_json.is_array())
	{
		std::for_each(m_json.begin(), m_json.end(), [&](auto& item)
		{
			FormatJson obj;
			obj.positionObj = DirectX::XMVectorSet(item["j_position"]["x"], item["j_position"]["y"],
			                                       item["j_position"]["z"], item["j_position"]["n"]);
			obj.scaleObj = DirectX::XMVectorSet(item["j_scale"]["x"], item["j_scale"]["y"], item["j_scale"]["z"],
			                                    item["j_scale"]["n"]);
			obj.forwardObj = DirectX::XMVectorSet(item["j_forward"]["x"], item["j_forward"]["y"],
			                                      item["j_forward"]["z"], item["j_forward"]["n"]);
			checkpointInfos.push_back(obj);
		});
	}
	//else
		//std::cout << "Error: Json file is not an array" << std::endl;
}

void JsonParser::getControleIA(std::vector<FormatJson>& controleIAInfos)
{
	if (m_json.is_array())
	{
		std::for_each(m_json.begin(), m_json.end(), [&](auto& item) {
			FormatJson obj;
			obj.positionObj = DirectX::XMVectorSet(item["j_position"]["x"], item["j_position"]["y"], item["j_position"]["z"], item["j_position"]["n"]);
			controleIAInfos.push_back(obj);
			});

	}
	//else		//std::cout << "Error: Json file is not an array" << std::endl;
}

std::vector<FormatJson> JsonParser::getObjs()
{
	openFile();

	updateList();

	return objs;
}

std::vector<FormatJsonLight> JsonParser::getLights()
{
	std::ifstream file("res/json/Light.json");

	nlohmann::json data;

	std::vector<FormatJsonLight> m_lights{};
	if(file.is_open())
	{
		file >> data;
	}
	else
	{
		//std::cout << "Error: Json file is not open" << std::endl;
	}

	const auto& dl = data["DL"];
	if (dl.is_array())
		for(auto& item : dl)
		{
			FormatJsonLight obj{};
			obj.direction = DirectX::XMVectorSet(item["Direction"]["x"], item["Direction"]["y"], item["Direction"]["z"], item["Direction"]["n"]);
			obj.ambiant = DirectX::XMVectorSet(item["Ambiant"]["r"], item["Ambiant"]["g"], item["Ambiant"]["b"], item["Ambiant"]["a"]);
			obj.diffuse = DirectX::XMVectorSet(item["Diffuse"]["r"], item["Diffuse"]["g"], item["Diffuse"]["b"], item["Diffuse"]["a"]);
			obj.strength = item["Strength"];
			obj.type = TypeLight::DIRECTIONAL;
			m_lights.push_back(obj);
		}

	const auto& pl = data["PL"];

	if (pl.is_array())
	for(auto& item : pl)
	{
		FormatJsonLight obj{};
		obj.distance = item["Distance"];
		obj.position = DirectX::XMVectorSet(item["Position"]["x"], item["Position"]["y"], item["Position"]["z"], item["Position"]["n"]);
		obj.ambiant = DirectX::XMVectorSet(item["Ambiant"]["r"], item["Ambiant"]["g"], item["Ambiant"]["b"], item["Ambiant"]["a"]);
		obj.diffuse = DirectX::XMVectorSet(item["Diffuse"]["r"], item["Diffuse"]["g"], item["Diffuse"]["b"], item["Diffuse"]["a"]);
		obj.falloff = item["Falloff"];
		obj.specular = item["Specular"];
		obj.range = computeRangeFromDistance(obj.distance);
		obj.type = TypeLight::POINT;
		m_lights.push_back(obj);
	}

	const auto& sl = data["SL"];

	if (sl.is_array())
	for(auto& item : sl)
	{
		FormatJsonLight obj{};
		obj.direction = DirectX::XMVectorSet(item["Direction"]["x"], item["Direction"]["y"], item["Direction"]["z"], item["Direction"]["n"]);
		obj.ambiant = DirectX::XMVectorSet(item["Ambiant"]["r"], item["Ambiant"]["g"], item["Ambiant"]["b"], item["Ambiant"]["a"]);
		obj.diffuse = DirectX::XMVectorSet(item["Diffuse"]["r"], item["Diffuse"]["g"], item["Diffuse"]["b"], item["Diffuse"]["a"]);
		obj.position = DirectX::XMVectorSet(item["Position"]["x"], item["Position"]["y"], item["Position"]["z"], item["Position"]["n"]);
		obj.falloff = item["Falloff"];
		obj.radius = item["Radius"];
		obj.strength = item["Strength"];
		obj.specular = item["Specular"];
		obj.type = TypeLight::SPOTLIGHT;
		m_lights.push_back(obj);
	}

	file.close();
	return m_lights;
}

