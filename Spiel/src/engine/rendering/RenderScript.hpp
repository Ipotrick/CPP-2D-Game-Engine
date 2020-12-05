#pragma once

#include "RenderTypes.hpp"

class RenderingWorker;
class RenderLayer;

/**
 * Notable information:
 *	- when a script is attached to layer the content of the layer is not
 *	  rendered to the main ftbo any more. If one wants to keep the layer
 *	  ftbo, it must be rendered to the main ftbo with a pass shader manually.
 */
class RenderScript {
public:
	/**
	 * Is called after the script is attached to a layer.
	 * Has full gpraphics api access.
	 * 
	 * \param render is the current rendering worker
	 * \param layer is the layer the script is attached to
	 */
	virtual void onInitialize(RenderingWorker& render, RenderLayer& layer) {}

	/**
	 * Is called every frame once after the coresponding layer is rendererd.
	 * Has full gpraphics api access.
	 * Is no called when the layer is not beeing drawn.
	 * 
	 * \param render is the current rendering worker
	 * \param layer is the layer the script is attached to
	 */
	virtual void onUpdate(RenderingWorker& render, RenderLayer& layer) = 0;

	/**
	 * Is called after script is detached from layer.
	 * Has full gpraphics api access.
	 * 
	 * \param render is the current rendering worker
	 */
	virtual void onDestroy(RenderingWorker& render) {}

	/**
	 * IMPORTANT: This function should have very minimal runtime as it blocks botch main and rendering thread.
	 * Is called once per frame when the rendering worker and main thread are synced.
	 * Should be used to exchange data via pointer swap.
	 * Is only called after the layer is initialized.
	 * No gpraphics api access.
	 */
	virtual void onBuffer() {}
	
	bool isInitialized() const { return bInitialized; }
private:
	friend class RenderingWorker;
	bool bInitialized{ false };
};