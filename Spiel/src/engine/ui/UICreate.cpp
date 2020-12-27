#include "UICreate.hpp"

UIElement* ui::pair(UIPair::Parameters param, std::initializer_list<UIElement*> children) { return makeui<UIPair>(UIPair(param), children); }

UIEntityHandle ui::frame(UIFrame::Parameters param, std::initializer_list<UIElement*> children) {
	assert(children.size() == 1);
	UIFrame frame(param);

	frame.addChild(*children.begin());

	return EngineCore::ui.createFrame(frame);
}

UIEntityHandle ui::frame(const char* alias, UIFrame::Parameters param, std::initializer_list<UIElement*> children)
{
	assert(children.size() == 1);
	UIFrame frame(param);

	frame.addChild(*children.begin());

	return EngineCore::ui.createFrame(frame, alias);
}

UIElement* ui::bar(UIBar::Parameters param) { return makeui<UIBar>(UIBar(param)); }

UIElement* ui::text(UIText::Parameters param) { return makeui<UIText>(UIText(param)); }
