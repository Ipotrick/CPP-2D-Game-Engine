#pragma once

#include <charconv>

#include "GUICommonElementsDraw.hpp"
#include "../../rendering/Window.hpp"

namespace gui {

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _TextInput& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		if (!manager.tex->isHandleValid(self.fontTexPair.second)) {
			self.fontTexPair.second = manager.tex->makeHandle(self.fontTexPair.first);
		}
		if (!manager.fonts->isHandleValid(self.fontPair.second)) {
			self.fontPair.second = manager.fonts->makeHandle(self.fontPair.first);
		}
		return self.size;
	}
	template<> inline void onDraw(Manager& manager, _TextInput& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const auto scaledCompleteSize = manager.minsizes[id] * context.scale;
		const Vec2 centerPlace = getPlace(scaledCompleteSize, context);

		const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace::Window, RenderSpace::Pixel);
		const bool cursorOverElement = isCoordInBounds(scaledCompleteSize, centerPlace, cursor);

		if (cursorOverElement)
			manager.requestMouseEvent(id, context.root, context.renderDepth);

		Vec4 color{ 0.8,0.8,0.8,1 };
		const bool focused = manager.focusedTextInput.first == id;
		if (focused) {
			// we are the focused text input
			color = { 1,1,1,1 };
			if (manager.window->keyJustPressed(Key::ESCAPE)) {
				manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
			}

			auto keyEvents = manager.window->getKeyEventsInOrder();

			bool shift = manager.window->keyPressed(Key::LEFT_SHIFT);

			for (auto e : keyEvents) {
				if (e.type == KeyEvent::Type::JustPressed || e.type == KeyEvent::Type::Repeat) {
					if (e.key == Key::BACKSPACE) {
						if (self.str.size() > 0) self.str.pop_back();
					}
					else if (e.key == Key::ENTER) {
						if (shift) {
							self.str += '\n';
						}
						else {
							manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
							self.onStore(self.str);
							if (self.bClearOnEnter) self.str.clear();
						}
					}
					else if (e.key == Key::DELETE) {
						self.str.clear();
					}
					else {
						auto opt = keyToChar(e.key, shift);
						if (opt.has_value()) {
							self.str += opt.value();
						}
					}
					manager.window->consumeKeyEvent(e.key);
				}
			}
		}
		out.push_back(Sprite{
			.color = color,
			.position = Vec3{centerPlace, context.renderDepth},
			.scale = scaledCompleteSize,
			.cornerRounding = 3.0f * context.scale,
			.drawMode = RenderSpace::Pixel 
			}
		);

		TextContext textContext{ context };
		textContext.xalign = self.xalign;
		textContext.yalign = self.yalign;
		textContext.fontSize = self.fontSize;
		textContext.color = self.color;
		textContext.tex = self.fontTexPair.second;
		textContext.font = &manager.fonts->get(self.fontPair.second);
		fit(textContext, scaledCompleteSize, centerPlace);
		auto charsDrawn = drawFontText(self.str.data(), textContext, out);

		if (self.str.size() > charsDrawn) /* we stopped char processing cause the string is too long, we need to remove the last few chars */{
			self.str = self.str.substr(0, charsDrawn);
		}
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, StaticText& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		if (!manager.tex->isHandleValid(self.fontTexPair.second)) {
			self.fontTexPair.second = manager.tex->makeHandle(self.fontTexPair.first);
		}
		if (!manager.fonts->isHandleValid(self.fontPair.second)) {
			self.fontPair.second = manager.fonts->makeHandle(self.fontPair.first);
		}
		return getMinSizeText(self.value.data(), self.fontSize, manager.fonts->get(self.fontPair.second));
	}
	template<> inline void onDraw(Manager& manager, StaticText& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		TextContext textContext{ context };
		textContext.fontSize = self.fontSize;
		textContext.color = self.color;
		textContext.tex = self.fontTexPair.second;
		textContext.font = &manager.fonts->get(self.fontPair.second);
		drawFontText(self.value.data(), textContext, out);
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, Text& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		if (!manager.tex->isHandleValid(self.fontTexPair.second)) {
			self.fontTexPair.second = manager.tex->makeHandle(self.fontTexPair.first);
		}
		if (!manager.fonts->isHandleValid(self.fontPair.second)) {
			self.fontPair.second = manager.fonts->makeHandle(self.fontPair.first);
		}
		return getMinSizeText(std::string(self.value).c_str(), self.fontSize, manager.fonts->get(self.fontPair.second));
	}
	template<> inline void onDraw(Manager& manager, Text& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		TextContext textContext{ context };
		textContext.fontSize = self.fontSize;
		textContext.color = self.color;
		textContext.tex = self.fontTexPair.second;
		textContext.font = &manager.fonts->get(self.fontPair.second);
		drawFontText(std::string(self.value).data(), textContext, out);
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _TextInputF64& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		if (!manager.tex->isHandleValid(self.fontTexPair.second)) {
			self.fontTexPair.second = manager.tex->makeHandle(self.fontTexPair.first);
		}
		if (!manager.fonts->isHandleValid(self.fontPair.second)) {
			self.fontPair.second = manager.fonts->makeHandle(self.fontPair.first);
		}
		return self.size;
	}
	template<> inline void onDraw(Manager& manager, _TextInputF64& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{

		const Vec2 scaledCompleteSize = manager.minsizes[id] * context.scale;
		const Vec2 centerPlace = getPlace(scaledCompleteSize, context);

		const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace::Window, RenderSpace::Pixel);
		const bool cursorOverElement = isCoordInBounds(scaledCompleteSize, centerPlace, cursor);

		if (cursorOverElement) {
			manager.requestMouseEvent(id, context.root, context.renderDepth);
		}

		Vec4 color = self.color;
		bool focused = manager.focusedTextInput.first == id;
		if (focused) {
			// we are the focused text input
			if (manager.window->keyJustPressed(Key::ESCAPE)) {
				manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
			}

			auto keyEvents = manager.window->getKeyEventsInOrder();

			const bool shift = manager.window->keyPressed(Key::LEFT_SHIFT);

			for (auto e : keyEvents) {
				if (e.type == KeyEvent::Type::JustPressed || e.type == KeyEvent::Type::Repeat) {
					if (e.key == Key::BACKSPACE) {
						if (self.str.size() > 0) self.str.pop_back();
					}
					else if (e.key == Key::ENTER) {
						manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
						focused = false;
						break;
					}
					else if (e.key == Key::DELETE) {
						self.str.clear();
					}
					else {
						auto opt = keyToChar(e.key, shift);
						if (opt.has_value()) {
							self.str += opt.value();
						}
					}
					manager.window->consumeKeyEvent(e.key);
				}
			}
		} 

		char* p = nullptr;
		f64 value = std::strtod(self.str.data(), &p);
		const bool bTextValid = p == self.str.data() + self.str.size() && self.str.size() > 0;
		if (!focused) /* this cannot be the else of the upper if statement as the variable 'focused' could change in the upper if statement */{
			color.r *= 0.8f;
			color.g *= 0.8f;
			color.b *= 0.8f;
			if (self.value) {
				if (*self.value != self.lastKnownValue) /* value changed from outside, we take that value into the text */ {
					self.str = std::to_string(*self.value);
				}
				else if (bTextValid && value != *self.value) /* we changed the text so we write it to the value */ {
					*self.value = value;
				}
				else /* set text to the value (makes formating with .0's prettier) */ {
					self.str = std::to_string(*self.value);
				}
				self.lastKnownValue = *self.value;
			}
		}

		out.push_back(Sprite{
			.color = bTextValid ? color : self.colorFontError,
			.position = Vec3{centerPlace, context.renderDepth},
			.scale = scaledCompleteSize,
			.cornerRounding = 3.0f * context.scale,
			.drawMode = RenderSpace::Pixel
			}
		);

		TextContext textContext{ context };
		textContext.xalign = XAlign::Center;
		textContext.yalign = YAlign::Center;
		textContext.fontSize = self.fontSize;
		textContext.color = self.colorFont;
		textContext.tex = self.fontTexPair.second;
		textContext.font = &manager.fonts->get(self.fontPair.second);
		fit(textContext, scaledCompleteSize, centerPlace);
		fit(textContext, self.textPadding);
		u32 charsProcessed = drawFontText(self.str.c_str(), textContext, out);

		if (self.str.size() > charsProcessed) /* we stopped char processing cause the string is too long, we need to remove the last few chars */ {
			self.str = self.str.substr(0, charsProcessed);
		}
	}
}
