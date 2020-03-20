 
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