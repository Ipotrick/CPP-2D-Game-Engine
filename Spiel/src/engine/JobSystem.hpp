#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <cinttypes>
#include <deque>
#include <vector>
#include <unordered_map>
#include <cassert>

class JobSystem {
public:
	using Tag = uint64_t;

	/**
	 * Derive a new Job from this class.
	 */
	class Job {
	public:
		virtual void execute(const uint32_t threadId) = 0;
	private:
	};

	template<typename TJob>
	static Tag submit(TJob&& job)
	{
		static_assert(std::is_base_of<Job, TJob>::value, "a job must be derived from the ThreadJob");
		assert(state == Running);
		std::unique_lock lock(mut);
		uint32_t tag = nextJobTag++;
	
		auto newbatch = JobBatch(new TJob(std::move(job)), 1ull );
		batches[tag] = newbatch;
		jobQueue.push_back({ static_cast<Job*>(newbatch.memory), tag });
	
		workerCV.notify_one();
	
		return tag;
	}

	/**
	 * This only takes r value references and takes ownership of the vector.
	 * move the vector with std::move(vec) into the parameter.
	 * 
	 * \param jobList is a list of jobs that derive from JobSystem::ThreadJob.
	 * \return a uint32_t tag as a ticket to wait for completion of the vector of jobs.
	 */
	template<typename TJob>
	static Tag submitVec(std::vector<TJob>&& jobList)
	{
		static_assert(std::is_base_of<Job, TJob>::value, "a job must be derived from the ThreadJob");
		assert(state == Running);
		std::unique_lock lock(mut);
		uint32_t tag = nextJobTag++;

		size_t jobListSize = jobList.size();
		JobBatch newbatch = { new std::vector<TJob>(std::move(jobList)), jobListSize };
		batches[tag] = newbatch;

		std::vector<TJob>& jobsInMem = *static_cast<std::vector<TJob>*>(newbatch.memory);
		for (auto& job : jobsInMem) {
			jobQueue.push_back(std::pair{ static_cast<Job*>(&job), tag });
		}

		workerCV.notify_all();

		return tag;
	}

	static void wait(Tag tag);

	static void initialize();

	static void reset();

	static size_t workerCount() { return threadCount; }

private:

	/**
	 * Contains either a ThreadJob or a vector of ThreadJob's.
	 * memory holds heap data of either the vector or the single job.
	 */
	struct JobBatch {
		JobBatch() = default;
		
		JobBatch(void* memory, size_t jobCount) :
			memory{ memory }, jobsLeft{ jobCount }
		{}
		void* memory{ nullptr };
		size_t jobsLeft{ 0 };
	};

	enum class State {
		Uninitialized,
		Running
	};

	static void workerFunction(const uint32_t id);

	inline static State state{ State::Uninitialized };
	inline static const size_t threadCount{ std::max(std::thread::hardware_concurrency()-1, 1u) };
	inline static std::vector<std::thread> threads;

	inline static std::mutex mut;
	inline static std::condition_variable workerCV;
	inline static std::condition_variable waiterCV;

	inline static Tag nextJobTag{ 0 };
	inline static std::deque<std::pair<Job*, Tag>> jobQueue;
	inline static std::unordered_map<Tag, JobBatch> batches;
};