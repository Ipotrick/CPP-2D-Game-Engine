#pragma once

#include <vector>
#include <variant>
#include <deque>

#include "../rendering/Renderer.hpp"

#include "base/GUIRootElement.hpp"
#include "base/GUIElement.hpp"
#include "components/GUIBox.hpp"
#include "components/GUIColumn.hpp"

namespace gui {

	using ElementVariant = std::variant<std::monostate, Box, FillBox, Column>;

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
				index = static_cast<u32>(elements.size() - 1);
			}
			return index;
		}

		void destroy(const RootElementHandle& handle);

		bool isHandleValid(const RootElementHandle& handle) const;

		void draw(DrawContext const& context, Renderer& renderer);

		size_t size() const;
	private: 
		template<typename T>
		friend void onDraw(Manager&, T&, DrawContext const&, std::vector<Sprite>& ); 
		template<typename T>
		friend void onDestroy(Manager&, T&, std::vector<u32>&);

		struct RootElementVersionPair {
			Root element; 
			u32 version = 0;
			bool containsElement = false;
		};

		std::vector<RootElementVersionPair> rootElements;
		std::vector<u32> freeRootElementIndices;
		std::vector<u32> destroylist;						// used as buffer for element indices when destroying elements


		std::vector<ElementVariant> elements;
		std::deque<u32> freeElementIndices;
	};
}