#include <cstdint>
#include <thread>

#include "glad/gl.h"
#include "display.h"
#include "win_app.h"
#include "settings.h"
#include "main.h"
#include "shader.h"
#include "realtime.h"
#include "resources/resources.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"

namespace display
{
	namespace
	{
		bool viewportChanged = true;
		bool shaderChanged = true;
		int glWidth;
		int glHeight;
		float width;
		float height;

		uint32_t texture = 0;
		uint32_t VAO = 0;
		uint32_t VBO = 0;
		uint32_t EBO = 0;

		std::thread renderThread;

		float vertices[]
		{
			-1.0f, -1.0f,   0.0f, 1.0f,
			-1.0f,  1.0f,   0.0f, 0.0f,
			1.0f, -1.0f,   1.0f, 1.0f,
			1.0f,  1.0f,   1.0f, 0.0f,
		};

		unsigned int indices[]
		{
			0, 1, 3,
			3, 2, 0
		};

		void init()
		{
			// Set OpenGL context
			wglMakeCurrent(win_app::hDC, win_app::glCtx);

			glClearColor(settings::current.displayBackgroundColorR, settings::current.displayBackgroundColorG, settings::current.displayBackgroundColorB, 1.0f);

			// Declare vertex arrays and buffers
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			// Declare elements buffer array for triangle indices
			glGenBuffers(1, &EBO);

			// Bind the Vertex Array Object
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			// Bind the elements buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			// Declare vertices positions attribute for shaders
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			// Declare uv coordinates attribute for shaders
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			glEnableVertexAttribArray(1);

			// Declare and bind texture for screen drawing
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			// Set the texture wrapping parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			// Set texture filtering parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Set texture dimensions
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GL_DISPLAY_BUFFER_WIDTH, GL_DISPLAY_BUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			// Initialize ImGui Platform and Renderer backends
			ImGui_ImplWin32_InitForOpenGL(win_app::hWnd);
			ImGui_ImplOpenGL3_Init();
		}

		void run()
		{
			init();

			glClear(GL_COLOR_BUFFER_BIT);
			SwapBuffers(win_app::hDC);

			while (!main::emulationThreadReady)
			{
				Sleep(1);
			}

			while (win_app::running)
			{
				// Wait for frame to be ready
				std::unique_lock<std::mutex> lock(frameReadyMutex);
				frameReadyConditionVariable.wait(lock, []
				{
					return frameReady;
				});
				frameReady = false;

				if (viewportChanged)
				{
					viewportChanged = false;
					glViewport(0, 0, glWidth, glHeight);
				}
				
				if (shaderChanged)
				{
					shaderChanged = false;
					// TODO: get shader from settings

					shader::cleanUp();
					shader::compile(IDR_SHADER_SHARP_VERT, IDR_SHADER_SHARP_FRAG);
				}

				glClear(GL_COLOR_BUFFER_BIT);

				// Upload the framebuffer into the OpenGL texture.
				// The texture is reused every frame to avoid recreating GPU resources.
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GL_DISPLAY_BUFFER_WIDTH, GL_DISPLAY_BUFFER_HEIGHT, GL_BGRA, GL_UNSIGNED_BYTE, displayBuffer);

				// Prepare variables for further calculations
				float normalizedDisplayBufferWidth = DISPLAY_BUFFER_WIDTH;
				float normalizedDisplayBufferHeight = DISPLAY_BUFFER_HEIGHT;
				
				// Convert user-configured border sizes into texture-space UV
				// coordinates so overscan/borders can be cropped without
				// modifying the original framebuffer.
				float clipWidth = (MAX_BORDER_SIZE - static_cast<float>(settings::current.displayHorizontalBorderSize)) / normalizedDisplayBufferWidth;
				float clipHeight = (MAX_BORDER_SIZE - static_cast<float>(settings::current.displayVerticalBorderSize)) / normalizedDisplayBufferHeight;

				// Apply pixel ratio
				normalizedDisplayBufferWidth *= settings::current.displayPixelRatio;

				// Set uv coordinates
				vertices[2] = vertices[6] = clipWidth;
				vertices[3] = vertices[11] = 1.0f - clipHeight;
				vertices[10] = vertices[14] = 1.0f - clipWidth;
				vertices[7] = vertices[15] = clipHeight;

				// Normalize width and height with new border sizes
				normalizedDisplayBufferWidth -= (normalizedDisplayBufferWidth * clipWidth * 2.0f);
				normalizedDisplayBufferHeight -= (normalizedDisplayBufferHeight * clipHeight * 2.0f);

				// Maintain the screen aspect ratio while using the maximum
				// available window area. Depending on the window dimensions,
				// either horizontal or vertical letterboxing will occur.
				float calcX, calcY, calcWidth, calcHeight;
				if (width / normalizedDisplayBufferWidth > height / normalizedDisplayBufferHeight)
				{
					// Window is wider than the display aspect ratio.
					// Fit the image vertically and center it horizontally.
					calcHeight = height;
					calcWidth = (height * normalizedDisplayBufferWidth) / normalizedDisplayBufferHeight;
					calcX = (width - calcWidth) / 2.0f;
					calcY = 0.0f;
				}
				else
				{
					// Window is taller than the display aspect ratio.
					// Fit the image horizontally and center it vertically.
					calcHeight = (width * normalizedDisplayBufferHeight) / normalizedDisplayBufferWidth;
					calcWidth = width;
					calcY = (height - calcHeight) / 2.0f;
					calcX = 0.0f;
				}

				// Set vertices positions
				vertices[0] = vertices[4] = calcX * 2.0f / width - 1.0f;
				vertices[1] = vertices[9] = calcY * 2.0f / height - 1.0f;
				vertices[8] = vertices[12] = (calcX + calcWidth) * 2.0f / width - 1.0f;
				vertices[5] = vertices[13] = (calcY + calcHeight) * 2.0f / height - 1.0f;

				// Set shader uniforms
				glUseProgram(shader::shaderId);
				shader::setVec2("TextureSize", DISPLAY_BUFFER_WIDTH, DISPLAY_BUFFER_HEIGHT);
				shader::setVec2("InputSize", DISPLAY_BUFFER_WIDTH, DISPLAY_BUFFER_HEIGHT);
				shader::setVec2("OutputSize", width, height);

				// Render screen
				glBindVertexArray(VAO);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				// Start the Dear ImGui frame
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				// Render UI
				realtime::render();

				// Render ImGui
				ImGui::Render();
		        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

				// Present
				SwapBuffers(win_app::hDC);
			}

			ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplWin32_Shutdown();
			
			shader::cleanUp();
		}
	}

	uint32_t* displayBuffer;
	bool frameReady = false;
	std::mutex frameReadyMutex;
	std::condition_variable frameReadyConditionVariable;

	void startRenderThread()
	{
		displayBuffer = new uint32_t[GL_DISPLAY_BUFFER_SIZE];
		std::fill(displayBuffer, displayBuffer + GL_DISPLAY_BUFFER_SIZE, 0x00000000);

		renderThread = std::thread(run);
	}

	void stopRenderThread()
	{
		// Prevent render thread waiting forever for next frame to be ready
		frameReady = true;
		frameReadyConditionVariable.notify_one();

		if (renderThread.joinable())
		{
			renderThread.join();
		}

		delete[GL_DISPLAY_BUFFER_SIZE] displayBuffer;
	}

	void setViewport(int width, int height)
	{
		glWidth = width;
		glHeight = height;
		display::width = static_cast<float>(width);
		display::height = static_cast<float>(height);
		viewportChanged = true;
	}
}