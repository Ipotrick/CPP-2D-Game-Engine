#pragma once

#include "RenderTypes.hpp"
#include "UIContext.hpp"

void drawFrame(std::vector<Drawable>& buffer, UIContext context, Vec2 pos, Vec2 size, Vec2 border, Vec4 borderColor, Vec4 fillColor);