#pragma once

#include <unordered_map>
#include <string_view>
#include <functional>
#include <any>
#include <set>

#include "types/ShortNames.hpp"

class EventSystem {
public:
	template<typename T>
	using Callback = std::function<bool(T&)>;

	template<typename T>
	void subscribe(std::string_view event, Callback<T> callback)
	{
		if (events.find(event) == events.end()) {
			events[event] = std::vector<Callback<T>>{};
		}

		auto& callbacks = std::any_cast<std::vector<Callback<T>>&>(events[event]);
		callbacks.push_back(std::move(callback));
	}

	template<typename T>
	void publish(std::string_view event, T data)
	{
		auto& callbacks = std::any_cast<std::vector<Callback<T>>&>(events[event]);
		killSet.clear();

		for (uint32_t i = 0; i < callbacks.size(); i++) {
			if (bool killCallback = callbacks[i](data)) {
				killSet.insert(i);
			}
		}

		for (auto riter = killSet.rbegin(); riter != killSet.rend(); riter++) {
			callbacks.erase(callbacks.begin() + *riter);
		}
	}
private:
	std::unordered_map<std::string_view, std::any> events;
	std::set<uint32_t> killSet;
};
