#include "JobSystem.hpp"

void JobSystem::wait(Tag tag)
{
	std::unique_lock lock(mut);
	assert(state == Running);

	waiterCV.wait(lock,
		[&]() -> bool {
			return batches[tag].jobsLeft == 0;
		}
	);

	//delete batches[tag].memory;

	batches.erase(tag);
}

void JobSystem::initialize()
{
	std::unique_lock lock(mut);
	state = State::Running;

	threads.reserve(threadCount);
	for (uint32_t id = 0; id < threadCount; ++id) {
		threads.push_back(std::thread(workerFunction, id));
	}
	for (auto& thread : threads) {
		thread.detach();
	}
}

void JobSystem::reset()
{
	std::unique_lock lock(mut);
	state = State::Uninitialized;
	workerCV.notify_all();

	threads.clear();
}

void JobSystem::workerFunction(const uint32_t id)
{
	std::unique_lock lock(mut);
	for (;;) {
		workerCV.wait(lock,
			[&]() {
				return !jobQueue.empty() || state == State::Uninitialized;
			}
		);
		if (state == State::Uninitialized) return;

		auto [job, tag] = jobQueue.back();
		jobQueue.pop_back();

		lock.unlock();
		job->execute(id);
		lock.lock();

		auto& batch = batches[tag];
		batch.jobsLeft -= 1;
		if (batch.jobsLeft == 0) {
			waiterCV.notify_one();
		}
	}
}
