#pragma once

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
	void save(Archive& ar, const unsigned int version) const
	{
		ar << boost::serialization::base_object<EntityComponentManager>(*this);
		ar << physics;
		ar << texture;
	}

	template<class Archive>
	void load(Archive& ar, const unsigned int version)
	{
		*this = World();
		ar >> boost::serialization::base_object<EntityComponentManager>(*this);
		ar >> physics;
		ar >> texture;
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		boost::serialization::split_member(ar, *this, file_version);
	}
private:
};