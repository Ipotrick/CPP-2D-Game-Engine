#include "GUIManager.hpp"

#include "components/GUIDrawElements.hpp"
#include "components/GUIDrawTextElements.hpp"
#include "components/GUIDestroyElements.hpp"
#include "components/GUIMouseEvent.hpp"

namespace gui {

	RootElementHandle gui::Manager::build(Root&& root)
	{
		RootElementHandle handle;
		if (!freeRootElementIndices.empty()) /* reuse index */ {
			handle.index = freeRootElementIndices.back();
			freeRootElementIndices.pop_back();

			rootElements[handle.index].element = root;
			rootElements[handle.index].containsElement = true;
			handle.version = ++rootElements[handle.index].version;
		}
		else /* expand root element vector */ {
			rootElements.emplace_back( root, 0, true);
			handle.index = static_cast<u32>(rootElements.size() - 1);
			handle.version = 0;
		}
		return handle;
	}

	void Manager::destroy(const RootElementHandle& handle)
	{
		assert(isHandleValid(handle));

		auto& [element, version, exists] = rootElements[handle.index];

		if (element.child != INVALID_ELEMENT_ID) {
			destroylist.clear();
			destroylist.push_back(element.child);

			while (!destroylist.empty()) {
				const u32 id = destroylist.back();
				destroylist.pop_back();

				gui::destroy(*this, elements[id], destroylist);
				 
				freeElementIndices.push_back(id);
				elements[id] = std::monostate();
			}
		}

		exists = false;
		freeRootElementIndices.push_back(handle.index);
	}

	bool Manager::isHandleValid(const RootElementHandle& handle) const
	{   
		return static_cast<u32>(rootElements.size()) > handle.index &&
			rootElements[handle.index].containsElement &&
			rootElements[handle.index].version == handle.version;
	}

	void Manager::draw(Renderer& renderer, Window& window, float deltaTime)
	{
		this->window = &window;
		this->renderer = &renderer;
		this->deltaTime = deltaTime;
		this->mouseEventElement = INVALID_ELEMENT_ID;
		this->mouseEventElementDepth = 0.0f;

		for (auto& size : minsizes) { size.reset(); }	// clear sizes cache

		updateDraggedElement();
		updateFocusedTextInput();

		std::vector<Sprite> sprites;
		for (u32 id = 0; id < rootElements.size(); ++id) {
			if (rootElements[id].containsElement) { drawRoot(*this, id, sprites); }
		}
		renderer.submit(sprites, renderLayer);

		if (mouseEventElement != INVALID_ELEMENT_ID) {
			// execute mouse event request success:
			onMouseEvent(*this, elements[mouseEventElement], mouseEventElement, mouseEvenetElementRoot);
			window.consumeMouseButtonEvent(MouseButton::MB_LEFT);
			window.consumeMouseButtonEvent(MouseButton::MB_MIDDLE);
			window.consumeMouseButtonEvent(MouseButton::MB_RIGHT);
			window.consumeMouseButtonEvent(MouseButton::MB_4);
			window.consumeMouseButtonEvent(MouseButton::MB_5);
		}

		this->mouseEventElementDepth = 0.0f;
		this->mouseEventElement = INVALID_ELEMENT_ID;
		this->deltaTime = 0.0f;
		this->renderer = nullptr;
		this->window = nullptr;
	}

	size_t Manager::size() const
	{
		return rootElements.size() - freeRootElementIndices.size() + elements.size() - freeElementIndices.size();
	}

	void Manager::requestMouseEvent(u32 id, u32 rootid, float depth)
	{
		if (depth >= mouseEventElementDepth) {
			mouseEventElement = id;
			mouseEvenetElementRoot = rootid;
		}
	}

	void Manager::updateDraggedElement()
	{
		auto& [elementid, roothandle] = draggedElement;
		if (window->buttonPressed(MouseButton::MB_LEFT) && elementid != INVALID_ELEMENT_ID && isHandleValid(roothandle)) {

			Root& root = rootElements[roothandle.index].element;
			Vec2 cursorPos = renderer->convertCoordSys<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(window->getCursorPos());
			Vec2 cursorPrevPos = renderer->convertCoordSys<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(window->getPrevCursorPos());

			DrawContext windowContext;
			windowContext.scale = globalScaling;
			windowContext.topleft = Vec2{ 0.0f, window->getSizeVec().y };
			windowContext.bottomright = Vec2{ window->getSizeVec().x, 0.0f };
			root.placeing.move(cursorPos - cursorPrevPos, windowContext, root.sizeing);
			window->consumeMouseButtonEvent(MouseButton::MB_LEFT);
			window->consumeMouseButtonEvent(MouseButton::MB_MIDDLE);
			window->consumeMouseButtonEvent(MouseButton::MB_RIGHT);
			window->consumeMouseButtonEvent(MouseButton::MB_4);
			window->consumeMouseButtonEvent(MouseButton::MB_5);
		}
		else {
			draggedElement = { INVALID_ELEMENT_ID, {} };
		}
	}

	void Manager::updateFocusedTextInput()
	{
		auto& [elementid, roothandle] = focusedTextInput;
		if (!(elementid != INVALID_ELEMENT_ID && isHandleValid(roothandle)) || window->buttonJustPressed(MouseButton::MB_LEFT)) {
			// TODO call the store function of the text input
			focusedTextInput = { INVALID_ELEMENT_ID, {} };
		}
	}
}
