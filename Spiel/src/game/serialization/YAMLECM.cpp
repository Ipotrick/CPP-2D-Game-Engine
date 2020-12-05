#include "YAMLECM.hpp"

YAMLEntitySerializer::YAMLEntitySerializer(World& manager)
	: world{ manager }
{ }

void YAMLEntitySerializer::serialize(YAML::Emitter& out, EntityHandle entity)
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

bool YAMLEntitySerializer::deserialize(YAML::Node entityNode)
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

YAML::Emitter& operator<<(YAML::Emitter& out, World& v)
{
	out << YAML::BeginMap;
	YAMLEntitySerializer es(v);
	for (auto entity : v.entityView<Transform>()) {
		es.serialize(out, entity);
	}
	out << YAML::EndMap;
	return out;
}
