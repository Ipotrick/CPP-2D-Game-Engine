#pragma once
#define CORE_COMPONENT_SEGMENT \
ComponentStorage<Base, direct_indexing>, \
ComponentStorage<Draw, direct_indexing>, \
ComponentStorage<Collider, sparse_indexing>, \
ComponentStorage<CollisionsToken, sparse_indexing>, \
ComponentStorage<Movement, sparse_indexing>, \
ComponentStorage<PhysicsBody, sparse_indexing>, \
ComponentStorage<TextureRef, sparse_set>, \
ComponentStorage<LinearEffector, sparse_set>, \
ComponentStorage<FrictionEffector, sparse_set>, \
ComponentStorage<Parent, sparse_set>, \
ComponentStorage<BaseChild, sparse_set>

#define COMMENT1 \
ComponentStorage<Base, direct_indexing>, \
ComponentStorage<Draw, direct_indexing>, \
ComponentStorage<Collider, sparse_indexing>, \
ComponentStorage<Movement, sparse_indexing>, \
ComponentStorage<PhysicsBody, sparse_indexing>, \
ComponentStorage<TextureRef, sparse_set>, \
ComponentStorage<LinearEffector, sparse_set>, \
ComponentStorage<FrictionEffector, sparse_set>, \
ComponentStorage<Parent, sparse_set>, \
ComponentStorage<BaseChild, sparse_set>