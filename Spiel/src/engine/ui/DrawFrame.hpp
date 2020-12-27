#pragma once

#include "../rendering/RenderTypes.hpp"
#include "UIContext.hpp"

void drawFrame(std::vector<Sprite>& buffer, UIContext context, Vec2 pos, Vec2 size, Vec2 border, Vec4 borderColor, Vec4 fillColor);