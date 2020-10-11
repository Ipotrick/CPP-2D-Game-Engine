#pragma once
#pragma once

#include "UIMultiParent.hpp"

template<std::size_t CAPACITY>
class UIField : public UIMultiParent<CAPACITY> {
public:
	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override
	{
		context = this->anchor.shrinkContextToMe(this->size, context);

		context.increaseDrawPrio();
		for (auto& el : this->children) {
			el->draw(buffer, context);
		}
	}
};

using UIField8 = UIField<8>;
using UIField16 = UIField<16>;
using UIField32 = UIField<32>;
using UIField64 = UIField<32>;