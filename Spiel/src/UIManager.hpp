#pragma once

#include "UIReflect.hpp"
#include "InputManager.hpp"
#include "utils.hpp"

class UIManager {
public:
	UIManager(Renderer& renderer, InputManager& in)
		:renderer{renderer}, in{ in }
	{ }

	UIEntity createFrame(UIFrame frame)
	{
		return getElementContainer<UIFrame>().create(frame);
	}
	UIEntity createFrame(UIFrame frame, std::string_view alias)
	{
		auto uient = getElementContainer<UIFrame>().create(frame);
		setAlias(alias, uient);
		return uient;
	}
	void destroyFrame(UIEntity index);
	void destroyFrame(std::string_view name);
	bool doesFrameExist(UIEntity ent);
	void setAlias(std::string_view alias, int index) 
	{ 
		if (aliasToEntity.contains(alias) || entityToAlias.contains(index)) {
			throw new std::exception("ERROR: the alias to entity relationships MUST be injectiv!");
		}
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
	bool exists(std::string_view alias) const
	{
		return aliasToEntity.contains(alias);
	}
	UIFrame& getFrame(UIEntity index)
	{
		if (getElementContainer<UIFrame>().contains(index)) {
			return getElementContainer<UIFrame>().get(index);
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
	template<typename T> T* getElement(UIEntity index)
	{
		return &getElementContainer<T>().get(index);
	}
	template<typename T> UIElement* createAndGet(const T& element)
	{
		return getElement<T>(createElement<T>(element));
	}
	template<typename T> bool doesElementExist(UIEntity entity)
	{
		return getElementContainer<T>().contains(entity);
	}

	UIFrame& operator [] (std::string_view alias)
	{
		if (aliasToEntity.contains(alias)) {
			return getElementContainer<UIFrame>().get(aliasToEntity[alias]);
		}
		else {
			throw new std::exception("invalid alias name");
		}
	}

	size_t elementCount() const;

	size_t activeElementCount() const { return lastUpdateActiveElements; }

	/*
	* retuns the amount of drawables commited to the renderer last update
	*/
	size_t drawCount() const { return lastUpdateDrwawbleCount; }
private:
	void perEntityUpdate();

	void focusUpdate();

	void clickableUpdate();
	 
	void postUpdate();

	// buffers:
	std::vector<UIFocusable*> focusedElementCandidates;	// buffer for focused element candidates used in focusUpdate()

	// fields:
	size_t lastUpdateActiveElements{ 0 };
	size_t lastUpdateDrwawbleCount{ 0 };
	Renderer& renderer; 
	InputManager& in;
	robin_hood::unordered_map<std::string_view, UIEntity> aliasToEntity;
	robin_hood::unordered_map<UIEntity, std::string_view> entityToAlias;
	UIElementTuple uiElementTuple;
};