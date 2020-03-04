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
	enum class Type {
		SLEEPY,
		BUSY
	};
	Waiter(Unit waiting_time_, Type type_ = Type::SLEEPY, Unit* actual_waiting_time_ = nullptr)
		: waitingTime{ waiting_time_ }, was_wait_called{ false }, actual_waiting_time{ actual_waiting_time_}, type{ type_ }
	{
		startTimePoint = Clock::now();
	}

	virtual void wait() {
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

/*

template <typename Clock = std::chrono::high_resolution_clock, typename Unit = std::chrono::microseconds>
class SleepyWaiter : _Waiter<Clock, Unit> {
public:
	SleepyWaiter(Unit waiting_time_, Unit* actual_waiting_time_) : _Waiter<Clock, Unit>(waiting_time_, actual_waiting_time_) {}
	virtual void wait() {
		_Waiter<Clock, Unit>::actualWaitingStartTimePoint = Clock::now();
		_Waiter<Clock, Unit>::was_wait_called = true;
		if ((_Waiter<Clock, Unit>::startTimePoint + _Waiter<Clock, Unit>::waitingTime) > Clock::now()) {
			std::this_thread::sleep_until((_Waiter<Clock, Unit>::startTimePoint + _Waiter<Clock, Unit>::waitingTime));
		}
	}
	~SleepyWaiter() {
		if (!was_wait_called) {
			wait();
		}
		if (actual_waiting_time != nullptr) {
			*actual_waiting_time = std::chrono::duration_cast<Unit>(Clock::now() - actualWaitingStartTimePoint);
		}
	}
};

template <typename Clock = std::chrono::high_resolution_clock, typename Unit = std::chrono::microseconds>
class BusyWaiter : _Waiter<Clock, Unit> {
public:
	BusyWaiter(Unit waiting_time_, Unit* actual_waiting_time_) : _Waiter<Clock, Unit>(waiting_time_, actual_waiting_time_) {}
	virtual void wait() {
		_Waiter<Clock, Unit>::actualWaitingStartTimePoint = Clock::now();
		if (!_Waiter<Clock, Unit>::was_wait_called) {
			while (Clock::now() < _Waiter<Clock, Unit>::waitingTime + _Waiter<Clock, Unit>::startTimePoint - std::chrono::microseconds(40)/*return from function call and destructors cost about 40usec);
		}
		_Waiter<Clock, Unit>::was_wait_called = true;
	}
	~BusyWaiter() {
		if (!was_wait_called) {
			wait();
		}
		if (actual_waiting_time != nullptr) {
			*actual_waiting_time = std::chrono::duration_cast<Unit>(Clock::now() - actualWaitingStartTimePoint);
		}
	}
};

*/

/*

template <typename Clock = std::chrono::high_resolution_clock, typename Unit = std::chrono::microseconds>
class SleepyWaiter {
public:
	SleepyWaiter(Unit waitingTime_)
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
	~SleepyWaiter() {
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
			while (Clock::now() < waitingTime + startTimePoint - std::chrono::microseconds(40)/*return from function call and destructors cost about 40usec);
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

*/