#pragma once

#include <atomic>

class Spinlock {
public:
	Spinlock() = default;

	Spinlock(std::atomic_flag& atomicFlag);

	~Spinlock();

	void lock(std::atomic_flag& atomicFlag);

	void unlock();

	bool isLocked() const { return this->atomicFlag != nullptr; }
private:
	std::atomic_flag* atomicFlag{ nullptr };
};