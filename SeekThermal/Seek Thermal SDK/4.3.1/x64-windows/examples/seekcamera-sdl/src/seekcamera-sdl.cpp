/*Copyright (c) [2020] [Seek Thermal, Inc.]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The Software may only be used in combination with Seek cores/products.

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Project:	 Seek Thermal SDK Demo
 * Purpose:	 Demonstrates how to communicate with Seek Thermal Cameras
 * Author:	 Seek Thermal, Inc.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// C includes
#include <cstring>

// C++ includes
#include <algorithm>
#include <array>
#include <atomic>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <condition_variable>
#include <mutex>

// SDL includes
#if defined(__linux__) || defined(__APPLE__)
#	include <SDL2/SDL.h>
#elif defined(_WIN32)
#	define SDL_MAIN_HANDLED
#	include <SDL.h>
#endif

// Seek SDK includes
#include "seekcamera/seekcamera.h"
#include "seekcamera/seekcamera_manager.h"
#include "seekframe/seekframe.h"

// Structure representing a renderering interface.
// It uses SDL and all rendering is done on the calling thread.
struct seekrenderer_t
{
	seekcamera_t* camera{};

	// Rendering data
	SDL_Window* window{};
	SDL_Renderer* renderer{};
	SDL_Texture* texture{};

	// Synchronization data
	std::atomic<bool> is_active;
	std::atomic<bool> is_dirty;
	seekcamera_frame_t* frame{};
};

// Define the global variables
static std::atomic<bool> g_exit_requested;                       // Controls application shutdown.
static std::map<seekcamera_t*, seekrenderer_t*> g_renderers;     // Tracks all renderers.
static std::mutex g_mutex;										 // Synchronizes camera events that need to be coordinated with main thread.
static std::condition_variable g_condition_variable;			 // Synchronizes camera events that need to be coordinated with main thread.
static std::atomic<bool> g_is_dirty;

// Switches Image Processing Filter Pipeline Mode
// Filter options = 0,1,2
bool seekrenderer_switch_pipeline_mode(seekrenderer_t* renderer)
{

	if(!renderer->is_active.load())
	{
		return false;
	}

	// get value for current filter mode
	seekcamera_pipeline_mode_t current_mode;
	if(seekcamera_get_pipeline_mode(renderer->camera, &current_mode) != SEEKCAMERA_SUCCESS) 
		return false;

	// Rotate through smoothing filter values and cycle back to the beginning once SEEKVISION is hit
	current_mode = (seekcamera_pipeline_mode_t)((current_mode + 1) % SEEKCAMERA_IMAGE_LASTVALUE);
	std::cout << "Filter pipeline mode: " << seekcamera_pipeline_mode_get_str(current_mode) << std::endl;
	return seekcamera_set_pipeline_mode(renderer->camera, current_mode) == SEEKCAMERA_SUCCESS;
}

// Toggles the sharpen filter state
// 0 = disable, 1 = enable
bool seekrenderer_toggle_sharpen_filter(seekrenderer_t* renderer)
{
	if(!renderer->is_active.load())
	{
		return false;
	}
	// get value for current filter mode
	seekcamera_pipeline_mode_t current_mode;
	if(seekcamera_get_pipeline_mode(renderer->camera, &current_mode) != SEEKCAMERA_SUCCESS)
	{
		std::cout << "Failed to get the filter mode" << std::endl;
		return false;
	}

	if(current_mode == SEEKCAMERA_IMAGE_SEEKVISION)
	{
		std::cout << "Cannot update the sharpen correction filter while using SeekVision mode" << std::endl;
		return false;
	}
	//Get the current sharpen correction state
	seekcamera_filter_state_t current_state;
	if(seekcamera_get_filter_state(renderer->camera, SEEKCAMERA_FILTER_SHARPEN_CORRECTION, &current_state) != SEEKCAMERA_SUCCESS)
		return false;

	// Toggle the sharpen correction state, circling around from disabled to enabled state
	current_state = (seekcamera_filter_state_t)((current_state + 1) % SEEKCAMERA_FILTER_STATE_LASTVALUE);
	std::cout << "sharpen correction: " << seekcamera_get_filter_state_str(SEEKCAMERA_FILTER_SHARPEN_CORRECTION, current_state) << std::endl;
	return seekcamera_set_filter_state(renderer->camera, SEEKCAMERA_FILTER_SHARPEN_CORRECTION, current_state) == SEEKCAMERA_SUCCESS;
}

// Switches the current color palette.
// Settings will be refreshed between frames.
bool seekrenderer_switch_color_palette(seekrenderer_t* renderer)
{
	if(!renderer->is_active.load())
	{
		return false;
	}
	seekcamera_color_palette_t current_palette;
	if(seekcamera_get_color_palette(renderer->camera, &current_palette) != SEEKCAMERA_SUCCESS)
		return false;

	// Not including the user palettes so we will cycle back to the beginning once GREEN is hit
	current_palette = (seekcamera_color_palette_t)((current_palette + 1) % SEEKCAMERA_COLOR_PALETTE_USER_0);
	std::cout << "color palette: " << seekcamera_color_palette_get_str(current_palette) << std::endl;
	return seekcamera_set_color_palette(renderer->camera, current_palette) == SEEKCAMERA_SUCCESS;
}

// Closes the SDL window associated with a renderer.
void seekrenderer_close_window(seekrenderer_t* renderer)
{
	if(renderer == NULL)
		return;
	if(renderer->is_active.load())
		seekcamera_capture_session_stop(renderer->camera);
	renderer->is_active.store(false); 
	renderer->is_dirty.store(false);
	renderer->frame = nullptr;

	if(renderer->texture != NULL)
	{
		SDL_DestroyTexture(renderer->texture);
		renderer->texture = nullptr;
	}

	if(renderer->renderer != NULL)
	{
		SDL_DestroyRenderer(renderer->renderer);
		renderer->renderer = nullptr;
	}

	if(renderer->window != NULL)
	{
		SDL_DestroyWindow(renderer->window);
		renderer->window = nullptr;
	}
	// Invalidate references that rely on the camera lifetime.
	renderer->camera = nullptr;
}

void seekrenderer_close_all()
{
	for(auto& kvp : g_renderers)
	{
		if(kvp.second != nullptr)
		{
			seekrenderer_close_window(kvp.second);
		}
	}
}

seekrenderer_t* seekrenderer_find_by_window_id(Uint32 id)
{
	// Find the renderer associated with this window.
	for(auto& kvp : g_renderers)
	{
		auto renderer = kvp.second;
		if(renderer == nullptr || !renderer->is_active.load())
			continue;
		if(SDL_GetWindowID(renderer->window) == id)
			return renderer;
	}
	return nullptr;
}

// Handles frame available events.
void handle_camera_frame_available(seekcamera_t* camera, seekcamera_frame_t* camera_frame, void* user_data)
{
	(void)camera;
	auto* renderer = (seekrenderer_t*)user_data;

	// Lock the seekcamera frame for safe use outside of this callback.
	seekcamera_frame_lock(camera_frame);
	renderer->is_dirty.store(true);

	// Note that this will always render the most recent frame. There could be better buffering here but this is a simple example.
	renderer->frame = camera_frame;
	g_is_dirty.store(true);
	g_condition_variable.notify_one();
}

// Handles camera connect events.
void handle_camera_connect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)event_status;
	(void)user_data;
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	seekrenderer_t* renderer = g_renderers[camera] == nullptr ? new seekrenderer_t() : g_renderers[camera];
	renderer->is_active.store(true);
	renderer->camera = camera;

	// Register a frame available callback function.
	seekcamera_error_t status = seekcamera_register_frame_available_callback(camera, handle_camera_frame_available, (void*)renderer);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to register frame callback: " << seekcamera_error_get_str(status) << std::endl;
		renderer->is_active.store(false);
		return;
	}

	// Start the capture session.
	status = seekcamera_capture_session_start(camera, SEEKCAMERA_FRAME_FORMAT_COLOR_ARGB8888);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to start capture session: " << seekcamera_error_get_str(status) << std::endl;
		renderer->is_active.store(false);
		return;
	}
	g_renderers[camera] = renderer;
}

// Handles camera disconnect events.
void handle_camera_disconnect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)event_status;
	(void)user_data;
	auto renderer = g_renderers[camera];
	if(renderer != nullptr)
	{
		renderer->is_active.store(false);
		g_is_dirty.store(true);
	}
		
}

// Handles camera error events.
void handle_camera_error(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)user_data;
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	std::cerr << "unhandled camera error: (CID: " << cid << ")" << seekcamera_error_get_str(event_status) << std::endl;
}

// Handles camera ready to pair events
void handle_camera_ready_to_pair(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	// Attempt to pair the camera automatically.
	// Pairing refers to the process by which the sensor is associated with the host and the embedded processor.
	const seekcamera_error_t status = seekcamera_store_calibration_data(camera, nullptr, nullptr, nullptr);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to pair device: " << seekcamera_error_get_str(status) << std::endl;
	}

	// Start imaging.
	handle_camera_connect(camera, event_status, user_data);
}

// Callback function for the camera manager; it fires whenever a camera event occurs.
void camera_event_callback(seekcamera_t* camera, seekcamera_manager_event_t event, seekcamera_error_t event_status, void* user_data)
{
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	std::cout << seekcamera_manager_get_event_str(event) << " (CID: " << cid << ")" << std::endl;

	// Handle the event type.
	switch(event)
	{
		case SEEKCAMERA_MANAGER_EVENT_CONNECT:
			handle_camera_connect(camera, event_status, user_data);
			break;
		case SEEKCAMERA_MANAGER_EVENT_DISCONNECT:
			handle_camera_disconnect(camera, event_status, user_data);
			break;
		case SEEKCAMERA_MANAGER_EVENT_ERROR:
			handle_camera_error(camera, event_status, user_data);
			break;
		case SEEKCAMERA_MANAGER_EVENT_READY_TO_PAIR:
			handle_camera_ready_to_pair(camera, event_status, user_data);
			break;
		default:
			break;
	}
}

// Application entry point.
int main()
{
	// Initialize global variables.
	g_exit_requested.store(false);
	g_is_dirty.store(false);

	std::cout << "seekcamera-sdl starting\n";
	std::cout << "user controls:\n\t1) mouse click: next color palette\n\t2) q: quit" << std::endl;

	// Initialize SDL and enable bilinear stretching.
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	std::cout << "display driver: " << SDL_GetVideoDriver(0) << std::endl;

	// Create the camera manager.
	// This is the structure that owns all Seek camera devices.
	seekcamera_manager_t* manager = nullptr;
	seekcamera_error_t status = seekcamera_manager_create(&manager, SEEKCAMERA_IO_TYPE_USB);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to create camera manager: " << seekcamera_error_get_str(status) << std::endl;
		return 1;
	}

	// Register an event handler for the camera manager to be called whenever a camera event occurs.
	status = seekcamera_manager_register_event_callback(manager, camera_event_callback, nullptr);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to register camera event callback: " << seekcamera_error_get_str(status) << std::endl;
		return 1;
	}

	// Poll for events until told to stop.
	// Both renderer events and SDL events are polled.
	// Events are polled on the main thread because SDL events must be handled on the main thread.
	while(!g_exit_requested.load())
	{
		std::unique_lock<std::mutex> event_lock(g_mutex);
		if(g_condition_variable.wait_for(event_lock, std::chrono::milliseconds(150), [=] { return g_is_dirty.load(); }))
		{
			g_is_dirty.store(false);
			for(auto& kvp : g_renderers)
			{
				auto renderer = kvp.second;
				if(renderer == NULL)
					continue;

				// Close inactive windows
				if(!renderer->is_active.load())
				{
					seekrenderer_close_window(renderer);
					continue;
				}
					

				// Create window if not created yet
				if(renderer->is_active.load() && renderer->window == NULL && renderer->renderer == NULL)
				{
					// Set the window title.
					seekcamera_chipid_t cid{};
					seekcamera_get_chipid(renderer->camera, &cid);
					std::stringstream window_title;
					window_title << "Seek Thermal - SDL Sample (CID: " << cid << ")";

					// Setup the window handle.
					SDL_Window* window = SDL_CreateWindow(window_title.str().c_str(), 100, 100, 0, 0, SDL_WINDOW_HIDDEN);
#if SDL_VERSION_ATLEAST(2, 0, 5)
					SDL_SetWindowResizable(window, SDL_TRUE);
#endif
					// Setup the window renderer.
					SDL_Renderer* window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

					// Setup the renderer area.
					renderer->window = window;
					renderer->renderer = window_renderer;
				}

				// Render frame if necessary
				if(renderer->is_dirty.load())
				{
					if(renderer->frame == NULL || !renderer->is_active.load())
						continue;

					// Get the frame to draw.
					seekframe_t* frame = nullptr;
					status = seekcamera_frame_get_frame_by_format(renderer->frame, SEEKCAMERA_FRAME_FORMAT_COLOR_ARGB8888, &frame);
					if(status != SEEKCAMERA_SUCCESS)
					{
						std::cerr << "failed to get frame: " << seekcamera_error_get_str(status) << std::endl;
						seekcamera_frame_unlock(renderer->frame);
						continue;
					}

					// Get the frame dimensions.
					const int frame_width = (int)seekframe_get_width(frame);
					const int frame_height = (int)seekframe_get_height(frame);
					const int frame_stride = (int)seekframe_get_line_stride(frame);

					// Lazy allocate the texture data.
					if(renderer->texture == nullptr)
					{
						// Resize and show the window -- upscaling by two.
						const int scale_factor = 2;
						SDL_RenderSetLogicalSize(renderer->renderer, frame_width * scale_factor, frame_height * scale_factor);
						renderer->texture = SDL_CreateTexture(renderer->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, frame_width, frame_height);
						SDL_SetWindowSize(renderer->window, frame_width * scale_factor, frame_height * scale_factor);
						SDL_ShowWindow(renderer->window);
					}

					// Update the SDL windows and renderers.
					SDL_UpdateTexture(renderer->texture, nullptr, seekframe_get_data(frame), frame_stride);
					SDL_RenderCopy(renderer->renderer, renderer->texture, nullptr, nullptr);
					SDL_RenderPresent(renderer->renderer);

					// Unlock the camera frame.
					seekcamera_frame_unlock(renderer->frame);
					renderer->is_dirty.store(false);
					renderer->frame = nullptr;
				}
			}
		}
		else
		{
			event_lock.unlock();
		}

		// Handle the SDL window events.
		// The events need to be polled in order for the window to be responsive.
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			seekrenderer_t* renderer = seekrenderer_find_by_window_id(event.window.windowID);

			if(renderer == nullptr)
			{
				continue;
			}

			if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				seekrenderer_close_window(renderer);
			}
			else if(event.type == SDL_MOUSEBUTTONDOWN)
			{
				seekrenderer_switch_color_palette(renderer);
			}
			 else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_n)
			 {
				 seekrenderer_switch_pipeline_mode(renderer);
			 }
			else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s)
			{
				seekrenderer_toggle_sharpen_filter(renderer);
			}
			else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q)
			{
				g_exit_requested.store(true);
			}
		}
	}

	std::cout << "Destroying camera manager" << std::endl;

	// Teardown the camera manager.
	seekcamera_manager_destroy(&manager);

	seekrenderer_close_all();
	g_renderers.clear();

	// Teardown SDL.
	SDL_Quit();
	std::cout << "done" << std::endl;
	return 0;
}
