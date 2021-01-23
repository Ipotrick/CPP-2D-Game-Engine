#pragma once

#include <deque>

#include "RenderingWorker.hpp"

class RenderWorkerThread {
public:
	using Tag = u64;

	~RenderWorkerThread();

	enum class Action {
		Init,
		Update,
		Reset
	};

	static u64 submit(RenderingWorker* workerdata, Action action)
	{
		std::unique_lock lock(mut);
		assert(state == State::Running);
		Tag tag = nextJobTag++;

		auto newjob = Job(workerdata, action);
		jobs[tag] = newjob;
		jobQueue.push_back(tag);

		workerCV.notify_one();
		return tag;
	}

	/**
	 * Stops the curret thread until job batch belonging to the given tag is finished.
	 * Deletes memory for job batch at the end of the function.
	 * Deletes Tag from System after call.
	 *
	 * Call with invalid tag will cause an assertion failure.
	 *
	 * \param tag used to identify the job batch.
	 */
	static void wait(Tag tag);

	static bool exists(Tag tag);

	static void init();

	static void reset();

private:

	/**
	 * Contains either a ThreadJob or a vector of ThreadJob's.
	 */
	struct Job {
		Job() = default;

		Job(RenderingWorker* workerData, Action action) :
			workerData{ workerData }, action{ action }
		{}

		RenderingWorker* workerData;
		Action action;
		bool bFinished{ false };
		bool bWaitedFor{ false };
	};

	static void workerFunction();

	// used to check for uninitialized use
	enum class State {
		Uninitialized,
		Running
	};
	inline static std::thread thread;

	inline static std::mutex mut;						// central syncronization mutex used for every read wnad write to the fields below:
	/// 
	/// THE FOLLOWING FIELDS ARE GUARDED BY THE MUTEX MUT:
	/// 
	inline static State state{ State::Uninitialized };	// used for checking uninitialized use
	inline static std::condition_variable workerCV;		// cv used by the worker threads to get informed when jobs is in queue
	inline static std::condition_variable clientCV;		// cv used by threads that are waiting for a job batch to be finished
	inline static Tag nextJobTag{ 0 };					// Job tags are created sequentially, beginning at 0 after application start.
	inline static std::deque<Tag> jobQueue;
	inline static std::unordered_map<Tag, Job> jobs;
};