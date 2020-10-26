#include "BaseTypes.hpp"
#include "Vec.hpp"
#include "EntityComponentStorage.hpp"
#include "Timing.hpp"

// basis component

struct Transform : public CompData {
	Transform(Vec2 pos = Vec2{ 0,0 }) :
		position{ pos },
		rotation{ 0 },
		rotaVec{ 1,0 }
	{}

	Transform(Vec2 pos, float rota) :
		position{ pos },
		rotation{ rota },
		rotaVec{ sinf(rotation / RAD),
			cosf(rotation / RAD) }
	{}

	Transform(Vec2 pos, RotaVec2 rota) :
		position{ pos },
		rotation{ 0 },
		rotaVec{ rota }
	{}

	Vec2 position;
	RotaVec2 rotaVec;
	float rotation;
};

// movement component

struct Movement : public CompData {
	Movement(Vec2 vel = { 0,0 }, float anglVel = 0.0f) :
		velocity{ vel },
		angleVelocity{ anglVel } 
	{}

	Vec2 velocity;
	float angleVelocity;
};


using CollisionMask = uint16_t;

// collider component
template<int Group>
struct CollisionGroup {
	static_assert(Group >= 0 && Group < 16);
	static const CollisionMask mask = 1 << Group;
};

struct CompountCollider {
	CompountCollider() = default;

	CompountCollider(Vec2 size,
		Vec2 relativePos,
		RotaVec2 relativeRota,
		Form form)
		:size{size}, relativePos{ relativePos }, relativeRota{ relativeRota }, form{ form }
	{}

	Vec2 size{0,0};
	Vec2 relativePos{0,0};
	RotaVec2 relativeRota{0,0};
	Form form{Form::Rectangle};
};

struct Collider : public CompData {
	Collider(Vec2 size = { 1,1 }, Form form = Form::Circle, bool particle = false) :
		size{ size },
		form{ form },
		particle{ particle },
		sleeping{ false }
	{}

	inline void setIgnore(uint8_t mask)
	{
		collisionSettings |= mask;
	}

	inline void unsetIgnore(uint8_t mask)
	{
		collisionSettings &= ~mask;
	}

	inline void setIgnoredBy(uint8_t mask)
	{
		collisionSettings |= mask << 4;
	}

	inline void unsetIgnoredBy(uint8_t mask)
	{
		collisionSettings &= ~(mask << 4);
	}

	inline bool isIgnoring(uint8_t mask)
	{
		return (collisionSettings & mask) != false;
	}

	inline bool isIgnoredBy(uint8_t mask)
	{
		return (collisionSettings & (mask << 4)) != false;
	}

	static const uint8_t DYNAMIC = 1 << 0;
	static const uint8_t STATIC = 1 << 1;
	static const uint8_t PARTICLE = 1 << 2;
	static const uint8_t SENSOR = 1 << 3;

	Vec2 size;
	CollisionMask ignoreGroupMask = 0;
	CollisionMask groupMask = CollisionGroup<0>::mask;
	std::vector<CompountCollider> extraColliders;
	bool particle;
	bool sleeping;
	Form form;
	// lower 4 bits: who am i ignoring?
	// upper 4 bits: who am i ignored by/hiding from
	uint8_t collisionSettings = 0;
};

struct CollisionsToken {
private:
	friend class CollisionSystem;
	uint32_t begin{ 0 };
	uint32_t end{ 0 };
};

// solidBody component

struct PhysicsBody : public CompData {
	PhysicsBody(float elasticity, float mass, float mOI, float friction) :
		elasticity{ elasticity },
		mass{ mass },
		momentOfInertia{ mOI },
		friction{ friction }
	{}

	PhysicsBody() :
		elasticity{ 0.f },
		mass{ 0.f },
		momentOfInertia{ 0.f },
		friction{ 0.f }
	{}

	float elasticity;
	float mass;
	float momentOfInertia;
	float friction;
};

// draw component

struct Draw : public CompData {
	Draw(Vec4 color = Vec4(1, 1, 1, 1), Vec2 scale = Vec2(1, 1), float drawingPrio = 0.5f, Form form = Form::Rectangle) :
		color{ color },
		scale{ scale },
		drawingPrio{ drawingPrio },
		form{ form }
	{}
	 
	Vec4 color;
	Vec2 scale;
	float drawingPrio;
	Form form;
};

// effector components

struct LinearEffector : public CompData {
	LinearEffector(Vec2 const& mvdr = { 1,0 }, float frc = 0.0f, float accel = 0.0f) :
		direction{ mvdr },
		force{ frc },
		acceleration{ accel }
	{}

	Vec2 direction;
	float force;
	float acceleration;
};
struct FrictionEffector : public CompData {
	FrictionEffector(float frctn = 0, float rotaFrctn = 0) :
		friction{ frctn },
		rotationalFriction{ rotaFrctn }
	{}

	float friction;
	float rotationalFriction;
};