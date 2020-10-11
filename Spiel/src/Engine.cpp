#include "Engine.hpp"


Engine::Engine(World& wrld, std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_) :
	world{ wrld },
	running{ true },
	iteration{ 0 },
	minimunLoopTime{ 1 },	// 10000 microseconds = 10 milliseonds => 100 loops per second
	maxDeltaTime{ 0.02f },
	deltaTime{ 0.0 },
	window{ std::make_shared<Window>(windowName_, windowWidth_, windowHeight_) },
	jobManager(std::thread::hardware_concurrency()),
	collisionSystem{ world, jobManager, perfLog },
	physicsSystem2{ jobManager, perfLog },
	renderer{ window },
	in{ *window },
	ui{ renderer, in }
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
		{
			Timer mainTimer(perfLog.getInputRef("maintime"));

			// game update:
			{
				Timer t(perfLog.getInputRef("updatetime"));
				update(getDeltaTimeSafe());
			}

			// update io:
			in.engineUpdate(camera);

			// update rendering:
			ui.update();
			UIContext context;
			context.drawingPrio = 1.0f;
			context.drawMode = RenderSpace::PixelSpace;
			context.scale = guiScale;
			context.ulCorner = { 0.0f, 0.0f };
			context.drCorner = { static_cast<float>(window->width), static_cast<float>(window->height) };
			ui.draw(context);
			rendererUpdate(world);
		}
		if (glfwWindowShouldClose(window->glfwWindow)) { // if window closes the program ends
			running = false;
		}
		else {
			std::lock_guard l(window->mut);
			glfwPollEvents();

			if (!glfwGetWindowAttrib(window->glfwWindow, GLFW_FOCUSED) && in.getFocus() != Focus::Out) {
				in.takeFocus(Focus::Out);
				in.takeMouseFocus(Focus::Out);
			}
			else if (glfwGetWindowAttrib(window->glfwWindow, GLFW_FOCUSED) && in.getFocus() == Focus::Out) {
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

Drawable Engine::buildWorldSpaceDrawable(World& world, Entity entity) {
	Base& base = world.getComp<Base>(entity);
	Draw& draw = world.getComp<Draw>(entity);
	if (world.hasComp<TextureRef2>(entity)) {
		TextureRef2& texRef = world.getComp<TextureRef2>(entity);
		if (!texRef.good()) {
			// if a TexRef component was created without the renderer, it will be replaced here:
			texRef = renderer.makeTexRef(texRef.getInfo());
		}
		return Drawable(entity, base.position, draw.drawingPrio, draw.scale, draw.color, draw.form, base.rotaVec, RenderSpace::WorldSpace, texRef.makeSmall());
	}
	else {
		return Drawable(entity, base.position, draw.drawingPrio, draw.scale, draw.color, draw.form, base.rotaVec, RenderSpace::WorldSpace);
	}
}

void Engine::rendererUpdate(World& world)
{
	for (auto ent : world.entityView<Base,Draw>()) {
		auto d = buildWorldSpaceDrawable(world, ent);
		renderer.submit(d);
	}
	renderer.setCamera(camera);

	renderer.waitTillFinished();
	renderer.flushSubmissions();

	perfLog.submitTime("rendertime",renderer.getRenderingTime());
}
