#include "BaseTypes.h"
#include "glmath.h"
#include "ECS.h"
#include "Timing.h"

// basis component

struct Base : public CompData {
	vec2 position;
	float rotation;
	Base(vec2 pos_ = { 0,0 }, float rota_ = 0.0f) :
		position{ pos_ },
		rotation{ rota_ } {}
};

// movement component

struct Movement : public CompData {
	vec2 velocity;
	float angleVelocity;
	Movement(vec2 vel_ = { 0,0 }, float anglVel_ = 0.0f) :
		velocity{ vel_ },
		angleVelocity{ anglVel_ } {}
};

// collider component

struct Collider : public CompData {
	vec2 size;
	Form form;
	bool particle;
	Collider(vec2 size_ = { 1,1 }, Form form_ = Form::CIRCLE, bool particle_ = false) :
		size{ size_ }, 
		form{ form_ },
		particle{ particle_ } {}
};

// solidBody component

struct SolidBody : public CompData {
	float elasticity;
	float mass;
	float momentOfInertia;
	SolidBody(float elasticity_, float mass_, float mOI_) : elasticity{ elasticity_ }, mass{ mass_ }, momentOfInertia{ mOI_ } {}
	SolidBody() : 
		elasticity{ 0.f }, 
		mass{ 0.f }, 
		momentOfInertia{ 0.f } {
		
	}
};

// draw component

struct Draw : public CompData {
	vec4 color;
	vec2 scale;
	float drawingPrio;
	Form form;
	bool throwsShadow;

	Draw(vec4 color_ = vec4(1, 1, 1, 1), vec2 scale_ = vec2(1, 1), float drawingPrio_ = 0.5f, Form form_ = Form::RECTANGLE, bool throwsShadow_ = false) :
		color{ color_ },
		scale{ scale_ },
		drawingPrio{ drawingPrio_ },
		form{ form_ },
		throwsShadow{ throwsShadow_ }
	{
	}
};

// slave component

struct Slave {
	uint32_t ownerID;
	Slave(uint32_t id_ = 0) :
		ownerID{ id_ } {}

};

// composit component

template<int N>
struct Composit : public CompData {
	struct Slave {
		Slave() : id{ 0 }, relativePos{ 0,0 }, relativeRota{ 0.f } {}

		Slave(uint32_t id_, vec2 relativePos_, float relativeRota_) :
			id{ id_ },
			relativePos{ relativePos_ },
			relativeRota{ relativeRota_ }
		{}

		uint32_t id;
		vec2 relativePos;
		float relativeRota;
	};

	Slave slaves[N];

	Composit() {
		for (int i = 0; i < N; ++i) {
			slaves[i] = { 0, vec2(0,0), 0 };
		}
	}

	Composit(Slave slaves_[]) {
		for (int i = 0; i < N; ++i) {
			slaves[i] = slaves_[i];
		}
	}
};

// moveField component

struct MoveField : public CompData {
	MoveField(vec2 const& mvdr = {1,0}, float frc = 0.0f, float accel = 0.0f) :
		movdir{ mvdr },
		force{ frc },
		acceleration{ accel }
	{ }

	vec2 movdir;
	float force;
	float acceleration;
};