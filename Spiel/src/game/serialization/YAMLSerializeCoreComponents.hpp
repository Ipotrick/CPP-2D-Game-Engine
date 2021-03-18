#pragma once

#include "YAMLSerialize.hpp"

template<> constexpr bool isYAMLSerializable<Transform>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Transform& b);
namespace YAML {
    template<>
    struct convert<Transform> {
        static Node encode(const Transform& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, Transform& rhs)
        {
            if (node.size() != 2) {
                return false;
            }

            rhs.position = node["Position"].as<Vec2>();
            rhs.rotaVec = node["RotaVec"].as<RotaVec2>();
            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Movement>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Movement& b);
namespace YAML {
    template<>
    struct convert<Movement> {
        static Node encode(const Movement& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, Movement& rhs)
        {
            if (node.size() != 2) {
                return false;
            }

            rhs.velocity = node["Velocity"].as<Vec2>();
            rhs.angleVelocity = node["AnlgeVelocity"].as<float>();
            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<CompountCollider>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const CompountCollider& b);
namespace YAML {
    template<> struct convert<CompountCollider> {
        static Node encode(const CompountCollider& rhs)
        {
            Node node;
            node["Size"] = rhs.size;
            node["RelPos"] = rhs.relativePos;
            node["RelRota"] = rhs.relativeRota;
            node["Form"] = rhs.form;
            return node;
        }

        static bool decode(const Node& node, CompountCollider& rhs)
        {
            if (node.size() != 4) {
                return false;
            }

            rhs.size = node["Size"].as<Vec2>();
            rhs.relativePos = node["RelPos"].as<Vec2>();
            rhs.relativeRota = node["RelRota"].as<RotaVec2>();
            rhs.form = node["Form"].as<Form>();
            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Collider>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Collider& b);
namespace YAML {
    template<> struct convert<Collider> {
        static Node encode(const Collider& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, Collider& rhs)
        {
            if (node.size() != 8) {
                return false;
            }
            
            rhs.size                = node["Size"].as<Vec2>();
            rhs.ignoreGroupMask     = node["IgnoreMask"].as<CollisionMask>();
            rhs.groupMask           = node["Mask"].as<CollisionMask>();
            rhs.extraColliders      = node["ExtraCollider"].as<std::vector<CompountCollider>>();
            rhs.particle            = node["particle"].as<bool>();
            rhs.form                = node["Form"].as<Form>();
            rhs.collisionSettings = 0x00;
            rhs.collisionSettings &= node["IgnoreTypes"]["Dynamic"].as<bool>()  *  Collider::DYNAMIC;
            rhs.collisionSettings &= node["IgnoreTypes"]["Static"].as<bool>()   *  Collider::STATIC;
            rhs.collisionSettings &= node["IgnoreTypes"]["Particle"].as<bool>() *  Collider::PARTICLE;
            rhs.collisionSettings &= node["IgnoreTypes"]["Sensor"].as<bool>()   *  Collider::SENSOR;
            rhs.collisionSettings &= node["MyTypes"]["Dynamic"].as<bool>()      * (Collider::DYNAMIC << 4);
            rhs.collisionSettings &= node["MyTypes"]["Static"].as<bool>()       * (Collider::STATIC << 4);
            rhs.collisionSettings &= node["MyTypes"]["Particle"].as<bool>()     * (Collider::PARTICLE << 4);
            rhs.collisionSettings &= node["MyTypes"]["Sensor"].as<bool>()       * (Collider::SENSOR << 4);

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<PhysicsBody>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const PhysicsBody& b);
namespace YAML {
    template<>
    struct convert<PhysicsBody> {
        static Node encode(const PhysicsBody& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, PhysicsBody& rhs)
        {
            if (node.size() != 4) {
                return false;
            }

            rhs.elasticity = node["Elasticity"].as<float>();
            rhs.mass = node["Mass"].as<float>();
            rhs.momentOfInertia = node["MomentInertia"].as<float>();
            rhs.friction = node["Friction"].as<float>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Draw>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Draw& b);
namespace YAML {
    template<>
    struct convert<Draw> {
        static Node encode(const Draw& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, Draw& rhs)
        {
            if (node.size() != 4) {
                return false;
            }

            rhs.color = node["Color"].as<Vec4>();
            rhs.scale = node["Scale"].as<Vec2>();
            rhs.drawingPrio = node["DrawingPrio"].as<float>();
            rhs.form = node["Form"].as<Form>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<LinearEffector>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const LinearEffector& b);
namespace YAML {
    template<>
    struct convert<LinearEffector> {
        static Node encode(const LinearEffector& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, LinearEffector& rhs)
        {
            if (node.size() != 3) {
                return false;
            }

            rhs.direction = node["Direction"].as<Vec2>();
            rhs.force = node["Force"].as<float>();
            rhs.acceleration = node["Acceleration"].as<float>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<FrictionEffector>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const FrictionEffector& b);
namespace YAML {
    template<>
    struct convert<FrictionEffector> {
        static Node encode(const FrictionEffector& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, FrictionEffector& rhs)
        {
            if (node.size() != 2) {
                return false;
            }

            rhs.friction = node["Friction"].as<float>();
            rhs.rotationalFriction = node["RotaFriction"].as<float>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<TextureName>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const TextureName& b);
namespace YAML {
    template<>
    struct convert<TextureName> {
        static Node encode(const TextureName& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, TextureName& rhs)
        {
            if (node.size() != 1) {
                return false;
            }

            rhs.name = node["name"].as<std::string>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<TextureLoadInfo>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const TextureLoadInfo& b);
namespace YAML {
    template<>
    struct convert<TextureLoadInfo> {
        static Node encode(const TextureLoadInfo& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, TextureLoadInfo& rhs)
        {
            if (node.size() != 4) {
                return false;
            }

            rhs.filepath = node["filepath"].as<std::string>();
            rhs.minFilter = cast<TexFilter>(node["minFilter"].as<u32>());
            rhs.magFilter = cast<TexFilter>(node["magFilter"].as<u32>());
            rhs.clampMethod = cast<TexClamp>(node["clampMethod"].as<u32>());

            return true;
        }
    };
}