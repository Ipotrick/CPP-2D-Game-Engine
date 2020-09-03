#pragma once
#define CORE_COMPONENT_SEGMENT \
ComponentStorage<Base, direct_indexing>, \
ComponentStorage<Draw, direct_indexing>, \
ComponentStorage<Collider, paged_indexing>, \
ComponentStorage<CollisionsToken, paged_indexing>, \
ComponentStorage<Movement, paged_indexing>, \
ComponentStorage<PhysicsBody, paged_indexing>, \
ComponentStorage<TextureRef, paged_set>, \
ComponentStorage<LinearEffector, paged_set>, \
ComponentStorage<FrictionEffector, paged_set>

#define COMMENT1