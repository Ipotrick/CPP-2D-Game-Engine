#pragma once

#include <vector>
#include <variant>
#include <deque>

#include "../rendering/Renderer.hpp"

#include "base/GUIRootElement.hpp"
#include "base/GUIElement.hpp"
#include "components/GUICommonElements.hpp"
#include "components/GUITextElements.hpp"

namespace gui {

	using ElementVariant = std::variant<std::monostate, Box, FillBox, Column, StaticText, _TextInput, _Button, DragField>;

	struct RootElementHandle {
		bool valid() const { return index != 0xFFFFFFFF && version != 0xFFFFFFFF; }

		u32 index = 0xFFFFFFFF;
		u32 version = 0xFFFFFFFF;
	};

	class Manager {
	public:
		RootElementHandle build(Root&& root);

		Manager(int renderLayer = 0) : renderLayer{ renderLayer } {}

		template<CElement T>
		u32 build(T&& element)
		{
			u32 index = -1;
			if (!freeElementIndices.empty()) {
				index = freeElementIndices.back();
				freeElementIndices.pop_back();
				elements[index] = std::move(element);
			}
			else {
				elements.emplace_back(std::move(element));
				minsizes.resize(elements.size(), Vec2{ 0.0f, 0.0f });
				index = static_cast<u32>(elements.size() - 1);
			}
			return index;
		}

		void destroy(const RootElementHandle& handle);

		bool isHandleValid(const RootElementHandle& handle) const;

		void draw(Renderer& renderer, Window& window, float deltaTime);

		size_t size() const;

		float globalScaling{ 1.0f };
	private:
		friend Vec2 minsizeOf(Manager&, u32);
		friend void drawRoot(Manager& manager, u32 id, std::vector<Sprite>& out);
		template<typename T>
		friend void onDraw(Manager&, T&, u32, DrawContext const&, std::vector<Sprite>& ); 
		template<typename T>
		friend void onDestroy(Manager&, T&, std::vector<u32>&);
		template<typename T, typename ... Args>
		friend void onUpdate(Manager&, T&, const Args& ...);
		template<typename T>
		friend void onMouseEvent(Manager& manager, T& element, u32 id, u32 rootid);

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
		/// ----------------------------------------------- ///

		int renderLayer = 0;
		struct RootElementVersionPair {
			Root element;
			u32 version = 0;
			bool containsElement = false;
		};
		std::vector<RootElementVersionPair> rootElements;
		std::vector<u32> freeRootElementIndices;
		std::vector<u32> destroylist;						// used as buffer for element ids when destroying elements and their children

		std::vector<ElementVariant> elements;
		std::vector<std::optional<Vec2>> minsizes;			// used to cache min sizes of elements, to minimise redundant calculations
		std::deque<u32> freeElementIndices;


		std::pair<u32 /*element id*/, RootElementHandle> draggedElement{ INVALID_ELEMENT_ID, {} };
		std::pair<u32 /* element id*/, RootElementHandle> focusedTextInput{ INVALID_ELEMENT_ID, {} };
	};
}