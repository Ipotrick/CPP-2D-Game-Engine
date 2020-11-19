#pragma once

#include "RenderTypes.hpp"

class RenderingWorker;
class RenderLayer;

class RenderScript {
public:
	virtual void onInitialize(RenderingWorker& render, RenderLayer& layer) {}
	virtual void onUpdate(RenderingWorker& render, RenderLayer& layer) = 0;
	virtual void onDestroy(RenderingWorker& render) {}

	bool isInitialized() const { return bInitialized; }
private:
	friend class RenderingWorker;
	bool bInitialized{ false };
};