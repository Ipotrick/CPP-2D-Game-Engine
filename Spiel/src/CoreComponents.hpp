#include "BaseTypes.hpp"
#include "Vec.hpp"
#include "EntityComponentStorage.hpp"
#include "Timing.hpp"

// basis component

struct Base : public CompData {
	Vec2 position;
	float rotation;
	RotaVec2 rotaVec;
	Base(Vec2 pos_ = Vec2{0,0}) :
		position{ pos_ },
		rotation{0},
		rotaVec{1,0}
	{}
	Base(Vec2 pos_, float rota_) :
		position{ pos_ },
		rotation{ rota_ },
	rotaVec{ sinf(rotation / RAD),
		cosf(rotation / RAD) }
	{}
	Base(Vec2 pos_, RotaVec2 rota_) :
		position{ pos_ },
		rotation{ 0 },
		rotaVec{ rota_ }
	{}
};

// movement component

struct Movement : public CompData {
	Vec2 velocity;
	float angleVelocity;
	Movement(Vec2 vel_ = { 0,0 }, float anglVel_ = 0.0f) :
		velocity{ vel_ },
		angleVelocity{ anglVel_ } {}
};


#define COLLISION_GROUP_TYPE uint8_t

// collider component
template<int Group>
struct CollisionGroup {
	static const COLLISION_GROUP_TYPE mask = 1 << Group;
};

struct Collider : public CompData {
	Vec2 size;
	COLLISION_GROUP_TYPE collisionMaskAgainst	= CollisionGroup<0>::mask;
	COLLISION_GROUP_TYPE collisionMaskSelf		= CollisionGroup<0>::mask;
	bool particle;
	bool sleeping;
	Form form;
	Collider(Vec2 size_ = { 1,1 }, Form form_ = Form::Circle, bool particle_ = false) :
		size{ size_ }, 
		form{ form_ },
		particle{ particle_ },
		sleeping{ false }
	{}
};

// solidBody component

struct PhysicsBody : public CompData {
	float elasticity;
	float mass;
	float momentOfInertia;
	float friction;
	float overlapAccum = 0;
	PhysicsBody(float elasticity_, float mass_, float mOI_, float friction) : 
		elasticity{ elasticity_ },
		mass{ mass_ }, 
		momentOfInertia{ mOI_ },
		friction{ friction }
	{}
	PhysicsBody() : 
		elasticity{ 0.f }, 
		mass{ 0.f }, 
		momentOfInertia{ 0.f },
		friction{ 0.f }
	{ }
};

// draw component

struct Draw : public CompData {
	Vec4 color;
	Vec2 scale;
	float drawingPrio;
	Form form;
	bool throwsShadow;

	Draw(Vec4 color_ = Vec4(1, 1, 1, 1), Vec2 scale_ = Vec2(1, 1), float drawingPrio_ = 0.5f, Form form_ = Form::Rectangle, bool throwsShadow_ = false) :
		color{ color_ },
		scale{ scale_ },
		drawingPrio{ drawingPrio_ },
		form{ form_ },
		throwsShadow{ throwsShadow_ }
	{
	}
};

// Parent component

struct Parent {
	Parent() {}
	std::vector<entity_id> children;
};

// BaseSlave component

struct BaseChild {
	BaseChild() : parent{ entity_id(0) }, relativePos{ 0, 0 }, relativeRota{ 0 } {}
	BaseChild(entity_id parent, Vec2 relativePos, float relativeRota) : parent{ parent }, relativePos{ relativePos }, relativeRota { relativeRota } {}
	entity_id parent;
	Vec2 relativePos;
	float relativeRota;
};

// effector components

struct LinearEffector : public CompData {
	Vec2 movdir;
	float force;
	float acceleration;
	LinearEffector(Vec2 const& mvdr = {1,0}, float frc = 0.0f, float accel = 0.0f) :
		movdir{ mvdr },
		force{ frc },
		acceleration{ accel }
	{ }
};
struct FrictionEffector : public CompData {
	float friction;
	float rotationalFriction;
	FrictionEffector(float frctn = 0, float rotaFrctn = 0) :
		friction{ frctn },
		rotationalFriction{ rotaFrctn }
	{}
};