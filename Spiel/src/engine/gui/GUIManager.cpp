#include "GUIManager.hpp"

#include "components/GUICommonElementsDraw.hpp"
#include "components/GUITextElementsDraw.hpp"
#include "components/GUIMouseEvent.hpp"
#include "components/GUIDragEvent.hpp"

namespace gui {
	Manager::Manager(TextureManager* tex, FontManager* fonts)
	{
		this->tex = tex;
		this->fonts = fonts;
		defaultStyle.font = fonts->getHandle(DEFAULT_FONT);
		defaultStyle.fontTexture = tex->getHandle(DEFAULT_FONT_TEXTURE);
	}

	Manager::RootHandle gui::Manager::build(Root&& root)
	{
		RootHandle handle;
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

	bool Manager::hasChild(u32 element)
	{
		bool bHasChild{ false };
		std::visit(
			[&bHasChild](auto&& element) {
				if (u32* child = getChild(element)) {
					bHasChild = *child != INVALID_ELEMENT_ID;
				}
				else if (std::vector<u32>* children = getChildren(element)) {
					bHasChild = children->size() > 0ULL;
				}
				else if (std::array<u32,2>* children = getChildPair(element)) {
					bHasChild = children->size() > 0ULL;
				}
				else {
					bHasChild = false;
				}
			},
			elements[element]
		);
		return bHasChild;
	}

	std::vector<u32>* Manager::getChildrenIf(u32 element)
	{
		std::vector<u32>* ret{ nullptr };
		assert(element < elements.size() && elements[element].index() != 0);
		std::visit(
			[&](auto&& element) { ret =  getChildren(element); },
			elements[element]
		);
		return ret;
	}

	u32* Manager::getChildIf(u32 element)
	{
		u32* ret{ nullptr };
		assert(element < elements.size() && elements[element].index() != 0);
		std::visit(
			[&](auto&& element) { ret =  getChild(element); },
			elements[element]
		);
		return ret;
	}

	void Manager::changeChildPosition(u32 child, u32 newPosition)
	{
		std::visit(
			[&](auto&& el) 
			{ 
				if (std::vector<u32>* children = getChildren(el)) {
					assert(std::find(children->begin(), children->end(), child) != children->end());		// assert that the parent knows the child
					std::remove_if(children->begin(), children->end(), [&](u32 c) {return c == child; });
					children->insert(children->begin() + newPosition, child);
				}
				else if (std::array<u32, 2>*children = getChildPair(el)) {
					assert(false);	// CANNOT CHANGE CHILD POS FOR PAIR CONTAINER
				}
				else {
					assert(false);	// the parent element can not change the position of its child
				}
			}, 
			elements[parents[child]]
		);
	}

	void Manager::orphanChild(u32 toOrphanChild)
	{
		u32 parent = parents[toOrphanChild];
		parents[toOrphanChild] = INVALID_ELEMENT_ID;
		if (parent != INVALID_ELEMENT_ID) {
			std::visit(
				[&](auto&& element) { 
					if (u32* child = getChild(element)) {
						assert(*child == toOrphanChild);		// assert that single parent knows this child
						*child = INVALID_ELEMENT_ID;
					}
					else if (std::array<u32, 2>*children = getChildPair(element)) {
						assert(false);	// CANNOT ORPHAN A CHILD OF A PAIR CONTAINER
					}
					else if (std::vector<u32>* children = getChildren(element)) {
						assert(std::find(children->begin(), children->end(), toOrphanChild) != children->end());		// assert that the parent knows the child
						children->erase(std::remove(children->begin(), children->end(), toOrphanChild), children->end());
					}
					else {
						assert(false);	// parent id is not of a parent element!
					}
				}, 
				elements[parent]
			);
		}
	}

	void Manager::adoptChild(u32 toAdoptChild, u32 parent, u32 position)
	{
		assert(parents[toAdoptChild] == INVALID_ELEMENT_ID);		// assert, that child does not have a parent yet
		parents[toAdoptChild] = parent;
		if (parent != INVALID_ELEMENT_ID) {
			std::visit(
				[&](auto&& element) {
					if (u32* child = getChild(element)) {
						assert(*child == INVALID_ELEMENT_ID);		// assert that single parent does not have a child yet
						*child = toAdoptChild;
					}
					else if (std::array<u32, 2>* children = getChildPair(element)) {
						assert(false);	// CANNOT ADOPT A CHILD TO A PAIR CONTAINER
					}
					else if (std::vector<u32>* children = getChildren(element)) {
						assert(std::find(children->begin(), children->end(), toAdoptChild) == children->end());		// assert that the parent doesnt know the child yet
						if (position == 0xFFFFFFFF) position = children->size();
						children->insert(children->begin() + position, toAdoptChild);
					}
					else {
						assert(false);	// parent id is not of a parent element!
					}
				},
				elements[parent]
			);
		}
	}

	void Manager::changeParent(u32 child, u32 newParent, u32 newParentPosition)
	{
		orphanChild(child);
		adoptChild(child, newParent, newParentPosition);
	}

	void Manager::updateChildHierarchy(u32 parent)
	{
		std::visit(
			[&](auto&& e) {
				if (u32* child = getChild(e)) {
					if (*child != INVALID_ELEMENT_ID) {
						parents[*child] = parent;
					}
				}
				else if (std::array<u32,2>* children = getChildPair(e)) {
					for (u32 child : *children) {
						if (child != INVALID_ELEMENT_ID) {
							parents[child] = parent;
						}
					}
				}
				else if (std::vector<u32>* children = getChildren(e)) {
					for (u32 child : *children) {
						if (child != INVALID_ELEMENT_ID) {
							parents[child] = parent;
						}
					}
				}
			},
			elements[parent]
		);
	}

	void Manager::destroy(const RootHandle& handle)
	{
		assert(isHandleValid(handle));

		auto& [element, version, exists] = rootElements[handle.index];

		destroy(element.child);

		exists = false;
		freeRootElementIndices.push_back(handle.index);
	}

	void Manager::destroy(const u32 elementid)
	{
		if (elementid != INVALID_ELEMENT_ID) {
			destroylist.clear();
			destroylist.push_back(elementid);
			while (!destroylist.empty()) {
				const u32 id = destroylist.back();
				destroylist.pop_back();

				parents[id] = INVALID_ELEMENT_ID;

				std::visit(
					[&](auto&& element) {
						if (u32* child = getChild(element)) {
							if (*child != INVALID_ELEMENT_ID) {
								destroylist.push_back(*child);
							}
						}
						else if (std::array<u32, 2>*children = getChildPair(element)) {
							for (u32 child : *children) {
								if (child != INVALID_ELEMENT_ID) {
									destroylist.push_back(child);
								}
							}
						}
						else if (std::vector<u32>* children = getChildren(element)) {
							for (u32 child : *children) {
								if (child != INVALID_ELEMENT_ID) {
									destroylist.push_back(child);
								}
							}
						}
					},
					elements[id]
						);

				freeElementIndices.push_back(id);
				elements[id] = std::monostate();
			}
		}
	}

	bool Manager::isHandleValid(const RootHandle& handle) const
	{   
		return static_cast<u32>(rootElements.size()) > handle.index &&
			rootElements[handle.index].containsElement &&
			rootElements[handle.index].version == handle.version;
	}

	void Manager::draw(const RenderCoordSys& coordSys, Window& window, float deltaTime)
	{
		this->window = &window;
		this->coordSys = coordSys;
		this->deltaTime = deltaTime;
		this->mouseEvenetQueue.clear();

		blurTextureHandle = tex->getHandle("blur");

		spritesOfLastDraw.clear();

		for (auto& size : minsizes) { size = {NAN,NAN}; }	// clear sizes cache


		updateDraggedElement();
		updateFocusedTextInput();

		for (u32 id = 0; id < rootElements.size(); ++id) {
			if (rootElements[id].containsElement) { drawRoot(*this, id, spritesOfLastDraw); }
		} 

		for (auto const& mouseEvent : mouseEvenetQueue) {
			onMouseEvent(*this, elements[mouseEvent.element], mouseEvent.element, mouseEvent.root);
		}
		if (mouseEvenetQueue.size()) {
			window.consumeMouseButtonEvent(MouseButton::MB_LEFT);
			window.consumeMouseButtonEvent(MouseButton::MB_MIDDLE);
			window.consumeMouseButtonEvent(MouseButton::MB_RIGHT);
			window.consumeMouseButtonEvent(MouseButton::MB_4);
			window.consumeMouseButtonEvent(MouseButton::MB_5);
			window.consumeMouseScrollX();
			window.consumeMouseScrollY();
		}

		this->deltaTime = NAN;
		this->window = nullptr;
	}

	size_t Manager::size() const
	{
		return rootElements.size() - freeRootElementIndices.size() + elements.size() - freeElementIndices.size();
	}

	void Manager::printMemoryUtalisation()
	{
		std::vector<std::pair<const char*, size_t>> typeSizes;
		typeSizes.push_back({ typeid(Box).name(), sizeof(Box) });
		typeSizes.push_back({ typeid(Group).name(), sizeof(Group) });
		typeSizes.push_back({ typeid(StaticText).name(), sizeof(StaticText) });
		typeSizes.push_back({ typeid(Text).name(), sizeof(Text) });
		typeSizes.push_back({ typeid(_TextInput).name(), sizeof(_TextInput) });
		typeSizes.push_back({ typeid(_Button).name(), sizeof(_Button) });
		typeSizes.push_back({ typeid(_Checkbox).name(), sizeof(_Checkbox) });
		typeSizes.push_back({ typeid(SliderF64).name(), sizeof(SliderF64) });

		std::sort(typeSizes.begin(), typeSizes.end(), [](std::pair<const char*, size_t>& a, std::pair<const char*, size_t>& b) { return a.second > b.second; });

		for (auto [name, size] : typeSizes) {
			std::cout << "type " << name << " has size " << size << std::endl;
		}

	}

	void Manager::requestMouseEvent(u32 id, u32 rootid, float depth)
	{
		auto mevent = MouseEvent{ id,rootid,depth };
		// inserts the mouse event in order into the mouse event queue
		// highest prio event is the first, least prio is last
		mouseEvenetQueue.insert(
			std::upper_bound(
				mouseEvenetQueue.begin(), 
				mouseEvenetQueue.end(),
				mevent,
				[](MouseEvent const& a, MouseEvent const& b) 
				{ 
					return a.depth >= b.depth;
				}
			),
			mevent
		);
	}

	void Manager::updateDraggedElement()
	{
		droppedElement = nullptr;
		droppedElementId = INVALID_ELEMENT_ID;
		auto& [elementid, roothandle] = draggedElement;
		if (window->buttonPressed(MouseButton::MB_LEFT) && elementid != INVALID_ELEMENT_ID && isHandleValid(roothandle)) {

			onDragEvent(*this, elements[elementid], elementid, roothandle.index);

			window->consumeMouseButtonEvent(MouseButton::MB_LEFT);
			window->consumeMouseButtonEvent(MouseButton::MB_MIDDLE);
			window->consumeMouseButtonEvent(MouseButton::MB_RIGHT);
			window->consumeMouseButtonEvent(MouseButton::MB_4);
			window->consumeMouseButtonEvent(MouseButton::MB_5);
		}
		else {
			if (elementid != INVALID_ELEMENT_ID && isHandleValid(roothandle)) {
				if (DragDroppable* d = std::get_if<DragDroppable>(&elements[elementid])) {
					droppedElement = d;
					droppedElementId = elementid;
				}
			}
			draggedElement = { INVALID_ELEMENT_ID, {} };
		}
	}

	void Manager::updateFocusedTextInput()
	{
		auto& [elementid, roothandle] = focusedTextInput;
		if (!(elementid != INVALID_ELEMENT_ID && isHandleValid(roothandle)) || window->buttonJustPressed(MouseButton::MB_LEFT)) {
			focusedTextInput = { INVALID_ELEMENT_ID, {} };
		}
	}
}
