#include "UIManager.hpp"

void UIManager::destroyFrame(UIEntity index)
{
	if (frames.get(index).hasChild()) {
		frames.get(index).getChild()->destroy();
	}

	if (entityToAlias.contains(index)) {		// remove alias
		aliasToEntity.erase(entityToAlias[index]);
		entityToAlias.erase(index);
	}

	frames.destroy(index);
}
void UIManager::destroyFrame(std::string_view name)
{
	int index = aliasToEntity[name];
	entityToAlias.erase(aliasToEntity[name]);	// remove alias
	aliasToEntity.erase(name);
	destroyFrame(index);
}

void UIManager::update()
{
	for (auto& uient : frames) {
		if (frames.get(uient).isDestroyed()) {
			frames.destroy(uient);
		}
	}
	tuple_for_each(uiElementTuple, [](auto& container) {
		for (auto& uient : container) {
			if (container.get(uient).isDestroyed()) {
				container.destroy(uient);
			}
		}
		});
}

void UIManager::draw(UIContext context)
{
	std::vector<Drawable> buffer;
	int i = 0;
	context.depth++;
	for (auto& ent : frames) {
		auto& frame = frames.get(ent);
		frame.draw(buffer, context);
	}
	for (auto&& d : buffer) {
		renderer.submit(d);
	}
}