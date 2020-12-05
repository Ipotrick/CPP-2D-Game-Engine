#include "UICreate.hpp"

UIElement* ui::pair(UIPair::Parameters param, std::initializer_list<UIElement*> children) { return makeui<UIPair>(UIPair(param), children); }

UIElement* ui::frame(UIFrame::Parameters param, std::initializer_list<UIElement*> children) { return makeui<UIFrame>(UIFrame(param), children); }

UIElement* ui::bar(UIBar::Parameters param) { return makeui<UIBar>(UIBar(param)); }

UIElement* ui::text(UIText::Parameters param) { return makeui<UIText>(UIText(param)); }
