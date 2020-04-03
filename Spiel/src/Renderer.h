#pragma once

#include <mutex>
#include <condition_variable>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Timing.h"
#include "BaseTypes.h"
#include "RenderTypes.h"
#include "PhysicsTypes.h"
#include "Camera.h"
#include "window.h"

static unsigned int compileShader(unsigned int type_, const std::string source_) {
	unsigned id = glCreateShader(type_);
	char const* src = source_.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)_malloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cerr << "error: failed to compile shader: " << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned createShader(const std::string& vertexShader_, const std::string& fragmentShader_) {
	unsigned program = glCreateProgram();
	unsigned vs = compileShader(GL_VERTEX_SHADER, vertexShader_);
	unsigned fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader_);

	assert(vs != 0);
	assert(fs != 0);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

struct ShadowVolume {
	ShadowVolume(vec2 corners_[4] , uint32_t lightID_) : lightID{ lightID_ } {
		corners[0] = corners_[0];
		corners[1] = corners_[1];
		corners[2] = corners_[2];
		corners[3] = corners_[3];
	}
	vec2 corners[4];
	uint32_t lightID;
};

class RenderBuffer {
public:
	std::vector<Drawable> worldSpaceDrawables;
	std::vector<Drawable> windowSpaceDrawables;
	Camera camera;
};

struct RendererSharedData {
	RendererSharedData():
		run{ true },
		ready{ false },
		new_renderTime{ 0 },
		new_renderSyncTime{ 0 },
		mut{  },
		cond{  }
	{}
	/* DO NOT ACCESS THESE, WHILE RENDER THREAD IS RUNNING, FROM OTHER THREAD */
	RenderBuffer renderBufferB;
	std::vector<Light> lights;
	std::vector<CollisionInfo> lightCollisions;
	std::chrono::microseconds new_renderTime;
	std::chrono::microseconds new_renderSyncTime;
	bool run;
	/* ONLY USE THERE WITH A MUTEX LOCK */
	std::mutex mut;
	bool ready;
	std::condition_variable cond;
};

class Renderer
{
public:
	Renderer(std::shared_ptr<RendererSharedData> sharedData_, std::shared_ptr<Window> window_) :
		sharedData{ sharedData_ },
		window{window_}
	{
	}

	void initiate();
	void operator()();
	void end();


	std::string readShader(std::string path_);
	
private:
	std::shared_ptr<RendererSharedData> sharedData;
	std::shared_ptr<Window> window;

	unsigned int shader;
	unsigned int shadowShader;
	const std::string vertexShaderPath = "shader/Basic.vert";
	const std::string fragmentShaderPath = "shader/Basic.frag";
	const std::string vertexShadowShaderPath = "shader/shadow.vert";
	const std::string fragmentShadowShaderPath = "shader/shadow.frag";
private:
	void drawWorldSpace(Drawable const& d, mat4 const& viewProjectionMatrix);
	void drawWindowSpace(Drawable const& d);
};

/*	DEPRECATED
inline ShadowVolume generateShadowVolumes(Light const& l, Drawable const& d) {
	auto translatedOtherPos = d.position - l.position;
	auto rotatedOtherPos = rotate(translatedOtherPos, -getAngle(translatedOtherPos));

	vec2 highgestShadowEdge;
	vec2 lowestShadowEdge;

	if (d.form == Form::CIRCLE) {
		highgestShadowEdge = rotatedOtherPos + vec2(0, d.scale.x / 2.0f);
		lowestShadowEdge = rotatedOtherPos - vec2(0, d.scale.x / 2.0f);
	}
	else {	//rect
		std::array<vec2, 4> corners = {
			vec2(d.scale.x * 0.5f,  d.scale.y * 0.5f),
			vec2(d.scale.x * 0.5f, -d.scale.y * 0.5f),
			vec2(-d.scale.x * 0.5f,  d.scale.y * 0.5f),
			vec2(-d.scale.x * 0.5f, -d.scale.y * 0.5f)
		};
		for (auto& el : corners) el = rotate(el, d.rotation - getAngle(translatedOtherPos));
		for (auto& el : corners) el += rotatedOtherPos;

		std::array<float, 4> rotations = {
			getAngle(corners[0]),
			getAngle(corners[1]),
			getAngle(corners[2]),
			getAngle(corners[3])
		};

		for (auto& rota : rotations) {	//make over vec2(1,0) negative relative to vec(1,0)
			if (rota > 180.0f) {
				rota = rota - 360.0f;
			}
		}

		float lowestRota = 0.0f;
		float highestRota = 0.0f;
		for (int i = 0; i < 4; i++) {
			if (rotations[i] > lowestRota) {
				lowestShadowEdge = corners[i];
				lowestRota = rotations[i];
			}
			if (rotations[i] < highestRota) {
				highgestShadowEdge = corners[i];
				highestRota = rotations[i];
			}
		}
	}

	vec2 worldSpaceHighest = rotate(highgestShadowEdge, getAngle(translatedOtherPos)) + l.position;
	vec2 worldSpaceLowest = rotate(lowestShadowEdge, getAngle(translatedOtherPos)) + l.position;

	auto distHighest = norm(worldSpaceHighest - l.position);
	auto distLowest = norm(worldSpaceLowest - l.position);

	auto worldSpaceHighestBehind = l.position + normalize(worldSpaceHighest - l.position) * (l.radius);
	auto worldSpaceLowestBehind = l.position + normalize(worldSpaceLowest - l.position) * (l.radius);
	vec2 res[4];
	res[0] = worldSpaceHighest;
	res[1] = worldSpaceHighestBehind;
	res[2] = worldSpaceLowest;
	res[3] = worldSpaceLowestBehind;
	return ShadowVolume(res, l.id);
	return ShadowVolume(res, l.id);
}*/