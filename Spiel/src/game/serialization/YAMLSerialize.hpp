#pragma once

#include <yaml-cpp/yaml.h>

#include "../Game.hpp"


/*
* spezialize this template to make your type serializable
*/
template<typename T> constexpr bool isYAMLSerializable() { return false; }

template<> constexpr bool isYAMLSerializable<Vec2>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Vec2& v);
namespace YAML {
    template<>
    struct convert<Vec2> {
        static Node encode(const Vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }

        static bool decode(const Node& node, Vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2) {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Vec3>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Vec3& v);
namespace YAML {
    template<>
    struct convert<Vec3> {
        static Node encode(const Vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, Vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3) {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Vec4>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Vec4& v);
namespace YAML {
    template<>
    struct convert<Vec4> {
        static Node encode(const Vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, Vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4) {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<RotaVec2>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const RotaVec2& v);
namespace YAML {
    template<>
    struct convert<RotaVec2> {
        static Node encode(const RotaVec2& rhs)
        {
            Node node;
            node.push_back(rhs.cos);
            node.push_back(rhs.sin);
            return node;
        }

        static bool decode(const Node& node, RotaVec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2) {
                return false;
            }

            rhs.cos = node[0].as<float>();
            rhs.sin = node[1].as<float>();
            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Form>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Form& v);
namespace YAML {
    template<>
    struct convert<Form> {
        static Node encode(const Form& rhs)
        {
            Node node;
            node = formToString(rhs);
            return node;
        }

        static bool decode(const Node& node, Form& rhs)
        {

            rhs = stringToForm(node.as<std::string>());
            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<LapTimer>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const LapTimer& v);
namespace YAML {
    template<>
    struct convert<LapTimer> {
        static Node encode(const LapTimer& rhs)
        {
            Node node;
            node = rhs.getLapTime();
            return node;
        }

        static bool decode(const Node& node, LapTimer& rhs)
        {
            //if (node.size() != 1) {
            //    return false;
            //}

            rhs.setLapTime(node.as<float>());
            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<UUID>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const UUID& v);
namespace YAML {
    template<>
    struct convert<UUID> {
        static Node encode(const UUID& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, UUID& rhs)
        {
            if (!node.IsSequence() || node.size() != 2) {
                return false;
            }
            rhs.high = node[0].as<uint64_t>();
            rhs.low = node[1].as<uint64_t>();
            return true;
        }
    };
}