#ifndef _JOB_MANAGER_HPP_
#define _JOB_MANAGER_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <deque>
#include <vector>

class JobFunctor {
public:
    virtual int operator()() = 0;
};

struct JobWorkerPoolData {
    // meta:
    std::mutex mut{};
    std::condition_variable workerCV;                         // this condition variable is for waking up the worker threads
    std::condition_variable clientCV;                         // the client condition valiable is for the waking up the owning thread
    bool killWorkers{ false };
    // job data:
    std::deque<std::pair<uint32_t, JobFunctor*>> openJobs{};  // they first in the pair is the tag the second is the job*
    std::unordered_map<uint32_t, int> closedJobs{};           // the key is the tag and the int is the return value of the job
};

inline void jobWorkerFunction(std::shared_ptr<JobWorkerPoolData> poolData) {
    std::unique_lock<std::mutex> lock(poolData->mut);

    while (!poolData->killWorkers) { 
        if (poolData->openJobs.empty()) // if no jobs are available, go to sleep
        {
            poolData->workerCV.wait(lock, [&]() { // wait thill new jobs are available
                return (!poolData->openJobs.empty()) || poolData->killWorkers;
            });
            if (poolData->killWorkers) break;   // when killSwitch got set while sleeping we get that and kill ourselfs
        }
        auto [tag, job] = poolData->openJobs.front(); poolData->openJobs.pop_front(); // take new job
        int returnCode = -2;
        {
            lock.unlock();          // unlock here as we do not need access to the syncronisation and meta data while executing a job
            returnCode = (*job)();  // job is a pointer to a functor (*job) dereferences the poihnter and () calls the functor function
            lock.lock();            // here we need to lock again, as this whole function needs secure access outside of the job execution
        }
        poolData->closedJobs[tag] = returnCode; 
        poolData->clientCV.notify_all();        // notify the waiting client(s), so they can checkj if their job is finished
    }
}

class JobManager {
    //meta:
    int workerCount = 0;
    int nextWorkerTag = 0;
    std::shared_ptr<JobWorkerPoolData> poolData;
    std::vector<std::thread> workerThreads{};
public:
    JobManager(int workerCount) 
    : workerCount{ workerCount } 
    {
        poolData = std::make_shared<JobWorkerPoolData>();
        workerThreads.reserve(workerCount);
        for (int i = 0; i < workerCount; i++) {
            workerThreads.push_back(std::thread(jobWorkerFunction, poolData));
        }
    }

    /**
        !IMPORTANT! the job MUST NOT ME FREED while job is not finished!
        ALLWAYS free the job after waiting for it to finish!
    */
    int addJob(JobFunctor* job) {
        auto tag = nextWorkerTag++;
        {
            std::lock_guard lock(poolData->mut);
            poolData->openJobs.push_back({ tag, job });
            poolData->workerCV.notify_one();
        }
        return tag;
    }

    /*
        Waits until the job with given tag is finished.
        returns the return value of the job.
    */
    int waitFor(int job_tag) {
        std::unique_lock lock(poolData->mut);
        int returnCode = -2;
        poolData->clientCV.wait(lock, [&]() {
            for (auto& [tag, retCode] : poolData->closedJobs) {
                if (tag == job_tag) {
                    returnCode = retCode;
                    return true;
                }
            }
            return false;
        });
        poolData->closedJobs.erase(job_tag);
        return returnCode;
    }

    // kills job workers
    void end() {
        {
            std::lock_guard<std::mutex> lock(poolData->mut);
            poolData->killWorkers = true;
            poolData->workerCV.notify_all();
        }
        for (auto& thread : workerThreads) thread.join();
    }
};

#endif