#include "RenderCoordinateSystem.hpp"

RenderCoordSys::RenderCoordSys(const Window& window, const Camera& camera) :
	windowWidth{ window.getSizeVec().x },
	windowHeight{ window.getSizeVec().y },
	camera{camera}
{ }

Vec2 RenderCoordSys::convertCoordSys(Vec2 coord, RenderSpace from, RenderSpace to) const
{
	Vec2 corrdInWindowSpace;
	switch (from) {
	case RenderSpace::Pixel:
		corrdInWindowSpace = convertCoordSys<RenderSpace::Pixel, RenderSpace::Window>(coord);
		break;
	case RenderSpace::UniformWindow:
		corrdInWindowSpace = convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Window>(coord);
		break;
	case RenderSpace::Window:
		corrdInWindowSpace = coord;
		break;
	case RenderSpace::Camera:
		corrdInWindowSpace = convertCoordSys<RenderSpace::Camera, RenderSpace::Window>(coord);
		break;
	}
	Vec2 retVal;
	switch (to) {
	case RenderSpace::Pixel:
		retVal = convertCoordSys<RenderSpace::Window, RenderSpace::Pixel>(corrdInWindowSpace);
		break;
	case RenderSpace::UniformWindow:
		retVal = convertCoordSys<RenderSpace::Window, RenderSpace::UniformWindow>(corrdInWindowSpace);
		break;
	case RenderSpace::Window:
		retVal = corrdInWindowSpace;
		break;
	case RenderSpace::Camera:
		retVal = convertCoordSys<RenderSpace::Window, RenderSpace::Camera>(corrdInWindowSpace);
		break;
	}
	return retVal;
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
