#pragma once
#include <vector>

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void addComponent() = 0;
	virtual void setComponent(int index, void* component) = 0;
	virtual void* getComponent(int index) = 0;
	virtual void* getComponentArray() = 0;
	virtual int size() = 0;
};

template<typename T>
class ComponentArray : public IComponentArray
{
public:
	ComponentArray()
	{

	}

	void addComponent() override
	{
		T* comp = new T{};
		m_Components.push_back(*comp);
	}

	void setComponent(int index, void* component) override
	{
		m_Components.at(index) = *static_cast<T*>(component);
	}

	int size() override
	{
		return m_Components.size();
	}

	void* getComponent(int index) override
	{
		return &m_Components.at(index);
	}

	void* getComponentArray() override
	{
		return m_Components.data();
	}

private:
	std::vector<T> m_Components;
}; 