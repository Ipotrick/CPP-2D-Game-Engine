#pragma once

#include <chrono>
#include <iostream>
#include <ostream>
#include <sstream>
#include <thread>

inline float micsecToFloat(std::chrono::microseconds mics) {
	return mics.count() * 0.000001f;
}

inline std::chrono::microseconds floatToMicsec(float fl) {
	return std::chrono::microseconds( static_cast<uint32_t>(fl * 1'000'000.0f) );
}

template <typename Clock = std::chrono::high_resolution_clock ,typename Unit = std::chrono::microseconds>
class Timer {
public:
	Timer(Unit & p_output) :
		startTimePoint{Clock::now()}, output{p_output}, stopped{ false }
	{}
	~Timer() {
		if (!stopped) {
			stop();
		}
	}

	void stop() {
		auto stopTimePoint = Clock::now();
		Unit difference = std::chrono::duration_cast<Unit>(stopTimePoint - startTimePoint);
		output = difference;
		stopped = true;
	}
protected:
	bool stopped;
	std::chrono::time_point<Clock> startTimePoint;
	Unit & output;
};

template <typename Clock = std::chrono::high_resolution_clock, typename Unit = std::chrono::microseconds>
class LogTimer {
public:
	LogTimer(std::ostream& os_) :
		startTimePoint{ Clock::now() }, os{ os_ }, stopped{ false }
	{}
	~LogTimer() {
		if (!stopped) {
			stop();
		}
	}
	void stop() {
		auto stopTimePoint = Clock::now();
		Unit difference = std::chrono::duration_cast<Unit>(stopTimePoint - startTimePoint);
		std::stringstream ss;
		ss << "time taken: " << difference.count() << "us\n";
		os << ss.str();
		stopped = true;
	}
protected:
	bool stopped;
	std::chrono::time_point<Clock> startTimePoint;
	std::ostream& os;
};

template <typename Clock = std::chrono::high_resolution_clock , typename Unit = std::chrono::microseconds>
class AccTimer {
public:
	AccTimer(Unit & accum) :
		startTimePoint{Clock::now()},
		accumulator{accum}
	{}
	~AccTimer() {
		auto stopTimePoint = Clock::now();
		Unit difference = std::chrono::duration_cast<Unit>(stopTimePoint - startTimePoint);
		accumulator += difference;
	}
protected:
	std::chrono::time_point<Clock> startTimePoint;
	Unit & accumulator;
};

template <typename Clock = std::chrono::high_resolution_clock, typename Unit = std::chrono::microseconds>
class Waiter {
public:
	enum class Type {
		SLEEPY,
		BUSY
	};
	Waiter(Unit waiting_time_, Type type_ = Type::SLEEPY, Unit* actual_waiting_time_ = nullptr)
		: waitingTime{ waiting_time_ }, was_wait_called{ false }, actual_waiting_time{ actual_waiting_time_}, type{ type_ }
	{
		startTimePoint = Clock::now();
	}

	void wait() {
		if (type == Type::SLEEPY) {
			actualWaitingStartTimePoint = Clock::now();
			was_wait_called = true;
			if ((startTimePoint + waitingTime) > Clock::now()) {
				std::this_thread::sleep_until((startTimePoint + waitingTime));
			}
		}
		else {
			actualWaitingStartTimePoint = Clock::now();
			if (!was_wait_called) {
				while (Clock::now() < waitingTime + startTimePoint - std::chrono::microseconds(40)/*return from function call and destructors cost about 40usec*/);
			}
			was_wait_called = true;
		}
	}

	~Waiter() {
		if (!was_wait_called) {
			wait();
		}
		if (actual_waiting_time != nullptr) {
			*actual_waiting_time = std::chrono::duration_cast<Unit>(Clock::now() - actualWaitingStartTimePoint);
		}
	}

protected:
	std::chrono::time_point<Clock> startTimePoint;
	std::chrono::time_point<Clock> actualWaitingStartTimePoint;
	Type type;
	Unit* actual_waiting_time;
	Unit waitingTime;
	bool was_wait_called;
};

template <typename Unit = std::chrono::microseconds>
class LapTimer {
public:
	LapTimer(float lap_time_) : lap_time{ floatToMicsec(lap_time_) }, time_since_last_lap{ 0 } {}
	uint64_t getLaps(float deltaTime) {
		time_since_last_lap = time_since_last_lap + floatToMicsec(deltaTime);
		uint64_t laps = (uint64_t)floorf((float)time_since_last_lap.count() / (float)std::max(1ll, lap_time.count()));
		if (laps > 0) {
			time_since_last_lap = time_since_last_lap % lap_time;
		}
		return laps;
	}

	void setLapTime(float lap_time) {
		this->lap_time = floatToMicsec(lap_time);
	}

private:
	Unit lap_time;
	Unit time_since_last_lap;
};