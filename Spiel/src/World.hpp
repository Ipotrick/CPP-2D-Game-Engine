#pragma once

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "EntityComponentManager.hpp"

#include "CoreSystemUniforms.hpp"

class World : public EntityComponentManager {
public:
	using EntityComponentManager::EntityComponentManager;

	virtual void loadMap(const std::string& filename);
	virtual void saveMap(const std::string& filename);
	virtual void saveWorld(const std::string& filename);
	virtual void loadWorld(const std::string& filename);
	virtual void saveGameState(const std::string& filename);
	virtual void loadGameState(const std::string& filename);

	// Core System Uniform Data
	PhysicsUniforms physics;
protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar & boost::serialization::base_object<EntityComponentManager>(*this);
		ar & physics;
	}
private:
};