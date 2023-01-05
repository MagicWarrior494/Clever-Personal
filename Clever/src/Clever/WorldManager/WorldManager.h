#pragma once
#include "Components/ComponentManager.h"
#include "OS-Dependant/Vulkan/VulkanInstance.h"
#include "Object/ObjectManager.h"
#include <iostream>
#include <string>

namespace World
{

	struct WorldFlags
	{
		std::string WorldFileLocation;
	};

	class WorldManager
	{
	public:
		WorldManager()
		{

		}
		~WorldManager()
		{

		}
		void worldInit(std::shared_ptr<VulkanInstance> vulkanInstance, WorldFlags flags = {})
		{

			/*
			------------------STEPS FOR INITIALIZATION-----------------
				1. Load/Create each object's data
				2. Load into ECS
				3. DONE!
			*/


			//Registering All Components before any entities are created
			{

				componentManager.RegisterComponent<Renderable>();

			}

			Renderable loadedObject{ vulkanInstance->m_Device, vulkanInstance->m_PhysicalDevice, vulkanInstance->m_RenderPass, vulkanInstance->m_CommandPool, vulkanInstance->m_GraphicsQueue, vulkanInstance->m_UniformBuffers, vulkanInstance->m_max_frames_in_flight };

			{
				int count = 25;
				std::pair<std::vector<Vertex>, std::vector<uint16_t>> md = loadModel("D:/Clever-Personal/Clever/Clever/Resource/Models/Teapot.obj");

				componentManager.AddEntity();
				loadedObject.setComponentData(md);
				loadedObject.setInstanceCount(count);

				for (int i = 0; i < count; i++)
				{
					loadedObject.setLocation({ (i % 5), 0, (i / 5)}, i);
				}

				componentManager.changeEntityComponent(0, loadedObject);
				//componentManager.changeEntityComponent(1, loadedObject);
				//Creating a Renderable Object with needed data for a Cube
			}
		}

		Renderable* getRenderables()
		{
			return componentManager.getComponentArray<Renderable>();
		}

		int getRenderablesSize()
		{
			return componentManager.getComponentArraySize<Renderable>();
		}

		void cleanup()
		{
			
		}

	private:
		ComponentManager componentManager{};

		std::vector<Vertex> vertices = {
		//{{1.0, 1.0, -1.0}, {0.0f, 0.0f, 0.0f}},
		//{{1.0,  -1.0, -1.0}, {1.0f, 0.0f, 0.0f}},
		//{{1.0,  1.0,  1.0}, {0.0f, 1.0f, 0.0f}},
		//{{1.0,  -1.0, 1.0}, {1.0f, 1.0f, 0.0f}},
		//{{-1.0, 1.0,  -1.0}, {0.0f, 0.0f, 1.0f}},
		//{{-1.0, -1.0, -1.0}, {1.0f, 0.0f, 1.0f}},
		//{{-1.0, 1.0,  1.0}, {0.0f, 1.0f, 1.0f}},
		//{{-1.0, -1.0, 1.0}, {1.0f, 1.0f, 1.0f}}
		};

		std::vector<uint16_t> indices = {
			/*5, 3, 1,
			3, 8, 4,
			7, 6, 8,
			2, 8, 6,
			1, 4, 2,
			5, 2, 6,
			5, 7, 3,
			3, 7, 8,
			7, 5, 6,
			2, 4, 8,
			1, 3, 4,
			5, 1, 2*/
		};
	};

}