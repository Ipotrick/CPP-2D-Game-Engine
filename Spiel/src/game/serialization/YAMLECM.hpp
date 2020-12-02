#pragma once

#include "YAMLSerialize.hpp"
#include "YAMLSerializeCoreComponents.hpp"
#include "YAMLSerializeGameComponents.hpp"

class YAMLEntitySerializer {
public:
    YAMLEntitySerializer(EntityComponentManager& manager);
    void serialize(YAML::Emitter& out, EntityHandle entity);
    bool deserialize(YAML::Node entityNode);
private:
    EntityComponentManager& manager;
};

template<> constexpr bool isYAMLSerializable<EntityComponentManager>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, EntityComponentManager& v);
namespace YAML {
    template<>
    struct convert<EntityComponentManager> {
        static Node encode(const EntityComponentManager& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, EntityComponentManager& rhs)
        {
            if (node.size() != 1) {
                return false;
            }

            auto entitiesNode = node["Entities"];
            if (!entitiesNode) return false;

            YAMLEntitySerializer entityS(rhs);
            for (auto entityNode : entitiesNode) {
                if (!entityS.deserialize(entityNode)) {
                    return false;
                }
            }
            
            return true;
        }
    };
}