#include "RenderPipelineThread.hpp"

RenderPipelineThread::RenderPipelineThread()
{
	thread = std::thread(threadFunction, this);
	thread.detach();
}
RenderPipelineThread::~RenderPipelineThread()
{
	std::unique_lock lock(mtx);
	killThread = true;
	cvWorker.notify_one();
}
enum class Action { Init, Exec, Reset, None };

void RenderPipelineThread::wait()
{
	std::unique_lock lock(mtx);
	cvClient.wait(
		lock,
		[&]() {
			return action == Action::None;
		}
	);
}

void RenderPipelineThread::execute(RenderPipeline* pipeline, Action action)
{
	std::unique_lock lock(mtx);
	cvClient.wait(
		lock,
		[&]() {
			return this->action == Action::None;
		}
	);

	this->action = action;
	this->pipeline = pipeline;

	cvWorker.notify_one();
}


void RenderPipelineThread::threadFunction(RenderPipelineThread* data)
{
	std::unique_lock lock(data->mtx);
	for (;;) {
		data->cvWorker.wait(
			lock,
			[&]() -> bool {
				return data->action != Action::None || data->killThread;
			}
		);
		if (data->killThread) break;

		lock.unlock();
		switch (data->action) {
		case Action::Init:
			data->pipeline->init(); break;
		case Action::Exec:
			data->pipeline->exec(); break;
		case Action::Reset:
			data->pipeline->reset(); break;
		}
		lock.lock();

		data->action = Action::None;
		data->pipeline = nullptr;
		data->cvClient.notify_one();
	}
}