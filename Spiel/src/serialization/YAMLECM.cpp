#include "YAMLECM.hpp"

YAMLEntitySerializer::YAMLEntitySerializer(EntityComponentManager& manager)
	: manager{ manager }
{ }

void YAMLEntitySerializer::serialize(YAML::Emitter& out, EntityHandle entity)
{
	out << YAML::BeginMap;
	UUID id;
	if (manager.hasId(entity)) {
		id = manager.identify(entity);
	}
	out << YAML::Key << "Entity" << YAML::Value << id;
	
	util::tuple_for_each(getNakedStorage(manager),
		[&](auto& componentStorage) {
			using ComponentType = std::remove_reference<decltype(componentStorage.get(0))>::type;
			if constexpr (isYAMLSerializable<ComponentType>()) {
				if (manager.hasComp<ComponentType>(entity)) {
					out << YAML::Key << typeid(ComponentType).name();
					out << manager.getComp<ComponentType>(entity);
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
	
	auto entity = manager.create(uuidNode.as<UUID>());
	util::tuple_for_each(getNakedStorage(manager),
		[&](auto& componentStorage) {
			using ComponentType = std::remove_reference<decltype(componentStorage.get(0))>::type;
			const std::string COMPONENT_NAME = typeid(ComponentType).name();
			if constexpr (isYAMLSerializable<ComponentType>()) {
				auto componentNode = entityNode[COMPONENT_NAME];
				if (componentNode) {
					manager.addComp<ComponentType>(entity, componentNode.as<ComponentType>());
				}
			}
		}
	);
	manager.spawn(entity);
	return true;
}

YAML::Emitter& operator<<(YAML::Emitter& out, EntityComponentManager& v)
{
	out << YAML::BeginMap;
	YAMLEntitySerializer es(v);
	for (auto entity : v.entityView<Transform>()) {
		es.serialize(out, entity);
	}
	out << YAML::EndMap;
	return out;
}
