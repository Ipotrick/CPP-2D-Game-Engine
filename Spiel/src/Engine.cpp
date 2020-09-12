#include "Engine.hpp"


Engine::Engine(World& wrld, std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_) :
	world{ wrld },
	running{ true },
	iteration{ 0 },
	minimunLoopTime{ 1 }, // 10000 microseconds = 10 milliseond => 100 loops per second
	maxDeltaTime{ 0.02f },
	deltaTime{ 0.0 },
	window{ std::make_shared<Window>(windowName_, windowWidth_, windowHeight_) },
	jobManager(std::thread::hardware_concurrency()),
	collisionSystem{ world, jobManager, perfLog },
	physicsSystem2{ jobManager, perfLog },
	renderer{ window, world.texture }
{
	perfLog.submitTime("maintime");
	perfLog.submitTime("mainwait");
	perfLog.submitTime("updatetime");
	perfLog.submitTime("physicstime");
	perfLog.submitTime("physicsprepare");
	perfLog.submitTime("physicscollide");
	perfLog.submitTime("physicsexecute");
	perfLog.submitTime("rendertime");
	perfLog.submitTime("calcRotaVecTime");
}

Engine::~Engine() {
	renderer.end();
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

InputStatus Engine::getKeyStatus(KEY key_) {
	std::lock_guard<std::mutex> l(window->mut);
	return (InputStatus)glfwGetKey(window->glfwWindow, int(key_));
}

bool Engine::keyPressed(KEY key_) {
	return getKeyStatus(key_) == InputStatus::PRESS;
}

bool Engine::keyReleased(KEY key_) {
	return getKeyStatus(key_) == InputStatus::RELEASE;
}

bool Engine::keyRepeating(KEY key_) {
	return getKeyStatus(key_) == InputStatus::REPEAT;
}

Vec2 Engine::getCursorPos() {
	Vec2 size = getWindowSize();
	std::lock_guard<std::mutex> l(window->mut);
	double xPos, yPos;
	glfwGetCursorPos(window->glfwWindow, &xPos, &yPos);
	return { (float)xPos / size.x * 2.0f - 1.f, -(float)yPos / size.y * 2.0f +1.f };
}

InputStatus Engine::getButtonStatus(BUTTON but_) {
	std::lock_guard<std::mutex> l(window->mut);
	return static_cast<InputStatus>( glfwGetMouseButton(window->glfwWindow, static_cast<int>(but_)));
}

bool Engine::buttonPressed(BUTTON but_) {
	return getButtonStatus(but_) == InputStatus::PRESS;
}

bool Engine::buttonReleased(BUTTON but_) {
	return getButtonStatus(but_) == InputStatus::RELEASE;
}

Vec2 Engine::getWindowSize() {
	std::lock_guard<std::mutex> l(window->mut);
	return { static_cast<float>(window->width), static_cast<float>(window->height) };
}

float Engine::getWindowAspectRatio() {
	std::lock_guard<std::mutex> l(window->mut);
	return static_cast<float>(window->width)/ static_cast<float>(window->height);
}

Vec2 Engine::getPosWorldSpace(Vec2 windowSpacePos_) {
	auto transformedPos = Mat3::translate(camera.position) * Mat3::rotate(camera.rotation) * Mat3::scale(Vec2(1 / camera.frustumBend.x, 1/ camera.frustumBend.y)) * Mat3::scale(1/camera.zoom) * Vec3(windowSpacePos_.x, windowSpacePos_.y, 1);
	return { transformedPos.x, transformedPos.y };
}

Vec2 Engine::getPosWindowSpace(Vec2 worldSpacePos)
{
	Mat3 viewProjectionMatrix = Mat3::scale(camera.zoom) * Mat3::scale(camera.frustumBend) * Mat3::rotate(-camera.rotation) * Mat3::translate(-camera.position);
	return viewProjectionMatrix * worldSpacePos;
}


void Engine::run() {
	create();

	while (running) {
		Timer loopTimer(new_deltaTime);
		Waiter<> loopWaiter(minimunLoopTime, Waiter<>::Type::BUSY);
		deltaTime = micsecToFloat(new_deltaTime);
		perfLog.commitTimes();
		//commitTimeMessurements();
		freeDrawableID = 0x80000000;
		{
			Timer mainTimer(perfLog.getInputRef("maintime"));
			{
				Timer t(perfLog.getInputRef("updatetime"));
				update(getDeltaTimeSafe());
			}
			{
				rendererUpdate(world);
			}
		}
		if (glfwWindowShouldClose(window->glfwWindow)) { // if window closes the program ends
			running = false;
		}
		else {
			//std::lock_guard l(window->mut);
			glfwPollEvents();
			renderer.startRendering();
			iteration++;
		}
	}
	renderer.end();
	destroy();
}

Drawable buildWorldSpaceDrawable(World& world, Entity entity) {
	if (!world.hasComp<TexRef>(entity)) {
		return std::move(Drawable(entity, world.getComp<Base>(entity).position, world.getComp<Draw>(entity).drawingPrio, world.getComp<Draw>(entity).scale, world.getComp<Draw>(entity).color, world.getComp<Draw>(entity).form, world.getComp<Base>(entity).rotaVec));
	}
	else {
		return std::move(Drawable(entity, world.getComp<Base>(entity).position, world.getComp<Draw>(entity).drawingPrio, world.getComp<Draw>(entity).scale, world.getComp<Draw>(entity).color, world.getComp<Draw>(entity).form, world.getComp<Base>(entity).rotaVec, DrawMode::WorldSpace, world.getComp<TexRef>(entity)));
	}
}

void Engine::rendererUpdate(World& world)
{
	for (auto ent : world.entityView<Base,Draw>()) {
		renderer.submit(buildWorldSpaceDrawable(world, ent));
	}
	renderer.setCamera(camera);

	renderer.waitTillFinished();
	renderer.flushSubmissions();

	perfLog.submitTime("rendertime",renderer.getRenderingTime());
}
