#pragma once

#include <chrono>
#include <iostream>
#include <iomanip>
#include <concepts>

#include <sstream>

#include "../types/Timing.hpp"

namespace Monke {
	inline static const std::chrono::time_point STARTING_TIME_POINT{ std::chrono::high_resolution_clock::now() };

	template<typename T>
	bool replaceFormatBraces (std::string& str, T&& el) {
		auto first = str.begin() + str.find_first_of("{}");
		if (first == str.end()) {
			return false;
		}
		str.replace(first, first + 2, std::to_string(el));
		return true;
	};

	template<typename ... T>
	static void log(std::string_view format, T&&... args)
	{
		int errorCode{ 0 };

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - STARTING_TIME_POINT).count();
		static constexpr auto h = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::hours(1)).count();
		static constexpr auto m = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::minutes(1)).count();
		static constexpr auto s = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1)).count();
		static constexpr auto ms = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(1)).count();
		std::stringstream ss;
		ss << "[";
		ss << std::setw(2) << std::setfill('0') << (duration / h) << "h:";
		duration %= h;
		ss << std::setw(2) << std::setfill('0') << (duration / m) << "m:";
		duration %= m;
		ss << std::setw(2) << std::setfill('0') << (duration / s) << "s:";
		duration %= s;
		ss << std::setw(3) << std::setfill('0') << (duration / ms) << ":";
		duration %= ms;
		ss << std::setw(3) << std::setfill('0') << duration << "]";
		ss << format;
		ss << '\n';
		auto str = ss.str();

		std::vector<std::string> paramStr;
		(paramStr.push_back( std::to_string(args) ), ...);


		while (true) {
			size_t firstIndex = str.find_first_of('{');
			if (firstIndex == -1) {
				break;
			}
			auto lastIndex = str.find_first_of('}', firstIndex);
			if (lastIndex == -1) {
				errorCode = 1;
				break;
			}

			size_t indexNumWidth = lastIndex - firstIndex - 1;
			if (indexNumWidth == -1) {
				errorCode = 2;
				break;
			}
			size_t index = std::stoi(str.substr(firstIndex + 1, lastIndex).c_str());
			if (index >= paramStr.size()) {
				errorCode = 3;
				break;
			}

			str.erase(firstIndex, lastIndex - firstIndex +1);
			str.insert(str.begin() + firstIndex, paramStr[index].begin(), paramStr[index].end());
		} 

		if (errorCode) {
			switch (errorCode) {
			case 1:
				printf("Log syntax error: missing second bracket\n");
				break;
			case 2:
				printf("Log syntax error: missing index in brackets\n");
				break;
			case 3:
				printf("Log syntax error: index out of bounds\n");
				break;
			default:
				printf("Log syntax error\n");
			}
			assert(false);
		}
		else {
			printf(str.c_str());
		}
	}

}