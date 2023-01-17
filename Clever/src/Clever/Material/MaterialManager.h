#pragma once
#include <fstream>
#include <iostream>
#include <glm.hpp>

#include "rapidjson/document.h"
#include <rapidjson/istreamwrapper.h>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
/*
-------------Material Stucture----------------

File Format: Json, v0.1(Will be replaced with more effient file size in future, v0.?)

Name: the name of the material, V0.1

Color: Color of Material, v0.1(Will be replaced with procedurally created texture of compounds, v0.?)

Properties: Real world material properties, v0.2
	Reflectiveness
	Roughness
	Densitya
	(Need to do more research on what propeties a material has)

MagicStructure: Same as properties but with Magic, entirly our Creation and design, v0.?

*/

namespace Material
{
	struct Material 
	{
		std::string name;
		glm::vec3 color;

		std::string toString()
		{
			return name + ": " + std::to_string(color.r) + ", " + std::to_string(color.g) + ", " + std::to_string(color.b);
		}
	};

	struct MaterialFlags
	{
		std::string materialFilePath;
	};

	class MaterialManager
	{

	public:

		class MaterialManager()
		{

		}

		void MaterialInit(MaterialFlags materialFlags)
		{
			loadMaterial(materialFlags.materialFilePath);
		}

		std::vector<Material> getMaterials()
		{
			return materialList;
		}
	private:
		void loadMaterial(std::string filePath)
		{
			using namespace rapidjson;
			std::ifstream material_file(filePath);
			IStreamWrapper streamWrapper(material_file);

			Document materials;
			materials.ParseStream(streamWrapper);

			//Material m{ materials["Stone"].GetString(), {materials["Stone"]["Color"]["r"].GetInt(),materials["Stone"]["Color"]["g"].GetInt(),materials["Stone"]["Color"]["b"].GetInt()}};

			//rapidjson::StringBuffer buffer;
			//rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			//materials.Accept(writer);
			materialList.resize(materials.MemberCount());

			int i = 0;
			for (Value::ConstMemberIterator itr = materials.MemberBegin(); itr != materials.MemberEnd(); itr++)
			{
				const auto& name = itr->value["Name"];
				const auto& color = itr->value["Color"];
				materialList.at(i) = { name.GetString(), {color["r"].GetFloat(), color["g"].GetFloat(), color["b"].GetFloat()}};
				i++;
			}
			std::cout << std::endl;
			for (Material m : materialList)
			{
				std::cout << m.toString() << std::endl;
			}
		}

	private:
		std::vector<Material> materialList;
	};
}

