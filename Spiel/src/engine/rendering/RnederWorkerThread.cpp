#include "RenderWorkerThread.hpp"

RenderWorkerThread::~RenderWorkerThread()
{
	if (state == State::Running) {
		reset();
	}
}

void RenderWorkerThread::wait(Tag tag)
{
	std::unique_lock lock(mut);
	assert(state == State::Running);
	assert(batches.contains(tag));				// can not wait for non existant job
	assert(!batches[tag].bOrphaned);			// can not wait for orphaned job
	jobs[tag].bWaitedFor = true;

	clientCV.wait(lock,
		[&]() -> bool {
			return jobs[tag].bFinished;
		}
	);
	jobs.erase(tag);
}

bool RenderWorkerThread::exists(Tag tag)
{
	std::unique_lock lock(mut);
	return jobs.find(tag) != jobs.end();
}

void RenderWorkerThread::init()
{
	std::unique_lock lock(mut);
	assert(state == State::Uninitialized);
	state = State::Running;

	thread = std::thread(workerFunction);
}

void RenderWorkerThread::reset()
{
	std::unique_lock lock(mut);
	assert(state == State::Uninitialized);
	state = State::Uninitialized;
	workerCV.notify_one();

	thread.join();
}

void RenderWorkerThread::workerFunction()
{
	std::unique_lock lock(mut);
	for (;;) {
		workerCV.wait(lock,
			[&]() {
				return !jobQueue.empty() || state == State::Uninitialized /* State::Unititialized is also used to notify all running workers to stop */;
			}
		);
		if (state == State::Uninitialized) return;

		Tag tag = jobQueue.back();
		Job* job = &jobs[tag];
		jobQueue.pop_back();

		lock.unlock();
		switch (job->action) {
		case Action::Init:
			job->workerData->initialize(); break;
		case Action::Update:
			job->workerData->update(); break;
		case Action::Reset:
			job->workerData->reset(); break;
		}
		lock.lock();

		job = &jobs[tag];
		job->bFinished = true;
		if (job->bWaitedFor) {
			clientCV.notify_one();
		}
	}
}
