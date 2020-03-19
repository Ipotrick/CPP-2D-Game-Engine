#include "Component.h"

// solidBody component

struct CompDataSolidBody : public CompData {

	float elasticity;
	float mass;
	CompDataSolidBody(float elasticity_, float mass_) : elasticity{ elasticity_ }, mass{ mass_ } {}
	CompDataSolidBody() : elasticity{0.f}, mass{0.f} {}
};