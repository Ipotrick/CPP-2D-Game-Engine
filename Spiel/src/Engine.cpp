#include "Engine.hpp"

using namespace std::literals::chrono_literals;

Engine::Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_)
{
	/*
	* there can only be one engine instance at a time
	*/
	if (bInstantiated) {
		std::cerr << "ERRROR: there can only be one instance of an engine at a time!" << std::endl;
		exit(-1);
	}
	bInstantiated = true;

	running = true;
	iteration = 0;
	minimunLoopTime = 1ms;	// 10000 microseconds = 10 milliseonds => 100 loops per second
	maxDeltaTime = 0.02f;
	deltaTime = 0.0;
	window.open(windowName_, windowWidth_, windowHeight_);
	renderer.initialize(&window);
	running = true;

	perfLog.submitTime("maintime");
	perfLog.submitTime("mainwait");
	perfLog.submitTime("updatetime");
	perfLog.submitTime("physicstime");
	perfLog.submitTime("physicsprepare");
	perfLog.submitTime("physicscollide");
	perfLog.submitTime("physicsexecute");
	perfLog.submitTime("rendertime");
	perfLog.submitTime("calcRotaVecTime");
	deltaTimeQueue.push_back(maxDeltaTime);
}

Engine::~Engine()
{
	/*
	* there can only be one engine instance at a time
	*/
	if (!bInstantiated) {
		std::cerr << "ERRROR: there can only be one instance of an engine at a time!" << std::endl;
		exit(-1);
	}
	bInstantiated = false;

	renderer.end();
	window.close();
}

float Engine::getDeltaTime(int sampleSize)
{
	if (sampleSize == 1) {
		return deltaTimeQueue[0];
	}
	else {
		int maxSamples = std::min(deltaTimeQueue.size(), 100ui64);

		sampleSize = std::min(sampleSize, maxSamples);
		float sum{ 0.0f };
		for (int i = 0; i < sampleSize; ++i) {
			sum += deltaTimeQueue[i];
		}
		return sum / sampleSize;
	}
}

std::string Engine::getPerfInfo(int detail) {
	std::stringstream ss;
	if (detail >= 4) ss << "Entity Max: " << world.maxEntityIndex() << "\n";
	if (detail >= 1) ss << "Entity Count: " << world.size() << "\n";
	if (detail >= 1) {
		ss << "    deltaTime(s): " << getDeltaTime() << "\n"
			<< "    Ticks/s: " << 1 / getDeltaTime() << "\n"
			<< "    simspeed: " << getDeltaTimeSafe() / getDeltaTime() << '\n';
	}
	if (detail >= 2) {
		ss << "        update(s): " << perfLog.getTime("updatetime") << "\n"

			<< "        physics(s): " << perfLog.getTime("physicstime") << '\n';
	}
	if (detail >= 3) {
		ss << "            collision prepare:      " << perfLog.getTime("collisionprepare") << '(' << floorf(perfLog.getTime("collisionprepare") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)\n"
			<< "            collision borad phase:  " << perfLog.getTime("collisionbroad") << '(' << floorf(perfLog.getTime("collisionbroad") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)\n"
			<< "            collision narrow phase: " << perfLog.getTime("collisionnarrow") << '(' << floorf(perfLog.getTime("collisionnarrow") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)\n"
			<< "            collision postamble:    " << perfLog.getTime("collisionpost") << '(' << floorf(perfLog.getTime("collisionpost") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)\n"
			<< "            physics prepare:        " << perfLog.getTime("physicsprepare") << '(' << floorf(perfLog.getTime("physicsprepare") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)\n"
			<< "            physics impulse:        " << perfLog.getTime("physicsimpulse") << '(' << floorf(perfLog.getTime("physicsimpulse") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)\n"
			<< "            physics other:        " << perfLog.getTime("physicsrest") << '(' << floorf(perfLog.getTime("physicsrest") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)\n";
	}
	if (detail >= 1) ss << "    renderTime(s): " << perfLog.getTime("rendertime") << '\n';

	return ss.str();
}

Vec2 Engine::getWindowSize() {
	std::lock_guard<std::mutex> l(window.mut);
	return { static_cast<float>(window.width), static_cast<float>(window.height) };
}

float Engine::getWindowAspectRatio() {
	std::lock_guard<std::mutex> l(window.mut);
	return static_cast<float>(window.width)/ static_cast<float>(window.height);
}

void Engine::run() {
	create();

	while (running) {
		Timer loopTimer(new_deltaTime);
		Waiter loopWaiter(minimunLoopTime, Waiter::Type::BUSY);
		deltaTime = micsecToFloat(new_deltaTime);
		if (deltaTimeQueue.size() > 99) {
			deltaTimeQueue.pop_back();
		}
		deltaTimeQueue.push_front(deltaTime);

		perfLog.commitTimes();
		{
			Timer mainTimer(perfLog.getInputRef("maintime"));

			// game update:
			{
				Timer t(perfLog.getInputRef("updatetime"));
				update(getDeltaTimeSafe());
			}

			// update in:
			in.engineUpdate(renderer.getCamera());

			// update rendering:
			ui.update();
			UIContext context;
			context.drawingPrio = 1.0f;
			context.drawMode = RenderSpace::PixelSpace;
			context.scale = guiScale;
			context.ulCorner = { 0.0f, static_cast<float>(window.height) };
			context.drCorner = { static_cast<float>(window.width), 0.0f };
			ui.draw(context);
			rendererUpdate(world);
		}
		if (glfwWindowShouldClose(window.glfwWindow)) { // if window closes the program ends
			running = false;
		}
		else {
			if (!glfwGetWindowAttrib(window.glfwWindow, GLFW_FOCUSED) && in.getFocus() != Focus::Out) {
				in.takeFocus(Focus::Out);
				in.takeMouseFocus(Focus::Out);
			}
			else if (glfwGetWindowAttrib(window.glfwWindow, GLFW_FOCUSED) && in.getFocus() == Focus::Out) {
				in.returnFocus();
				in.returnMouseFocus();
			}
			renderer.startRendering();
			iteration++;
		}
	}
	renderer.end();
	destroy();
}

void Engine::rendererUpdate(World& world)
{
	renderer.waitTillFinished();
	renderer.flushSubmissions();

	perfLog.submitTime("rendertime",renderer.getRenderingTime());
}
