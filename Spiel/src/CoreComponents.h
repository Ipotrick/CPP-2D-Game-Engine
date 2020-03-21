 
#include "glmath.h"
#include "Component.h"
#include "Timing.h"
#include "Entity.h"

// solidBody component

struct CompDataSolidBody : public CompData {

	float elasticity;
	float mass;
	float momentOfInertia;
	CompDataSolidBody(float elasticity_, float mass_) : elasticity{ elasticity_ }, mass{ mass_ }, momentOfInertia{0.f} {}
	CompDataSolidBody() : elasticity{ 0.f }, mass{ 0.f }, momentOfInertia{ 0.f } {}
};

// drawable component

struct CompDataDrawable : public CompData {
	vec4 color;
	vec2 scale;
	float drawingPrio;
	Form form;
	bool throwsShadow;

	CompDataDrawable(vec4 color_ = vec4(1, 1, 1, 1), vec2 scale_ = vec2(1, 1), float drawingPrio_ = 0.5f, Form form_ = Form::RECTANGLE, bool throwsShadow_ = false) :
		color{ color_ },
		scale{ scale_ },
		drawingPrio{ drawingPrio_ },
		form{ form_ },
		throwsShadow{ throwsShadow_ }
	{
	}
};

// compositMaster4 component

struct CompDataComposit4 : public CompData {
	struct Slave {
		Slave() : id{ 0 }, relativePos{ 0,0 }, relativeRota{0.f} {}

		Slave(uint32_t id_, vec2 relativePos_, float relativeRota_) : 
			id{id_},
			relativePos{relativePos_},
			relativeRota{ relativeRota_ }
		{}

		uint32_t id;
		vec2 relativePos;
		float relativeRota;
	};

	Slave slaves[4];

	CompDataComposit4() {
		Slave slave(0, vec2(0,0), 0);
		slaves[0] = slave;
		slaves[1] = slave;
		slaves[2] = slave;
		slaves[3] = slave;
	}

	CompDataComposit4(Slave slaves_[]) {
		slaves[0] = slaves_[0];
		slaves[1] = slaves_[1];
		slaves[2] = slaves_[2];
		slaves[3] = slaves_[3];
	}
};