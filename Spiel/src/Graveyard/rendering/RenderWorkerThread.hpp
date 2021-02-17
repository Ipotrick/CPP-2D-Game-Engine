#pragma once

#include <deque>
#include <variant>

#include "RenderingWorker.hpp"
#include "pipeline/RenderPipeline.hpp"

class RenderWorkerThread {
public:
	using Tag = u64;

	~RenderWorkerThread();

	enum class Action {
		Init,
		Update,
		Reset
	};

	static Tag submit(RenderingWorker* workerdata, Action action);

	static Tag submit(RenderPipeline* pipeline, Action action);

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
			order{ workerData }, action{ action }
		{}

		Job(RenderPipeline* workerData, Action action) :
			order{ workerData }, action{ action }
		{}

		std::variant<RenderingWorker*, RenderPipeline*> order;
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