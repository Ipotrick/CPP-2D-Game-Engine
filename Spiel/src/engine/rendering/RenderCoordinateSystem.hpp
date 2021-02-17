#pragma once

#include "RenderSpace.hpp"
#include "Window.hpp"
#include "Camera.hpp"

class RenderCoordSys {
public:
	RenderCoordSys(const Window& window, const Camera& camera);
	RenderCoordSys() = default;
	/**
	 * compile time convertion of coordinate system of vector.
	 *
	 * \param From RenderSpace the coord is in
	 * \param To RenderSpace the corrd should be converted to
	 * \param vec vector to convert
	 * \return converted vector
	 */
	template<RenderSpace From, RenderSpace To>
	Vec2 convertCoordSys(Vec2 vec) const;

	/**
	 * run time convertion of coordinate system of vector.
	 *
	 * \param vec vector to convert
	 * \param from RenderSpace the coord is in
	 * \param to RenderSpace the corrd should be converted to
	 * \return converted vector
	 */
	Vec2 convertCoordSys(Vec2 vec, RenderSpace from, RenderSpace to) const;
private:
	f32 windowWidth{ 1.0f };
	f32 windowHeight{ 1.0f };
	Camera camera;
};

template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Pixel, RenderSpace::Window>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Camera, RenderSpace::Window>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Window>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Window, RenderSpace::Pixel>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Window, RenderSpace::Camera>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Window, RenderSpace::UniformWindow>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Camera, RenderSpace::Pixel>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Pixel, RenderSpace::Camera>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Pixel>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Pixel, RenderSpace::UniformWindow>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Camera>(Vec2 coord) const;
template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace::Camera, RenderSpace::UniformWindow>(Vec2 coord) const;
