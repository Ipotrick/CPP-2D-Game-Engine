#pragma once
#define CORE_COMPONENT_SEGMENT \
ComponentStorage<Base, direct_indexing>, \
ComponentStorage<Draw, direct_indexing>, \
ComponentStorage<Collider, sparse_set>, \
ComponentStorage<Movement, sparse_set>, \
ComponentStorage<PhysicsBody, sparse_set>, \
ComponentStorage<TextureRef, sparse_set>, \
ComponentStorage<LinearEffector, sparse_set>, \
ComponentStorage<FrictionEffector, sparse_set>, \
ComponentStorage<Parent, sparse_set>, \
ComponentStorage<BaseChild, sparse_set>

#define COMMENT1 \
ComponentStorage<Base, direct_indexing>, \
ComponentStorage<Draw, direct_indexing>, \
ComponentStorage<Collider, lookup_table>, \
ComponentStorage<Movement, lookup_table>, \
ComponentStorage<PhysicsBody, sorted_lookup_table>, \
ComponentStorage<TextureRef, sorted_lookup_table>, \
ComponentStorage<LinearEffector, hashing>, \
ComponentStorage<FrictionEffector, hashing>, \
ComponentStorage<Parent, hashing>, \
ComponentStorage<BaseChild, hashing>