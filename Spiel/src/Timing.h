#pragma once

#include <chrono>
#include <iostream>
#include <ostream>
#include <sstream>
#include <thread>

inline double micsecToDouble(std::chrono::microseconds mics) {
	return mics.count() * 0.000001;
}

template <typename Clock = std::chrono::high_resolution_clock ,typename Unit = std::chrono::microseconds>
class Timer {
public:
	Timer(Unit & p_output) :
		startTimePoint{Clock::now()}, output{p_output}
	{}
	~Timer() {
		auto stopTimePoint = Clock::now();
		Unit difference = std::chrono::duration_cast<Unit>(stopTimePoint - startTimePoint);
		output = difference;
	}
protected:
	std::chrono::time_point<Clock> startTimePoint;
	Unit & output;
};

template <typename Clock = std::chrono::high_resolution_clock, typename Unit = std::chrono::microseconds>
class LogTimer {
public:
	LogTimer(std::ostream& os_) :
		startTimePoint{ Clock::now() }, os{ os_ }
	{}
	~LogTimer() {
		auto stopTimePoint = Clock::now();
		Unit difference = std::chrono::duration_cast<Unit>(stopTimePoint - startTimePoint);
		std::stringstream ss;
		ss << "time taken: " << difference.count() << "us\n";
		os << ss.str();
	}
protected:
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
	Waiter(Unit waitingTime_)
		: waitingTime{waitingTime_}, was_wait_called{false}
	{
		startTimePoint = Clock::now();
	}
	void wait() {
		was_wait_called = true;
		if ((startTimePoint + waitingTime) > Clock::now()) {
			std::this_thread::sleep_until((startTimePoint + waitingTime));
		}
	}
	~Waiter() {
		if (!was_wait_called) {
			wait();
		}
	}
protected:
	std::chrono::time_point<Clock> startTimePoint;
	Unit waitingTime;
	bool was_wait_called;
};

template <typename Clock = std::chrono::high_resolution_clock, typename Unit = std::chrono::microseconds>
class BusyWaiter {
public:
	BusyWaiter(Unit waitingTime_)
		: waitingTime{ waitingTime_ }, was_wait_called{ false }
	{
		startTimePoint = Clock::now();
	}
	void wait() {

		if (!was_wait_called) {
			while (Clock::now() < waitingTime + startTimePoint - std::chrono::microseconds(40)/*return from function call and destructors cost about 40usec*/);
		}
		was_wait_called = true;
	}
	~BusyWaiter() {
		if (!was_wait_called) {
			wait();
		}
	}
protected:
	std::chrono::time_point<Clock> startTimePoint;
	Unit waitingTime;
	bool was_wait_called;
};