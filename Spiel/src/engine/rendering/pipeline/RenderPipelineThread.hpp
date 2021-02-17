#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>

#include "../Window.hpp"
#include "RenderPipeline.hpp"

class RenderPipelineThread {
public:
	RenderPipelineThread();

	~RenderPipelineThread();

	void wait();

	enum class Action { Init, Exec, Reset, None };
	void execute(RenderPipeline* pipeline, Action action);
private:
	static void threadFunction(RenderPipelineThread* data);

	std::thread thread;
	std::mutex mtx;
	std::condition_variable cvClient;
	std::condition_variable cvWorker; 
	RenderPipeline* pipeline{ nullptr };
	Action action{ Action::None };
	bool killThread{ false };
};
