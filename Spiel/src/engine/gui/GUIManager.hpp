#pragma once

#include <vector>
#include <variant>
#include <deque>

#include "../rendering/Renderer.hpp"
 
#include "base/GUIRootElement.hpp"
#include "base/GUIElement.hpp"
#include "components/GUICommonElements.hpp"
#include "components/GUITextElements.hpp"
#include "components/GUICommonElementsStyling.hpp"
#include "components/GUITextElementsStyling.hpp"
#include "components/GUIChildAccess.hpp"

namespace gui {

	using ElementVariant = std::variant<
		std::monostate,
		Box, 
		Group,
		StaticText, 
		Text,
		_TextInput, 
		_TextInputF64, 
		_Button,
		_Checkbox, 
		SliderF64,
		DropBox,
		DragDroppable,
		Footer 
	>;

	class Manager {
	public:
		struct RootHandle {
			bool valid() const { return index != 0xFFFFFFFF && version != 0xFFFFFFFF; }

			u32 index = 0xFFFFFFFF;
			u32 version = 0xFFFFFFFF;
		};

		Manager(int renderLayer = 0) : renderLayer{ renderLayer } {}

		Manager(Manager&& rhs) = delete;

		Manager(const Manager& rhs) = delete;

		Manager& operator=(const Manager& rhs) = delete;

		Manager& operator=(const Manager&& rhs) = delete;

		RootHandle build(Root&& root);

		/**
		 * Creates a new element.
		 * Automaticly sets parent child relationships.
		 * The default Style is used to set any unset values here.
		 *
		 * \param element that should be constructed.
		 * \return the id of the element.
		 */
		template<CElement T>
		u32 build(T&& element)
		{
			return build<T>(DEFAULT_STYLE, std::move(element));
		}

		/**
		 * Creates a new element.
		 * Automaticly sets parent child relationships.
		 * 
		 * \param style the element should be styled after. 
		 * The Style is overwritten by any manually set values in construction.
		 * \param element that should be constructed.
		 * \return the id of the element.
		 */
		template<CElement T>
		u32 build(const Style& style, T&& element)
		{
			applyStyle(element, style);
			u32 index;

			if (!freeElementIndices.empty()) {
				index = freeElementIndices.back();
				freeElementIndices.pop_back();
				elements[index] = std::move(element);
			}
			else {
				elements.emplace_back(std::move(element));
				parents.emplace_back(INVALID_ELEMENT_ID);
				minsizes.resize(elements.size(), Vec2{ 0.0f, 0.0f });
				index = static_cast<u32>(elements.size() - 1);
			}

			std::visit( 
				[&](auto&& e)  {
					if (u32* child = getChild(e)) {
						if (*child != INVALID_ELEMENT_ID) {
							parents[*child] = index;
						}
					}
					if (std::vector<u32>* children = getChildren(e)) {
						for (u32 child : *children) {
							if (child != INVALID_ELEMENT_ID) {
								parents[child] = index;
							}
						}
					}
				}, 
				elements[index]
			);

			return index;
		}

		/**
		 * \param element to check the childcount for.
		 * \return if the given element contains and parent child relationships.
		 */
		bool hasChild(u32 element);

		/**
		 * repositions the child in the list of children of the parent.
		 * 
		 * \param child to change position of
		 * \param newPosition of the child.
		 */
		void changeChildPosition(u32 child, u32 newPosition);

		/**
		 * Orphans the child from its parent.
		 * New parent adopts the child.
		 * All parent-child relations are properly changed.
		 * 
		 * \param child that should change parents.
		 * \param newParent the new parent of the child.
		 * \param newParentPosition position in list of children of the new parent (ignored when parent is single parent), the palceholder value 0xFFFFFFFF means append to the end of the list.
		 */
		void changeParent(u32 child, u32 newParent, u32 newParentPosition = 0xFFFFFFFF);

		/**
		 * destroys a root and all its children.
		 * 
		 * \param handle of the root that should be destroyed.
		 */
		void destroy(const RootHandle& handle);

		/**
		 * asserts if root element that the handle refers too still exists.
		 *
		 * \param handle of the root.
		 */
		bool isHandleValid(const RootHandle& handle) const;

		/**
		 * Updates all elements.
		 * Makes all relevant callbacks.
		 * draws the ui to the set Layer in the given Renderer.
		 * 
		 * \param renderer to render to.
		 * \param window that the renderer belongs to.
		 * \param deltaTime the time difference to the last draw call.
		 */
		void draw(Renderer& renderer, Window& window, float deltaTime);

		/**
		 * \return the amount of root and normal elements currently stored.
		 */
		size_t size() const;

		void printMemoryUtalisation();

		float globalScaling{ 1.0f };
	private:
		friend Vec2 updateAndGetMinsize(Manager&, u32);
		friend void drawRoot(Manager& manager, u32 id, std::vector<Sprite>& out);
		template<typename T>
		friend void onDraw(Manager&, T&, u32, DrawContext const&, std::vector<Sprite>& ); 
		template<typename T>
		friend void onMouseEvent(Manager& manager, T& element, u32 id, u32 rootid);
		template<typename T>
		friend void onDragEvent(Manager& manager, T& self, u32 id, u32 rootid);

		/**
		 * Removes the child and parent relationship in the manager.
		 * Should only be used to transfer relationships, NOT to store elements without a root.
		 *
		 * \param child to remove the relationship of
		 */
		void orphanChild(u32 child);

		/**
		 * Adds a parent -child relationship inside the manager.
		 * Should only be used to transfer relationships, NOT to store elements without a root.
		 *
		 * \param child to adopt
		 * \param parent that adopts the child
		 * \param position the child should be placed in the children list (is ignored for single parents)
		 */
		void adoptChild(u32 child, u32 parent, u32 position = 0xFFFFFFFF);

		void requestMouseEvent(u32 elementid, u32 rootid, float depth);

		/// out factored functions called in draw:			///
		void updateDraggedElement();
		void updateFocusedTextInput();

		/// Temporary variables with lifetime of the draw:  ///
		Window* window{ nullptr };
		Renderer* renderer{ nullptr };
		float deltaTime{ 0.0f };
		// Mouse Event Listener:
		u32 mouseEventElement{ INVALID_ELEMENT_ID };
		u32 mouseEvenetElementRoot{ INVALID_ELEMENT_ID };
		float mouseEventElementDepth{ 0 };
		// Drag Drop:
		u32 droppedElementId{ INVALID_ELEMENT_ID };
		DragDroppable* droppedElement{ nullptr };
		/// ----------------------------------------------- ///

		int renderLayer = 0;
		struct RootElementMetaInfo {
			Root element;
			u32 version = 0;
			bool containsElement = false;
		};
		std::vector<RootElementMetaInfo> rootElements;
		std::vector<u32> freeRootElementIndices;
		std::vector<u32> destroylist;						// used as buffer for element ids when destroying elements and their children

		std::vector<ElementVariant>		elements;
		std::vector<u32>				parents;			// stores parent of each element
		std::vector<Vec2>				minsizes;			// used to cache min sizes of elements, to minimise redundant calculations
		std::deque<u32>					freeElementIndices;

		struct DraggedElementInfo {
			u32 elementId;
			u32 parentId;
			RootHandle root;
		};


		std::pair<u32 /*element id*/, RootHandle> draggedElement{ INVALID_ELEMENT_ID, {} };
		std::pair<u32 /* element id*/, RootHandle> focusedTextInput{ INVALID_ELEMENT_ID, {} };
	};
}