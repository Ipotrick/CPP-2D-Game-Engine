#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <cinttypes>
#include <deque>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <functional>

// TODO maybe move it into some sort of reflection hpp
template<typename T>
void deletor(void* el)
{
	reinterpret_cast<T*>(el)->~T();
}

/**
 * abstact Interface class for jobs.
 */
class IJob {
public:
	virtual void execute(const uint32_t threadId) = 0;
};

class LambdaJob : public IJob {
public:
	LambdaJob(std::function<void(uint32_t)>&& lambda) : lambda{ lambda }{}
	virtual void execute(const uint32_t threadId) override
	{
		lambda(threadId);
	}
private:
	std::function<void(uint32_t)> lambda;
};

template<typename T>
/**
 * Concept for a job class that derives the IJob interface.
 */
concept CJob = std::is_base_of_v<IJob, T>;

class JobSystem {
public:
	using Tag = uint64_t;

	~JobSystem();

	/**
	 * Submits a job to be executed in parallel by worker threads.
	 * The job that is submitted must be an RVALUE.
	 * The Jobsystem takes the ownership of the memory of the job.
	 * After submitting a job, it is illigal to use the jobs memory.
	 * The Tag one gets from this function can be used to get information about job after submission, for example wait(Tag) or finished(Tag).
	 * 
	 * \param job is a class that derives from the Base Class IJob, and implements the function void execute(uint32_t thread).
	 * \return tag that is used to identify the job. 
	 */
	template<CJob TJob>
	static Tag submit(TJob&& job)
	{
		std::unique_lock lock(mut);
		assert(state == State::Running);
		Tag tag = nextJobTag++;
		
		TJob* jobMemPtr = new TJob(std::move(job));
		auto newbatch = JobBatch((void*)jobMemPtr, 1ull );
		newbatch.destructor = deletor<TJob>;
		batches[tag] = newbatch;
		jobQueue.push_back({ tag, static_cast<IJob*>(jobMemPtr) });
	
		workerCV.notify_one();
	
		return tag;
	}

	/**
	 * Submits a list of jobs to be executed in parallel by worker threads.
	 * The job list that is submitted must be an RVALUE.
	 * The Jobsystem takes the ownership of the memory of the job list.
	 * After submitting a job list, it is illigal to use the jobs memory.
	 * The Tag one gets from this function can be used to get information about the job list after submission, for example wait(Tag) or finished(Tag).
	 *
	 * \param jobList is a vector that contains objects of a class that derives from the Base Class IJob, and implements the function void execute(uint32_t thread).
	 * \return tag that is used to identify the job.
	 */
	template<CJob TJob, typename TAllocator>
	static Tag submitVec(std::vector<TJob, TAllocator>&& jobList)
	{
		std::unique_lock lock(mut);
		assert(state == State::Running);
		Tag tag = nextJobTag++;

		const size_t jobListSize = jobList.size();
		std::vector<TJob, TAllocator>* jobMemPtr = new std::vector<TJob, TAllocator>(std::move(jobList));
		JobBatch newbatch = { (void*)jobMemPtr, jobListSize };
		newbatch.destructor = deletor<std::vector<TJob, TAllocator>>;
		batches[tag] = newbatch;

		for (auto& job : *jobMemPtr) {
			jobQueue.push_back({ tag, static_cast<IJob*>(&job) });
		}

		workerCV.notify_all();

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

	/**
	 * Checks if job is finished.
	 * When returnd false the job is either still in queue or still in execution.
	 * When returnd true the job batch's memory is released and the tag is deleted after call.
	 * 
	 * Call with invalid tag will cause an assertion failure.
	 * 
	 * \param tag used to identify the job batch.
	 * \return true when job is finished.
	 */
	static bool finished(Tag tag);

	/**
	 * Orphans a job batch. 
	 * After orphaning a job batch, the tag is no longer available for other calls like finished(Tag) or wait(tag).
	 * orphaned job batch's memory will be released immediately after the job is finished.
	 * 
	 * trying to call orphan on an invalid tag will be caught and nothing will happen.
	 * 
	 * \param tag used to identify the job batch.
	 */
	static void orphan(Tag tag);

	static void initialize();

	static void reset();

	/**
	 * \return number of worker threads.
	 */
	static size_t workerCount() { return threadCount; }

private:

	/**
	 * removes tag/batch from batches map.
	 * calls destructor of job batch.
	 * frees memory of job batch.
	 * 
	 * \param tag of job to delete.
	 */
	static void deleteJobBatch(Tag tag);

	/**
	 * Contains either a ThreadJob or a vector of ThreadJob's.
	 */
	struct JobBatch {
		JobBatch() = default;
		
		JobBatch(void* memory, size_t jobCount) :
			memory{ memory }, jobsLeft{ jobCount }
		{}

		/**
		 * Type erased batch memory ptr.
		 */
		void* memory{ nullptr };

		/**
		 * The destructor of the job batch will vary as the JobManager uses type erasure to store the job batches memory ptr.
		 * this std::fucntion stores the destructor of the job container.
		 * The constructor is called on destruction of the job batch.
		 */
		std::function<void(void*)> destructor;

		/**
		 * A batch can contain >= 1 jobs initially.
		 * As the jobsLeft falls to 0, the job is marked as completed.
		 */
		size_t jobsLeft{ 0 };

		/**
		 * For an orphan job batch, the tag is no longer available for other calls like finished(Tag) or wait(tag).
		 * orphaned job batch's memory will be released immediately after the job is finished.
		 */
		bool bOrphaned{ false };

		/**
		 * If a thread waits for a job batch to finish it is marked by this flag.
		 * Therefore, when this flag is set, a thread is waiting so he needs to be notified on completion or the job batch, with the clientCV.
		 * This gives us the advantage that we only notify the clientCV, when the right client is waiting.
		 * We dont want to notify every client for every completed job, wich would cause a lot of contention on the mutex.
		 */
		bool bWaitedFor{ false };
	};

	static void workerFunction(const uint32_t id);

	// used to check for uninitialized use
	enum class State {
		Uninitialized,
		Running
	};
	inline static const size_t threadCount{ std::max(std::thread::hardware_concurrency()-1, 1u) };	// the worker count is only n-1 hardwarethreads, as we dont want to pollute the os with threads.
	inline static std::vector<std::thread> threads;

	inline static std::mutex mut;						// central syncronization mutex used for every read wnad write to the fields below:
	/// 
	/// THE FOLLOWING FIELDS ARE GUARDED BY THE MUTEX MUT:
	/// 
	inline static State state{ State::Uninitialized };												// used for checking uninitialized use
	inline static std::condition_variable workerCV;		// cv used by the worker threads to get informed when jobs is in queue
	inline static std::condition_variable clientCV;		// cv used by threads that are waiting for a job batch to be finished
	inline static Tag nextJobTag{ 0 };					// Job tags are created sequentially, beginning at 0 after application start.
	inline static std::deque<std::pair<Tag, IJob*>> jobQueue;
	inline static std::unordered_map<Tag, JobBatch> batches;
};