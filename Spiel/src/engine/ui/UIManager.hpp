#pragma once

#include "UIReflect.hpp"
#include "../io/InputManager.hpp"
#include "../util/utils.hpp"

class UIManager : public UIElementPrivilage {
public:
	UIManager(Renderer& renderer, InputManager& in)
		:renderer{renderer}, in{ in }
	{ }

	void update();

	void draw(UIContext context);

	size_t elementCount() const;

	size_t activeElementCount() const { return lastUpdateActiveElements; }

	size_t drawCount() const { return lastUpdateDrwawbleCount; }

	//
	// Frame functions:
	//

	UIEntityHandle createFrame(UIFrame frame)
	{
		return getElementContainer<UIFrame>().createAsHandle(frame);
	}
	UIEntityHandle createFrame(UIFrame frame, std::string_view alias)
	{
		auto uient = getElementContainer<UIFrame>().createAsHandle(frame);
		setAlias(alias, uient);
		return uient;
	}
	UIFrame& getFrame(UIEntityHandle entity);
	UIFrame& getFrame(std::string_view name);
	void destroyFrame(UIEntityHandle frame);
	void destroyFrame(std::string_view name);
	bool doesFrameExist(UIEntityHandle frame);
	bool doesFrameExist(std::string_view alias) const
	{
		return aliasToEntity.contains(alias);
	}
	void setAlias(std::string_view alias, UIEntityHandle entity)
	{ 
		auto index = entity.index;
		if (aliasToEntity.contains(alias) || entityToAlias.contains(index)) {
			throw new std::exception("ERROR: the alias to entity relationships MUST be injectiv!");
		}
		aliasToEntity[alias] = index;
		entityToAlias[index] = alias;
	}
	bool hasAlias(UIEntityHandle entity)
	{
		return entityToAlias.contains(entity.index);
	}
	const std::string_view& getAlias(UIEntityHandle entity)
	{
		return entityToAlias[entity.index];
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

	//
	// UIElement functions:
	//

	template<typename T> UIEntityHandle createElement(const T& element)
	{
		return getElementContainer<T>().createAsHandle(element);
	}
	template<typename T> T* getElementPtr(UIEntityHandle entity)
	{
		return &getElementContainer<T>().get(entity);
	}
	template<typename T> T& getElement(UIEntityHandle entity)
	{
		return getElementContainer<T>().get(entity);
	}
	template<typename T> UIElement* createAndGetPtr(const T& element)
	{
		return getElementPtr<T>(createElement<T>(element));
	}
	template<typename T> bool doesElementExist(UIEntityHandle entity)
	{
		return getElementContainer<T>().contains(entity);
	}
private:
	template<typename T> UIContainer<T>& getElementContainer()
	{
		constexpr auto index = index_in_ui_storagetuple_fn<0, T, UIElementTuple>();
		return std::get<index>(uiElementTuple);
	}

	template<typename T> T& getElement(UIEntityIndex index)
	{
		return getElementContainer<T>().get(index);
	}

	void perEntityUpdate();

	void focusUpdate();

	void clickableUpdate();

	void clearImmediates();

	// buffers:
	std::vector<UIFocusable*> focusedElementCandidates;	// buffer for focused element candidates used in focusUpdate()

	// fields:
	size_t lastUpdateActiveElements{ 0 };
	size_t lastUpdateDrwawbleCount{ 0 };
	Renderer& renderer; 
	InputManager& in;
	robin_hood::unordered_map<std::string_view, UIEntityIndex> aliasToEntity;
	robin_hood::unordered_map<UIEntityIndex, std::string_view> entityToAlias;
	UIElementTuple uiElementTuple;
};