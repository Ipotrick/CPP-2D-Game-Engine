#include "UIManager.hpp"
#include "Log.hpp"

void UIManager::destroyFrame(UIEntity index)
{
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
		Monke::log("WARNING: trying to delete non existant UIEntity!");
	}
}

bool UIManager::doesFrameExist(UIEntity ent)
{
	return getElementContainer<UIFrame>().contains(ent);
}

void UIManager::update()
{
	perEntityUpdate();
	focusUpdate();
	clickableUpdate();
}

void UIManager::draw(UIContext context)
{
	std::vector<Drawable> buffer;
	++context.recursionDepth;
	lastUpdateDrwawbleCount = 0;
	for (auto& ent : getElementContainer<UIFrame>()) {
		auto& frame = getElementContainer<UIFrame>().get(ent);
		buffer.clear();
		if (frame.isEnabled()) {
			frame.draw(buffer, context);
		}
		for (auto&& d : buffer) {
			renderer.submit(d, frame.layer);
		}
		lastUpdateDrwawbleCount += buffer.size();
	}
}

size_t UIManager::elementCount() const
{
	size_t sum{ 0 };
	util::tuple_for_each(uiElementTuple,
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
	util::tuple_for_each(uiElementTuple,
		[&](auto& container) {
			for (auto& uient : container) {
				auto& element = container.get(uient);
				element.update();
				activeElements += (size_t)element.isEnabled();
				if (element.isDestroyed()) container.destroy(uient);
			}
		}
	);
	lastUpdateActiveElements = activeElements;
}

void UIManager::focusUpdate()
{
	focusedElementCandidates.clear();
	// find potential elements, that could be focused:
	util::tuple_for_each(uiElementTuple,
		[&](auto& container) {
			if constexpr (std::is_base_of<UIFocusable, std::remove_reference<decltype(container.get(0))>::type>::value) {
				for (auto& uient : container) {
					auto* element = &container.get(uient);
					UIFocusable* felement = static_cast<UIFocusable*>(element);
					if (felement->isEnabled() && felement->isFocusable()) {
						const auto& area = felement->getFocusArea();

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
								felement->bHoveredOver = false;
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
				return a->getFocusArea().sortKey < b->getFocusArea().sortKey;
			}
		);

		UIFocusable* felement = focusedElementCandidates.back();	// focused element
		focusedElementCandidates.pop_back();

		// for all unfocused elements:
		for (UIFocusable* fe : focusedElementCandidates) {
			if (fe->bHoveredOver) {
				in.returnMouseFocus();
				fe->onLeave();
				fe->bHoveredOver = false;
			}
		}

		// for the focused element:
		if (felement->bHoveredOver) {
			felement->onHover();
			felement->bHoveredOver = true;
		}
		else {
			in.takeMouseFocus(felement->hoverFocus);
			felement->onEnter();
			felement->bHoveredOver = true;
		}
	}
}

void UIManager::clickableUpdate()
{
	util::tuple_for_each(uiElementTuple,
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
								celement->bPressed = true;
							}
							else {
								celement->onClick();
								celement->bPressed = true;
							}
						}
						else if (celement->bPressed) {
							celement->onRelease();
							celement->bPressed = false;
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
