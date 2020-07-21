#pragma once

#include <chrono>

#include "robin_hood.h"

#include "Timing.hpp"

struct PerfLogger {
public:
	inline void clear() {
		timeMap.clear();
		timeFloatMap.clear();
	}
	inline void commitTimes() {
		for (auto& el : timeMap) {
			timeFloatMap[el.first] = micsecToFloat(el.second);
			el.second = std::chrono::microseconds(0);
		}
	}
	inline void submitTime(std::string_view str, std::chrono::microseconds time = std::chrono::microseconds(0)) {
		timeMap[str] = time;
	}
	inline float getTime(std::string_view str) {
		return timeFloatMap[str];
	}
	inline std::chrono::microseconds& getInputRef(std::string_view str) {
		return timeMap[str];
	}
private:
	std::unordered_map<std::string_view, std::chrono::microseconds> timeMap;
	std::unordered_map<std::string_view, float> timeFloatMap;
};