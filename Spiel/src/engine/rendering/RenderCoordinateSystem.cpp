#include "RenderCoordinateSystem.hpp"

RenderCoordSys::RenderCoordSys(const Window& window, const Camera& camera) :
	windowWidth{ window.getSizeVec().x },
	windowHeight{ window.getSizeVec().y },
	camera{camera}
{ }

Vec2 RenderCoordSys::convertCoordSys(Vec2 coord, RenderSpace from, RenderSpace to) const
{
	switch (from) {
	case RenderSpace::Pixel:
		switch (to) {
		case RenderSpace::Pixel:
			return coord;
			break;
		case RenderSpace::UniformWindow:
			return convertCoordSys<RenderSpace::Pixel, RenderSpace::UniformWindow>(coord);
			break;
		case RenderSpace::Window:
			return convertCoordSys<RenderSpace::Pixel, RenderSpace::Window>(coord);
			break;
		case RenderSpace::Camera:
			return convertCoordSys<RenderSpace::Pixel, RenderSpace::Camera>(coord);
			break;
		}
		break;
	case RenderSpace::UniformWindow:
		switch (to) {
		case RenderSpace::Pixel:
			return convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Pixel>(coord);
			break;
		case RenderSpace::UniformWindow:
			return coord;
			break;
		case RenderSpace::Window:
			return convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Window>(coord);
			break;
		case RenderSpace::Camera:
			return convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Camera>(coord);
			break;
		}
		break;
	case RenderSpace::Window:
		switch (to) {
		case RenderSpace::Pixel:
			return convertCoordSys<RenderSpace::Window, RenderSpace::Pixel>(coord);
			break;
		case RenderSpace::UniformWindow:
			return convertCoordSys<RenderSpace::Window, RenderSpace::UniformWindow>(coord);
			break;
		case RenderSpace::Window:
			return coord;
			break;
		case RenderSpace::Camera:
			return convertCoordSys<RenderSpace::Window, RenderSpace::Camera>(coord);
			break;
		}
		break;
	case RenderSpace::Camera:
		switch (to) {
		case RenderSpace::Pixel:
			return convertCoordSys<RenderSpace::Camera, RenderSpace::Pixel>(coord);
			break;
		case RenderSpace::UniformWindow:
			return convertCoordSys<RenderSpace::Camera, RenderSpace::UniformWindow>(coord);
			break;
		case RenderSpace::Window:
			return convertCoordSys<RenderSpace::Camera, RenderSpace::Window>(coord);
			break;
		case RenderSpace::Camera:
			return coord;
			break;
		}
		break;
	}
	return { 0, 0 };	// makes the compiler happy
}


template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Pixel, RenderSpace::Window>(Vec2 coord) const
{
	return {
		coord.x / windowWidth * 2.0f - 1.0f,
		coord.y / windowHeight * 2.0f - 1.0f
	};
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Camera, RenderSpace::Window>(Vec2 coord) const
{
	const f32 uniformXBend = cast<f32>(windowHeight) / cast<f32>(windowWidth);
	auto res = (rotate(coord - camera.position, -camera.rotation) * camera.frustumBend * camera.zoom);
	res.x *= uniformXBend;
	return res;
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Window>(Vec2 coord) const
{
	const float xScale = (float)windowWidth / (float)windowHeight;
	coord.x /= xScale;
	return coord;
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Window, RenderSpace::Pixel>(Vec2 coord) const
{
	return {
		(coord.x + 1.0f) / 2.0f * windowWidth,
		(coord.y + 1.0f) / 2.0f * windowHeight
	};
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Window, RenderSpace::Camera>(Vec2 coord) const
{
	const f32 uniformXBend = cast<f32>(windowHeight) / cast<f32>(windowWidth);
	coord.x /= uniformXBend;
	return rotate(coord / camera.frustumBend / camera.zoom, camera.rotation) + camera.position;
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Window, RenderSpace::UniformWindow>(Vec2 coord) const
{
	const float xScale = (float)windowWidth / (float)windowHeight;
	coord.x *= xScale;
	return coord;
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Camera, RenderSpace::Pixel>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::Window, RenderSpace::Pixel>(
		convertCoordSys<RenderSpace::Camera, RenderSpace::Window>(coord));
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Pixel, RenderSpace::Camera>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::Window, RenderSpace::Camera>(
		convertCoordSys<RenderSpace::Pixel, RenderSpace::Window>(coord));
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Pixel>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::Window, RenderSpace::Pixel>(
		convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Window>(coord));
}
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Pixel, RenderSpace::UniformWindow>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::Window, RenderSpace::UniformWindow>(
		convertCoordSys<RenderSpace::Pixel, RenderSpace::Window>(coord));
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Camera>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::Window, RenderSpace::Camera>(
		convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Window>(coord));
}

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Camera, RenderSpace::UniformWindow>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::Window, RenderSpace::UniformWindow>(
		convertCoordSys<RenderSpace::Camera, RenderSpace::Window>(coord));
}
