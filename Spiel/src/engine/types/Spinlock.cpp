#include "Spinlock.hpp"

Spinlock::Spinlock(std::atomic_flag& atomicFlag)
{
	lock(atomicFlag);
}

Spinlock::~Spinlock()
{
	unlock();
}

void Spinlock::lock(std::atomic_flag& atomicFlag)
{
	if (!this->atomicFlag) {
		this->atomicFlag = &atomicFlag;
		while (this->atomicFlag->test_and_set());
	}
}

void Spinlock::unlock()
{
	if (this->atomicFlag) {
		this->atomicFlag->clear();
		this->atomicFlag = nullptr;
	}
}
