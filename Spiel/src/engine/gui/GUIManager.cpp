#include "GUIManager.hpp"

#include "base/GUIDrawElements.hpp"
#include "base/GUIDestroyElements.hpp"

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

	void Manager::draw(DrawContext const& context, Renderer& renderer)
	{
		std::vector<Sprite> sprites;
		for (auto& [element, version, exists] : rootElements) {
			if (exists) {
				gui::onDraw(*this, element, context, sprites);
			}
		}
		renderer.submit(sprites);
	}

	size_t Manager::size() const
	{
		return rootElements.size() - freeRootElementIndices.size() + elements.size() - freeElementIndices.size();
	}
}
