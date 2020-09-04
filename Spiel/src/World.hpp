#pragma once

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "EntityComponentManager.hpp"

#include "CoreSystemUniforms.hpp"
#include "TextureUniforms.hpp"

class World : public EntityComponentManager {
public:
	using EntityComponentManager::EntityComponentManager;

	void loadMap(std::string);
	void saveMap(std::string);
public:
	// Core System Uniform Data
	PhysicsUniforms physics;
	TextureUniforms texture;
protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar & boost::serialization::base_object<EntityComponentManager>(*this);
		ar & physics;
		ar & texture;
	}
private:
};