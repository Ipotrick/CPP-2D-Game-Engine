#pragma once

#include "RenderPipeline.hpp"
#include "RenderPipelineThread.hpp"


class TestPipe : public IRenderPipe {
public:
	class TestPipeBackend : public IRenderPipeBackend {
	public:
		virtual void init() override
		{
			std::cout << "TestPipeBackend: Init\n";
		}
		virtual void reset() override
		{
			std::cout << "TestPipeBackend: Reset\n";
		}
		virtual void render(RenderPipeContext& context) override
		{
		}

		u32 intBack{ 0 };
	};

	virtual void flush() override
	{
		backend.intBack = intFront;
	}
	virtual IRenderPipeBackend* getBackend()  override
	{
		return &backend;
	}

	void setInt(u32 i) { this->intFront = i; }

private:
	u32 intFront{ 0 };
	TestPipeBackend backend;
};

class DefaultRenderer {
public:
	~DefaultRenderer()
	{
		if (bInitialized) {
			reset();
		}
	}
	void init(Window* window) 
	{
		assert(!bInitialized);
		pipeline.window = window;
		pipeline.context = &pipeContext;
		pipeline.pipes.push_back(pipe.getBackend());
		bInitialized = true;

		worker.execute(&pipeline, RenderPipelineThread::Action::Init);
	}
	void reset()
	{ 
		assert(bInitialized);

		worker.execute(&pipeline, RenderPipelineThread::Action::Reset);
		bInitialized = false;
	}
	void start()
	{
		assert(bInitialized);

		worker.wait();

		flush();

		worker.execute(&pipeline, RenderPipelineThread::Action::Exec);
	}

	TestPipe pipe;
private:
	void flush()
	{
		pipe.flush();
	}

	bool bInitialized{ false }; 
	RenderPipelineThread worker;
	RenderPipeContext pipeContext;
	RenderPipeline pipeline;
};
