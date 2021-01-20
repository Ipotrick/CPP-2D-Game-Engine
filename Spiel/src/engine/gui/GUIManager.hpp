#pragma once

#include <vector>
#include <variant>
#include <deque>

#include "../rendering/Renderer.hpp"
#include "../io/InputManager.hpp"

#include "base/GUIRootElement.hpp"
#include "base/GUIElement.hpp"
#include "components/GUICommonElements.hpp"

namespace gui {

	using ElementVariant = std::variant<std::monostate, Box, FillBox, Column, GUIButton>;

	struct RootElementHandle {
		bool valid() const { return index != -1 && version != -1; }

		u32 index = -1;
		u32 version = -1;
	};

	class Manager {
	public:
		RootElementHandle build(Root&& root);

		template<CElement T>
		u32 build(T&& element)
		{
			std::cout << "build a " << typeid(T).name() << std::endl;

			u32 index = -1;
			if (!freeElementIndices.empty()) {
				index = freeElementIndices.back();
				freeElementIndices.pop_back();
				elements[index] = element;
			}
			else {
				elements.emplace_back(element);
				minsizes.resize(elements.size(), Vec2{ 0.0f, 0.0f });
				index = static_cast<u32>(elements.size() - 1);
			}
			return index;
		}

		void destroy(const RootElementHandle& handle);

		bool isHandleValid(const RootElementHandle& handle) const;

		void draw(DrawContext const& context, Renderer& renderer, InputManager& in);

		size_t size() const;
	private: 
		template<typename T>
		friend void onDraw(Manager&, T&, DrawContext const&, std::vector<Sprite>& ); 
		template<typename T>
		friend void onDestroy(Manager&, T&, std::vector<u32>&);
		template<typename T, typename ... Args>
		friend void onUpdate(Manager&, T&, const Args& ...);
		friend Vec2 minsizeOf(Manager&, u32);

		/// Temporary variables for drawing:

		InputManager* in{ nullptr };
		Renderer const* renderer{ nullptr };

		/// --------------------------------

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
	};
}