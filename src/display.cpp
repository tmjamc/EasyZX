#include <cstdint>
#include <thread>

#include "glad/gl.h"
#include "display.h"
#include "win_app.h"

namespace display
{
	constexpr int DISPLAY_BUFFER_WIDTH = 352;
	constexpr int DISPLAY_BUFFER_HEIGHT = 288;

	uint32_t* displayBuffer = nullptr;

	bool viewportChanged = false;
	int width;
	int height;

	std::thread* thread;

	static void run()
	{
		wglMakeCurrent(win_app::hDC, win_app::glCtx);

		while (win_app::running)
		{
			// check if window size has changed
			if (viewportChanged)
			{
				viewportChanged = false;
				glViewport(0, 0, width, height);
			}
			
			Sleep(1);

			glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			SwapBuffers(win_app::hDC);
		}

		wglMakeCurrent(win_app::hDC, nullptr);
	}

	void startThread()
	{
		thread = new std::thread(run);
	}

	void stopThread()
	{
		thread->join();
		delete thread;
	}

	void setViewport(int width, int height)
	{
		display::width = width;
		display::height = height;
		viewportChanged = true;
	}
}