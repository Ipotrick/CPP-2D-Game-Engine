#include "YAMLSerialize.hpp"
YAML::Emitter& operator<<(YAML::Emitter& out, const Vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec4& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const RotaVec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.cos << v.sin << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Form& f)
{
	switch (f) {
	case Form::Circle:
		out << "Circle";
		break;
	case Form::Rectangle:
		out << "Rectangle";
		break;
	}
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const LapTimer& v)
{
	out << YAML::Value << v.getLapTime();
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const UUID& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.high << v.low << YAML::EndSeq;
	return out;
}
