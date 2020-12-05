#pragma once

#include "YAMLSerialize.hpp"
#include "YAMLSerializeCoreComponents.hpp"
#include "YAMLSerializeGameComponents.hpp"



class YAMLWorldSerializer {
public:
	YAMLWorldSerializer(World& world)
		:world{world}
	{ }

	void serializeEntity(YAML::Emitter& out, EntityHandle entity);

	bool deserializeEntity(const YAML::Node& entityNode);

	std::string serializeToString();

	bool deserializeString(std::string const& str);
private:
	const std::string ENTITITES_NODE_KEY{ "Entities" };


	World& world;
};