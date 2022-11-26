#pragma once
#include <unordered_map>
#include <typeinfo>
#include <memory>
#include "ComponentArray.h"
#include "Component/Renderable.h"

class ComponentManager
{
public:
	ComponentManager();
	~ComponentManager();

	template<typename T>
	void RegisterComponent()
	{
		m_ComponentArrays.insert({ typeid(T).name(), std::make_shared<ComponentArray<T>>() });
	}

	void AddEntity()
	{
		for (auto& item : m_ComponentArrays)
		{
			item.second->addComponent();
		}
	}

	int getNumberOfComponentArrays()
	{
		return m_ComponentArrays.size();
	}

	template<typename T>
	int getComponentArraySize()
	{
		return m_ComponentArrays.at(typeid(T).name())->size();
	}

	template<typename T>
	T getEntityComponent(int entityId)
	{
		return *static_cast<T*>(m_ComponentArrays.at(typeid(T).name())->getComponent(entityId));
	}

	template<typename T>
	void changeEntityComponent(int entityId, T component)
	{
		m_ComponentArrays.at(typeid(T).name())->setComponent(entityId, &component);
	}

	template<typename T>
	T* getComponentArray()
	{
		return static_cast<T*>(m_ComponentArrays.at(typeid(T).name())->getComponentArray());
	}

private:
	//! This should be replaced with a memory area for faster allocation
	//! For now will work
	
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_ComponentArrays;
};