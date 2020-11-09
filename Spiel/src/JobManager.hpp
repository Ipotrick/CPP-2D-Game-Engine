#ifndef _JOB_MANAGER_HPP_
#define _JOB_MANAGER_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <deque>
#include <vector>
#include <iostream>

#include <cassert>

#include "robin_hood.h"

using Tag = int;

class JobFunctor {
public:
    virtual void execute(int workerId) = 0;

    bool bEnableWaiting{ true };
    bool bSelfDestruct{ false };
};

class LambdaJob : public JobFunctor {
    std::function<void(int)> lambda;
public:
    LambdaJob(const std::function<void(int)>& lambda)
        :lambda{ lambda } {}
    LambdaJob(std::function<void(int)>&& lambda)
        :lambda{ std::move(lambda) } {}
    void execute(int workerId) override {
        lambda(workerId);
    }
};

struct JobWorkerPoolData {
    // meta:
    std::mutex mut{};
    std::condition_variable workerCV;                       // this condition variable is for waking up the worker threads
    std::condition_variable clientCV;                       // the client condition valiable is for the waking up the owning thread
    bool killWorkers{ false };
    // job data:
    std::deque<std::pair<Tag, JobFunctor*>> openJobs;       // they first in the pair is the tag the second is the job*
    robin_hood::unordered_set<Tag> closedJobs;              // the key is the tag and the int is the return value of the job

    robin_hood::unordered_set<Tag> singleFinishRequests;    // list of requests, that wait for a single job to complete
    /* 
    list of requests, that wait for multiple jobs to be completed
    the key is the pointer to the tags, the boolean is the value. the boolean represents if the request is finished.
    a request if finished, when every job in the job list is finished
    */
    robin_hood::unordered_map<std::vector<Tag>*, bool> multiFinishRequests; 

    bool areJobsFinished(const std::vector<Tag> * const jobs) {
        for (const Tag reqTag : *jobs) {
            if (!closedJobs.contains(reqTag))  
                return false;
        }
        return true;
    }

    bool isJobInMultiRequest(const Tag jobTag, const const std::vector<Tag>* multiRequest) {
        for (const auto requestTag : *multiRequest) {
            if (requestTag == jobTag) 
                return true;
        }
        return false;
    }
};

inline void jobWorkerFunction(JobWorkerPoolData* poolData, int workerId) {
    std::unique_lock<std::mutex> lock(poolData->mut);

    while (!poolData->killWorkers) {
        if (poolData->openJobs.empty())                                         // if no jobs are available, go to sleep
        {
            poolData->workerCV.wait(lock, [&]() {                               // wait till new jobs are available
                return (!poolData->openJobs.empty()) || poolData->killWorkers;
            });

            if (poolData->killWorkers) break;                                    // when killSwitch got set while sleeping we get that and kill ourselfs
        }

        auto[currentJobTag, job] = poolData->openJobs.front();
        poolData->openJobs.pop_front(); // take new job
        lock.unlock();          // unlock here as we do not need access to the syncronisation and meta data while executing a job
        job->execute(workerId);
        lock.lock();            // here we need to lock again, as this whole function needs secure access outside of the job execution
        if (job->bEnableWaiting) {
            poolData->closedJobs.emplace(currentJobTag);
        }
        if (job->bSelfDestruct) {
            delete job;
        }

        // if a client currently waits for the rescently completed job he will be notified:
        for (auto& clientRequestTag : poolData->singleFinishRequests) {
            if (clientRequestTag == currentJobTag) {
                poolData->clientCV.notify_all();
            }
        }
        // if a client waits for a bundle of jobns check if the new completed job "completes" a wait request:
        for (auto& [clientRequestTags, requestCompleted] : poolData->multiFinishRequests) {
            if (poolData->isJobInMultiRequest(currentJobTag, clientRequestTags) && poolData->areJobsFinished(clientRequestTags)) {
                requestCompleted = true;
                poolData->clientCV.notify_all();
            }
        }
    }
}

inline void helpFunction(JobWorkerPoolData* poolData, int workerId, std::vector<Tag>* myRequest) {
    std::unique_lock<std::mutex> lock(poolData->mut);
    while (true) {
        // are there jobs of our request left:
        if (poolData->openJobs.empty()) return;
        bool jobLeft = false;
        for (auto myJob : *myRequest) {
            if (poolData->openJobs.front().first == myJob) {
                jobLeft = true;
                break;
            }
        }
        if (!jobLeft) return;
        
        // execute job:
        auto [currentJobTag, job] = poolData->openJobs.front();
        poolData->openJobs.pop_front(); 
        lock.unlock();                                  // unlock here as we do not need access to the syncronisation and meta data while executing a job
        job->execute(workerId);
        lock.lock();                                    // here we need to lock again, as this whole function needs secure access outside of the job execution

        if (job->bEnableWaiting) {
            poolData->closedJobs.emplace(currentJobTag);
        }
        if (job->bSelfDestruct) {
            delete job;
        }

        // we only need to check if we completed our own multi request, as we are ONLY processing our own jobs here:
        if (poolData->areJobsFinished(myRequest)) {
            poolData->multiFinishRequests[myRequest] = true;
            poolData->clientCV.notify_all();
            return;
        }
    }
}

class JobManager {
    //meta:
    uint32_t workerNum = 0;
    int nextWorkerTag = 0;
    std::unique_ptr<JobWorkerPoolData> jobMetaData;
    std::vector<std::thread> workerThreads{};
public:
    JobManager(uint32_t hardwareThreads) 
    : workerNum{ std::max(hardwareThreads - 1, 1u) }
    {
        jobMetaData = std::make_unique<JobWorkerPoolData>();
        workerThreads.reserve(this->workerNum);
        for (int i = 0; i < this->workerNum; i++) {
            workerThreads.push_back(std::thread(jobWorkerFunction, &*jobMetaData, i));
        }
    }
    ~JobManager() {
        {
            std::lock_guard<std::mutex> lock(jobMetaData->mut);
            jobMetaData->killWorkers = true;
        }
        jobMetaData->workerCV.notify_all();
        for (auto& thread : workerThreads) thread.join();
    }

    /**
        !IMPORTANT! the job MUST NOT ME FREED while job is not finished!
    */
    int addJob(JobFunctor* job) {
        auto tag = nextWorkerTag++;
        {
            std::unique_lock<std::mutex> lock(jobMetaData->mut);
            jobMetaData->openJobs.push_back({ tag, job });
        }
        jobMetaData->workerCV.notify_one();
        return tag;
    }

    bool finished(Tag tag)
    {
        std::unique_lock<std::mutex> lock(jobMetaData->mut);
        return jobMetaData->closedJobs.contains(tag);
    }

    void clear(Tag tag)
    {
        std::unique_lock<std::mutex> lock(jobMetaData->mut);
        jobMetaData->closedJobs.erase(tag);
    }

    /*
        Waits until the job with given tag is finished.
    */
    void waitFor(const Tag job_tag) {
        std::unique_lock lock(jobMetaData->mut);
        if (!jobMetaData->closedJobs.contains(job_tag)) {
            jobMetaData->singleFinishRequests.emplace(job_tag);
            jobMetaData->clientCV.wait(lock, [&]() {
                return jobMetaData->closedJobs.contains(job_tag);
                });
            jobMetaData->singleFinishRequests.erase(job_tag);
        }
        jobMetaData->closedJobs.erase(job_tag);
    }

    /*
        Waits until a list of job tags.
    */
    void waitFor(std::vector<Tag>* const job_tags) {
        assert(job_tags);
        std::unique_lock lock(jobMetaData->mut);
        bool allJobsAllreadyFinished = true;

        if (!jobMetaData->areJobsFinished(job_tags)) {
            jobMetaData->multiFinishRequests[job_tags] = false;

            jobMetaData->clientCV.wait(lock, [&]() {
                return jobMetaData->multiFinishRequests[job_tags] == true;
                });
            jobMetaData->multiFinishRequests.erase(job_tags);
        }
        for (auto tag : *job_tags) {
            jobMetaData->closedJobs.erase(tag);
        }
    }
    /*
       Waits until a list of job tags.
       Also takes caller thread into worker pool temporarily, 
       so that it can help complete it's own job request.
   */
    void waitAndHelp(std::vector<Tag>* const job_tags) {
        if (job_tags->empty()) return;

        assert(job_tags);
        std::unique_lock lock(jobMetaData->mut);

        if (!jobMetaData->areJobsFinished(job_tags)) {
            jobMetaData->multiFinishRequests[job_tags] = false;
            lock.unlock();
            helpFunction(&*jobMetaData, workerNum, job_tags);
            lock.lock();
            // we wait for the last worker to finish our job
            jobMetaData->clientCV.wait(lock, [&]() {
                return jobMetaData->multiFinishRequests[job_tags] == true;
                });

            jobMetaData->multiFinishRequests.erase(job_tags);
        }
        for (auto tag : *job_tags) {
            jobMetaData->closedJobs.erase(tag);
        }
    }

    /*
        returns maximum amount of workers that could run in parallel
    */
    int neededBufferNum() const { return workerThreads.size()+1; }

};

#endif