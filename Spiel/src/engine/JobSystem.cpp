#include "JobSystem.hpp"

JobSystem::~JobSystem()
{
	reset();
}

void JobSystem::wait(Tag tag)
{
	std::unique_lock lock(mut);
	assert(state == State::Running);
	assert(batches.contains(tag));				// can not wait for non existant job
	assert(!batches[tag].bOrphaned);			// can not wait for orphaned job
	batches[tag].bWaitedFor = true;

	clientCV.wait(lock,
		[&]() -> bool {
			return batches[tag].jobsLeft == 0;
		}
	);
	deleteJobBatch(tag);
}

void JobSystem::initialize()
{
	std::unique_lock lock(mut);
	assert(state == State::Uninitialized);
	state = State::Running;

	threads.reserve(threadCount);
	for (uint32_t id = 0; id < threadCount; ++id) {
		threads.push_back(std::thread(workerFunction, id));
		threads.back().detach();
	}
}

void JobSystem::reset()
{
	std::unique_lock lock(mut);
	assert(state == State::Uninitialized);
	state = State::Uninitialized;
	workerCV.notify_all();

	for (auto& thread : threads) thread.join();

	threads.clear();
}

void JobSystem::orphan(Tag tag)
{
	std::unique_lock lock(mut);
	assert(state == State::Running);
	if (batches.contains(tag)) {
		batches[tag].bOrphaned = true;
	}
}

bool JobSystem::finished(Tag tag)
{
	std::unique_lock lock(mut);
	assert(state == State::Running);
	assert(batches.contains(tag));

	if (batches[tag].jobsLeft == 0) {
		deleteJobBatch(tag);
		return true;
	}
	return false;
}

void JobSystem::deleteJobBatch(Tag tag)
{
	batches[tag].destructor(batches[tag].memory);
	delete batches[tag].memory;
	batches.erase(tag);
}

void JobSystem::workerFunction(const uint32_t id)
{
	std::unique_lock lock(mut);
	for (;;) {
		workerCV.wait(lock,
			[&]() {
				return !jobQueue.empty() || state == State::Uninitialized /* State::Unititialized is also used to notify all running workers to stop */;
			}
		);
		if (state == State::Uninitialized) return;

		auto [tag, job] = jobQueue.back();
		jobQueue.pop_back();

		lock.unlock();
		job->execute(id);
		lock.lock();

		auto& batch = batches[tag];
		batch.jobsLeft -= 1;
		if (batch.jobsLeft == 0) /* if there are no jobs left in a batch the job batch is completed */ {
			if (batch.bOrphaned) {
				deleteJobBatch(tag);
			}
			else if (batch.bWaitedFor) {
				clientCV.notify_one();
			}
		}
	}
}
