#pragma once

#include <chrono>
#include <iostream>
#include <ostream>
#include <sstream>
#include <thread>

using Micsec = std::chrono::microseconds;

using StandartClock = std::chrono::system_clock;

inline float micsecToFloat(Micsec mics) 
{
	return static_cast<float>(mics.count()) * 0.000001f;
}

inline Micsec floatToMicsec(float fl) 
{
	return Micsec( static_cast<uint32_t>(fl * 1'000'000.0f) );
}

inline std::ostream& operator<<(std::ostream& os, Micsec us)
{
	return os << us.count() << "us";
}

class Timer {
public:
	Timer(Micsec& outTime) :
		startTimePoint{ StandartClock::now()}, output{ outTime }, bStopped{ false }
	{}
	~Timer() {
		if (!bStopped) {
			stop();
		}
	}

	void stop() {
		auto stopTimePoint = StandartClock::now();
		Micsec difference = std::chrono::duration_cast<Micsec>(stopTimePoint - startTimePoint);
		output = difference;
		bStopped = true;
	}
protected:
	bool bStopped;
	std::chrono::time_point<StandartClock> startTimePoint;
	Micsec& output;
};

class LogTimer {
public:
	LogTimer(std::ostream& os, std::string_view const& pre = "time taken: ") :
		preString{ pre }, startTimePoint{ StandartClock::now() }, os{ os }
	{}
	~LogTimer() {
		if (!bStopped) {
			stop();
		}
	}
	void stop() {
		auto stopTimePoint = StandartClock::now();
		Micsec difference = std::chrono::duration_cast<Micsec>(stopTimePoint - startTimePoint);
		std::stringstream ss;
		ss << preString << difference.count() << "us\n";
		os << ss.str();
		bStopped = true;
	}
protected:
	std::string_view preString;
	std::chrono::time_point<StandartClock> startTimePoint;
	std::ostream& os;
	bool bStopped{ false };
};

class Waiter {
public:
	enum class Type {
		SLEEPY,
		BUSY
	};
	Waiter(Micsec waitingTime, Type type = Type::SLEEPY, Micsec* outRealWaittime = nullptr)
		: waitingTime{ waitingTime }, bWaitCalled{ false }, realWaitTime{ outRealWaittime}, type{ type }
	{
		startTimePoint = StandartClock::now();
	}

	void wait() {
		if (type == Type::SLEEPY) {
			actualWaitingStartTimePoint = StandartClock::now();
			bWaitCalled = true;
			if ((startTimePoint + waitingTime) > StandartClock::now()) {
				std::this_thread::sleep_until((startTimePoint + waitingTime));
			}
		}
		else {
			actualWaitingStartTimePoint = StandartClock::now();
			if (!bWaitCalled) {
				while (StandartClock::now() < waitingTime + startTimePoint - std::chrono::microseconds(40)/*return from function call and destructors cost about 40usec*/);
			}
			bWaitCalled = true;
		}
	}

	~Waiter() {
		if (!bWaitCalled) {
			wait();
		}
		if (realWaitTime != nullptr) {
			*realWaitTime = std::chrono::duration_cast<Micsec>(StandartClock::now() - actualWaitingStartTimePoint);
		}
	}

protected:
	std::chrono::time_point<StandartClock> startTimePoint;
	std::chrono::time_point<StandartClock> actualWaitingStartTimePoint;
	Type type;
	Micsec* realWaitTime;
	Micsec waitingTime;
	bool bWaitCalled;
};

class LapTimer {
public:
	LapTimer() = default;
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

	float getLapTime() const { return micsecToFloat(lap_time); }

private:
	Micsec lap_time{ Micsec() };
	Micsec time_since_last_lap{ Micsec() };
};