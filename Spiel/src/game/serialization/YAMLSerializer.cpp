#include "YAMLSerializer.hpp"

void YAMLWorldSerializer::serializeEntity(YAML::Emitter& out, EntityHandle entity)
{
	out << YAML::BeginMap;
	UUID id;
	if (world.hasId(entity)) {
		id = world.identify(entity);
	}
	out << YAML::Key << "Entity" << YAML::Value << id;

	util::tuple_for_each(world.componentStorageTuple, 
		[&](auto& componentStorage) {
			using ComponentType = typename std::remove_reference<decltype(componentStorage.get(0))>::type;
			if constexpr (isYAMLSerializable<ComponentType>()) {
				if (world.hasComp<ComponentType>(entity)) {
					out << YAML::Key << typeid(ComponentType).name();
					out << world.getComp<ComponentType>(entity);
				}
			}
		}
	);

	out << YAML::EndMap;
}

bool YAMLWorldSerializer::deserializeEntity(const YAML::Node& entityNode)
{
	auto uuidNode = entityNode["Entity"];
	if (!uuidNode) return false;

	auto entity = world.create(uuidNode.as<UUID>());
	util::tuple_for_each(world.componentStorageTuple,
		[&](auto& componentStorage) {
			using ComponentType = typename std::remove_reference<decltype(componentStorage.get(0))>::type;
			const std::string COMPONENT_NAME = typeid(ComponentType).name();
			if constexpr (isYAMLSerializable<ComponentType>()) {
				auto componentNode = entityNode[COMPONENT_NAME];
				if (componentNode) {
					world.addComp<ComponentType>(entity, componentNode.as<ComponentType>());
				}
			}
		}
	);
	world.spawn(entity);
	return true;
}

std::string YAMLWorldSerializer::serializeToString()
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "World" << YAML::Value << "";
	out << YAML::Key << "PhysicsUniforms" << YAML::Value << world.physics;

	out << YAML::Key << ENTITITES_NODE_KEY << YAML::Value << YAML::BeginSeq;
	for (EntityHandle entity : world.entityView<Transform>()) {
		serializeEntity(out, entity);
	}
	out << YAML::EndSeq;

	out << YAML::EndMap;
	return std::string(out.c_str());
}

bool YAMLWorldSerializer::deserializeString(std::string const& str)
{
	std::ofstream ofs("read.yaml");
	ofs << str;
	ofs.close();
	YAML::Node data = YAML::Load(str);
	if (!data["World"]) return false;
	world.physics = data["PhysicsUniforms"].as<PhysicsUniforms>();

	auto entities = data[ENTITITES_NODE_KEY];
	if (entities) {
		for (auto entity : entities) {
			if (!deserializeEntity(entity)) return false;
		}
	}

	return true;
}
