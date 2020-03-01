#include "Renderer.h"

void Renderer::operator()()
{
	while (sharedData->run) {
		{
			sharedData->new_renderTime = std::chrono::microseconds(0);
			Timer<> t(sharedData->new_renderTime);
			/* process data */
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		{
			/* sync with main */
			std::unique_lock<std::mutex> switch_lock(sharedData->mut);
			sharedData->ready = true;
			sharedData->cond.notify_one();
			sharedData->cond.wait(switch_lock, [&]() { return sharedData->ready == false; });
		}
	}
}
