#pragma once

#include "UIReflect.hpp"

class UIManager {
public:
	UIManager(Renderer& renderer, World& world)
		:renderer{renderer}, world{world}
	{

	}

	UIEntity createFrame(UIFrame&& frame = UIFrame())
	{
		return frames.create(frame);
	}
	void destroyFrame(UIEntity index);
	void destroyFrame(std::string_view name);

	void setAlias(std::string_view alias, int index) 
	{ 
		aliasToEntity[alias] = index;
		entityToAlias[index] = alias;
	}
	bool hasAlias(UIEntity index)
	{
		return entityToAlias.contains(index);
	}
	const std::string_view& getAlias(UIEntity index)
	{
		return entityToAlias[index];
	}
	UIFrame& getFrame(UIEntity index)
	{
		if (frames.contains(index)) {
			return frames.get(index);
		}
		else {
			throw new std::exception("invalid access");
		}
	}
	UIFrame& getFrame(std::string_view name)
	{
		auto iter = aliasToEntity.find(name);
		if (iter != aliasToEntity.end()) {
			return getFrame(iter->second);
		}
		else {
			throw new std::exception("invalid alias");
		}
	}

	void update();
	void draw(UIContext context);

	template<typename T> UIContainer<T>& getElementContainer()
	{
		return std::get<getUIElementTypeIndex<T>()>(uiElementTuple);
	}
	template<typename T> UIEntity createElement(const T& element)
	{
		return getElementContainer<T>().create(element);
	}
	template<typename T> UIEntity createElement(T&& element)
	{
		return getElementContainer<T>().create(element);
	}
	template<typename T> void destroyElement(UIEntity index)
	{
		return getElementContainer<T>().destroy(index);
	}
	template<typename T> T& getElement(UIEntity index)
	{
		return getElementContainer<T>().get(index);
	}

	UIFrame& operator [] (std::string_view alias)
	{
		if (aliasToEntity.contains(alias)) {
			return frames.get(aliasToEntity[alias]);
		}
		else {
			throw new std::exception("invalid alias name");
		}
	}
private:

	Renderer& renderer;
	World& world;

	UIContainer<UIFrame> frames;
	robin_hood::unordered_map<std::string_view, UIEntity> aliasToEntity;
	robin_hood::unordered_map<UIEntity, std::string_view> entityToAlias;

	UIElementTuple uiElementTuple;
};