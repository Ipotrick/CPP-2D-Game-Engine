#pragma once

#include "YAMLSerialize.hpp"
#include "YAMLSerializeCoreComponents.hpp"
#include "YAMLSerializeGameComponents.hpp"

class YAMLEntitySerializer {
public:
    YAMLEntitySerializer(World& manager);
    void serialize(YAML::Emitter& out, EntityHandle entity);
    bool deserialize(YAML::Node entityNode);
private:
    World& world;
};

template<> constexpr bool isYAMLSerializable<World>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, World& v);
namespace YAML {
    template<>
    struct convert<World> {
        static Node encode(const World& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, World& rhs)
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