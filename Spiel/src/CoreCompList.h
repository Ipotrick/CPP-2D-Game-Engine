#pragma once

#define CoreComps \
ComponentStorage<Base, direct_indexing>,\
ComponentStorage<Movement, direct_indexing>,\
ComponentStorage<Collider, direct_indexing>,\
ComponentStorage<PhysicsBody, direct_indexing>,\
ComponentStorage<LinearEffector, hashing>,\
ComponentStorage<FrictionEffector, hashing>,\
ComponentStorage<Draw, direct_indexing>,\
ComponentStorage<TextureRef, direct_indexing>,\
ComponentStorage<Parent, hashing>,\
ComponentStorage<BaseChild, hashing>