#include "UIManager.hpp"

void UIManager::destroyFrame(UIEntity index)
{
	if (getElementContainer<UIFrame>().get(index).hasChild()) {
		getElementContainer<UIFrame>().get(index).getChild()->destroy();
	}

	if (entityToAlias.contains(index)) {			// remove alias
		aliasToEntity.erase(entityToAlias[index]);
		entityToAlias.erase(index);
	}

	getElementContainer<UIFrame>().destroy(index);
}

void UIManager::destroyFrame(std::string_view name)
{
	if (aliasToEntity.contains(name)) {
		int index = aliasToEntity[name];
		entityToAlias.erase(aliasToEntity[name]);	// remove alias
		aliasToEntity.erase(name);
		destroyFrame(index);
	}
	else {
		std::cerr << "WARNING: trying to delete non existant UIEntity!\n";
	}
}

bool UIManager::doesFrameExist(UIEntity ent)
{
	return getElementContainer<UIFrame>().contains(ent);
}

void UIManager::update()
{
	perEntityUpdate();
	postUpdate();
	focusUpdate();
	clickableUpdate();
}

void UIManager::draw(UIContext context)
{
	std::vector<Drawable> buffer;
	context.increaseDrawPrio();
	for (auto& ent : getElementContainer<UIFrame>()) {
		auto& frame = getElementContainer<UIFrame>().get(ent);
		if (frame.isEnabled()) {
			frame.draw(buffer, context);
		}
	}
	lastUpdateDrwawbleCount = buffer.size();
	for (auto&& d : buffer) {
		renderer.submit(d);
	}
}

size_t UIManager::elementCount() const
{
	size_t sum{ 0 };
	std_extra::tuple_for_each(uiElementTuple,
		[&](auto& container) {
			sum += container.size();
		}
	);
	return sum;
}

void UIManager::perEntityUpdate()
{
	// update
	size_t activeElements{ 0 };
	std_extra::tuple_for_each(uiElementTuple,
		[&](auto& container)
		{
			for (auto& uient : container) {
				auto& element = container.get(uient);
				element.update();
				activeElements += element.isEnabled() ? 1 : 0;
			}
		}
	);
	lastUpdateActiveElements = activeElements;
	// destroy
	std_extra::tuple_for_each(uiElementTuple,
		[](auto& container) 
		{
			for (auto& uient : container) {
				auto& element = container.get(uient);
				if (element.isDestroyed()) container.destroy(uient);
			}
		}
	);
}

void UIManager::focusUpdate()
{
	focusedElementCandidates.clear();
	// find potential elements, that could be focused:
	std_extra::tuple_for_each(uiElementTuple,
		[&](auto& container) {
			if constexpr (std::is_base_of<UIFocusable, std::remove_reference<decltype(container.get(0))>::type>::value) {
				for (auto& uient : container) {
					auto* element = &container.get(uient);
					UIFocusable* felement = static_cast<UIFocusable*>(element);
					if (felement->isEnabled() && felement->isFocusable()) {
						const auto& area = felement->getLastDrawArea();

						Vec2 cursorPos = in.getMousePosition(area.drawMode);

						// is cursor in area:
						if (	cursorPos.x <= area.drCorner.x
							&&	cursorPos.y >= area.drCorner.y
							&&	cursorPos.x >= area.ulCorner.x
							&&	cursorPos.y <= area.ulCorner.y) {
							// cursor over area
							focusedElementCandidates.push_back(felement);
						}
						else {
							// cursor not over area
							if (felement->bHoveredOver) {
								in.returnMouseFocus();
								felement->onLeave();
							}
						}
					}
					else {
						if (felement->bHoveredOver) {
							// if the focused element gets disabled, it's focus gets returnd
							in.returnMouseFocus();
							felement->bHoveredOver = false;
						}
					}
				}
			}
		}
	);

	if (!focusedElementCandidates.empty()) {
		// find element with the highest drawing prio (at the end is the higest):
		std::sort(focusedElementCandidates.begin(), focusedElementCandidates.end(),
			[](UIFocusable* a, UIFocusable* b) {
				return a->getLastDrawArea().drawingPrio < b->getLastDrawArea().drawingPrio;
			}
		);

		UIFocusable* felement = focusedElementCandidates.back();	// focused element
		focusedElementCandidates.pop_back();

		// for all unfocused elements:
		for (UIFocusable* fe : focusedElementCandidates) {
			if (fe->bHoveredOver) {
				in.returnMouseFocus();
				fe->onLeave();
			}
		}

		// for the focused element:
		if (felement->bHoveredOver) {
			felement->onHover();
		}
		else {
			in.takeMouseFocus(felement->hoverFocus);
			felement->onEnter();
		}
	}
}

void UIManager::clickableUpdate()
{
	std_extra::tuple_for_each(uiElementTuple,
		[&](auto& container) {
			if constexpr (std::is_base_of<UIClickable, std::remove_reference<decltype(container.get(0))>::type>::value) {
				for (auto& uient : container) {
					auto* element = &container.get(uient);
					UIClickable* celement = static_cast<UIClickable*>(element);

					bool leftClick = in.buttonPressed(Button::MB_LEFT, celement->hoverFocus);

					if (celement->isEnabled() && celement->bHoveredOver) {
						if (leftClick) {
							if (celement->bPressed) {
								celement->onHold();
							}
							else {
								celement->onClick();
							}
						}
						else if (celement->bPressed) {
							celement->onRelease();
						}
					}
					else if (celement->bPressed) {
						celement->bPressed = false;
					}
				}
			}
		}
	);
}

void UIManager::postUpdate()
{
	for (auto& uient : getElementContainer<UIFrame>()) {
		auto& frame = getElementContainer<UIFrame>().get(uient);
		frame.postUpdate();
	}
}
