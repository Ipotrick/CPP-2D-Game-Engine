#include "CompController.h"

// mortal component index: 0

void CompControllerMortal::executeScript(CompDataMortal& data, float deltaTime)
{
	if (data.maxHealth > 0) {
		auto [begin, end] = engine.getCollisionInfos(data.id);
		for (auto iter = begin; iter != end; ++iter) {
			if (isRegistered(iter->idB)) {
				auto otherData = getComponent(iter->idB);
				data.curHealth = data.curHealth - otherData->collisionDamage;
			}
		}
	}

	data.curAge += deltaTime;

	if ((data.curHealth <= 0 && data.maxHealth > 0) || (data.curAge > data.maxAge && data.maxAge > 0)) {
		engine.world.despawn(data.id);
	}
}

//player component index: 1

void CompControllerPlayer::executeScript(CompDataPlayer& data, float deltaTime)
{
	//querry entity
	Entity* const entity = engine.world.getEntityPtr(data.id);

	float exaustVel = 1.0f;
	vec2 exaustSize = vec2(0.06f, 0.06f);
	float exaustamount = 1;
	CompControllerMortal* mortalComponent = reinterpret_cast<CompControllerMortal*>(otherComponents.at(0));
	CompControllerBullet* bulletComponent = reinterpret_cast<CompControllerBullet*>(otherComponents.at(2));
	if (engine.keyPressed(KEY::W)) {
		entity->acceleration += rotate(vec2(0.0f, 2.0f), entity->rotation);
		for (int i = 0; i < exaustamount; i++) {
			engine.world.spawnEntity(Entity(Drawable(entity->getPos() + rotate(vec2(-entity->scale.y, 0) / 3.0f, entity->rotation + 90), entity->drawingPrio - 0.01f, exaustSize, vec4(1, 0, 0, 0.8f), Drawable::Form::CIRCLE, entity->rotation),
				Collidable(exaustSize, Collidable::Form::CIRCLE, 0.4f, true, 0.001f, entity->velocity + exaustVel * rotate(vec2(0, 1), entity->rotation))));
			mortalComponent->registerEntity(CompDataMortal(engine.world.latestID, -1, 0, 0.3f));
		}
	}
	if (engine.keyPressed(KEY::A)) {
		entity->acceleration += rotate(vec2(-10.0f, 0.0f), entity->rotation);
	}
	if (engine.keyPressed(KEY::S)) {
		entity->acceleration += rotate(vec2(0.0f, -10.0f), entity->rotation);
	}
	if (engine.keyPressed(KEY::D)) {
		entity->acceleration += rotate(vec2(10.0f, 0.0f), entity->rotation);
	}
	if (engine.keyPressed(KEY::Q)) {
		entity->rotation += 200.0f * deltaTime;
	}
	if (engine.keyPressed(KEY::E)) {
		entity->rotation -= 200.0f * deltaTime;
	}
	if (engine.keyPressed(KEY::F)) {
		engine.world.spawnEntity(Entity(Drawable(entity->getPos() + rotate(vec2(-entity->scale.y, 0) / 3.0f, entity->rotation + 90), entity->drawingPrio - 0.01f, exaustSize, vec4(0, 0, 1, 0.8f), Drawable::Form::CIRCLE, entity->rotation),
			Collidable(exaustSize, Collidable::Form::CIRCLE, 0.4f, true, 0.001f, (0, 3) * rotate(vec2(0, -1), entity->rotation))));
		bulletComponent->registerEntity(CompDataBullet(engine.world.latestID, 100, 20, 100, 2.0f));
	}
}

//bullet component index: 2

void CompControllerBullet::executeScript(CompDataBullet& data, float deltaTime) 
{
	auto [begin, end] = engine.getCollisionInfos(data.id);
	if (begin != end){
		engine.world.despawn(data.id);
	}
}