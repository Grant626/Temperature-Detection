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

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#	include <windows.h>
#	define inline __inline
#endif

#if defined(__linux__) || defined(__APPLE__)
#	include <unistd.h>
#	include <sys/time.h>
#endif

#include "seekcamera/seekcamera.h"
#include "seekcamera/seekcamera_manager.h"

// Options
#define NUM_MAX_DEVICES     15
#define MAX_FILENAME_LENGTH 64

// Structure holding the context for a Seek camera and additional application level metadata.
typedef struct samplectx_t
{
	bool is_free;
	bool is_live;
	FILE* log;
	seekcamera_t* camera;
} samplectx_t;

// Define the global variables.
volatile bool g_keep_running = true;
static samplectx_t g_ctx_pool[NUM_MAX_DEVICES] = { 0 };

// Signal handler function.
static void signal_callback(int signum)
{
	(void)signum;

	fprintf(stdout, "\nCaught Ctrl+C\n\n");
	g_keep_running = false;
}

// Prints the usage instructions.
void print_usage()
{
	fprintf(stdout, "Allowed options\n");
	fprintf(stdout, "\t-m : Discovery mode. Valid options: usb, spi, all (default: usb)\n");
	fprintf(stdout, "\t   : Required - No\n");
	fprintf(stdout, "\t-h : Displays this message\n");
	fprintf(stdout, "\t   : Required - No\n");
}

// Callback function for a particular Seek camera.
// This function fires whenever a frame is available.
void frame_available_callback(seekcamera_t* camera, seekcamera_frame_t* camera_frame, void* user_data)
{
	samplectx_t* ctx = (samplectx_t*)user_data;

	if(!ctx->is_live)
	{
		fprintf(stderr, "unable to continue: camera is not live\n");
		return;
	}

	if(ctx->log == NULL)
	{
		fprintf(stderr, "unable to continue: log handle is invalid\n");
		return;
	}

	seekcamera_chipid_t cid;
	seekcamera_get_chipid(camera, &cid);

	seekframe_t* frame = NULL;
	const seekcamera_error_t status = seekcamera_frame_get_frame_by_format(
		camera_frame,
		SEEKCAMERA_FRAME_FORMAT_THERMOGRAPHY_FLOAT,
		&frame);

	if(status != SEEKCAMERA_SUCCESS)
	{
		fprintf(stderr, "failed to get thermal frame: %s (%s)", cid, seekcamera_error_get_str(status));
		return;
	}

	const size_t width = seekframe_get_width(frame);
	const size_t height = seekframe_get_height(frame);

	fprintf(stdout, "frame available: %s (size: %zux%zu)\n", cid, width, height);

	// Log each header value to the CSV file.
	// See the documentation for a description of the header.
	seekcamera_frame_header_t* header = (seekcamera_frame_header_t*)seekframe_get_header(frame);

	size_t count = 0;

	fprintf(ctx->log, "sentinel=%u,", header->sentinel);
	++count;

	fprintf(ctx->log, "version=%u,", header->version);
	++count;

	fprintf(ctx->log, "type=%u,", header->type);
	++count;

	fprintf(ctx->log, "width=%u,", header->width);
	++count;

	fprintf(ctx->log, "height=%u,", header->height);
	++count;

	fprintf(ctx->log, "channels=%u,", header->channels);
	++count;

	fprintf(ctx->log, "pixel_depth=%u,", header->pixel_depth);
	++count;

	fprintf(ctx->log, "pixel_padding=%u,", header->pixel_padding);
	++count;

	fprintf(ctx->log, "line_padding=%u,", header->line_padding);
	++count;

	fprintf(ctx->log, "header_size=%u,", header->header_size);
	++count;

	fprintf(ctx->log, "timestamp_utc_ns=%zu,", header->timestamp_utc_ns);
	++count;

	fprintf(ctx->log, "chipid=%s,", header->chipid);
	++count;

	fprintf(ctx->log, "serial_number=%s,", header->serial_number);
	++count;

	fprintf(ctx->log, "core_part_number=%s,", header->core_part_number);
	++count;

	fprintf(ctx->log, "firmware_version=%u.%u.%u.%u,", header->firmware_version[0], header->firmware_version[1], header->firmware_version[2], header->firmware_version[3]);
	++count;

	fprintf(ctx->log, "io_type=%u,", header->io_type);
	++count;

	fprintf(ctx->log, "fpa_frame_count=%u,", header->fpa_frame_count);
	++count;

	fprintf(ctx->log, "fpa_diode_count=%u,", header->fpa_diode_count);
	++count;

	fprintf(ctx->log, "environment_temperature=%f,", header->environment_temperature);
	++count;

	fprintf(ctx->log, "thermography_min_x=%u,", header->thermography_min_x);
	++count;

	fprintf(ctx->log, "thermography_min_y=%u,", header->thermography_min_y);
	++count;

	fprintf(ctx->log, "thermography_min_value=%f,", header->thermography_min_value);
	++count;

	fprintf(ctx->log, "thermography_max_x=%u,", header->thermography_max_x);
	++count;

	fprintf(ctx->log, "thermography_max_y=%u,", header->thermography_max_y);
	++count;

	fprintf(ctx->log, "thermography_max_value=%f,", header->thermography_max_value);
	++count;

	fprintf(ctx->log, "thermography_spot_x=%u,", header->thermography_spot_x);
	++count;

	fprintf(ctx->log, "thermography_spot_y=%u,", header->thermography_spot_y);
	++count;

	fprintf(ctx->log, "thermography_spot_value=%f,", header->thermography_spot_value);
	++count;

	fprintf(ctx->log, "agc_mode=%u,", header->agc_mode);
	++count;

	fprintf(ctx->log, "histeq_agc_num_bins=%u,", header->histeq_agc_num_bins);
	++count;

	fprintf(ctx->log, "histeq_agc_bin_width=%u,", header->histeq_agc_bin_width);
	++count;

	fprintf(ctx->log, "histeq_agc_gain_limit_factor=%f,", header->histeq_agc_gain_limit_factor);
	++count;

	for(size_t i = 0; i < sizeof(header->histeq_agc_reserved) / sizeof(header->histeq_agc_reserved[0]); ++i)
	{
		fprintf(ctx->log, "histeq_agc_reserved,");
		++count;
	}

	fprintf(ctx->log, "linear_agc_min=%f,", header->linear_agc_min);
	++count;

	fprintf(ctx->log, "linear_agc_max=%f,", header->linear_agc_max);
	++count;

	for(size_t i = 0; i < sizeof(header->linear_agc_reserved) / sizeof(header->linear_agc_reserved[0]); ++i)
	{
		fprintf(ctx->log, "linear_agc_reserved,");
		++count;
	}

	fprintf(ctx->log, "gradient_correction_filter_state=%u,", header->gradient_correction_filter_state);
	++count;

	fprintf(ctx->log, "flat_scene_correction_filter_state=%u,", header->flat_scene_correction_filter_state);
	++count;

	for(size_t i = count; i < width; ++i)
	{
		fprintf(ctx->log, "blank,");
	}
	fputc('\n', ctx->log);

	// Log each temperature value to the CSV file.
	// See the documentation for a description of the frame layout.
	for(size_t y = 0; y < height; ++y)
	{
		float* pixels = (float*)seekframe_get_row(frame, y);
		for(size_t x = 0; x < width; ++x)
		{
			const float temperature_degrees_c = pixels[x];
			fprintf(ctx->log, "%.1f,", temperature_degrees_c);
		}
		fputc('\n', ctx->log);
	}
}

// Handles camera connect events.
void handle_camera_connect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)event_status;
	(void)user_data;

	// Each camera is associated with an application level context structure.
	// On each connect, the available context resource pool is searched to find a free context.
	samplectx_t* ctx = NULL;
	for(int i = 0; i < NUM_MAX_DEVICES && (ctx == NULL); ++i)
	{
		if(g_ctx_pool[i].is_free)
		{
			ctx = &(g_ctx_pool[i]);
		}
	}

	// The context pool is exhausted.
	// This can happen as the result of a large number of devices connected.
	// This is a limitation of the sample application -- not a limitation of the Seek API.
	if(ctx == NULL)
	{
		fprintf(stderr, "camera context pool is exhausted\n");
		return;
	}

	seekcamera_chipid_t cid;
	seekcamera_get_chipid(camera, &cid);

	// Reset the context values to be assocated with this camera.
	ctx->is_free = false;
	ctx->is_live = false;
	ctx->log = NULL;
	ctx->camera = camera;

	// The Seek camera API is asynchronous and event driven.
	// Frames are delivered to a unique callback function which is registered on a per camera basis.
	// Each callback passes an optional piece of user data.
	// The sample application passes the associated context structure as this optional piece of user data.
	seekcamera_error_t status = seekcamera_register_frame_available_callback(
		camera,
		frame_available_callback,
		(void*)ctx);

	if(status == SEEKCAMERA_SUCCESS)
	{
		fprintf(stdout, "registered camera callback: %s\n", cid);
	}
	else
	{
		fprintf(stderr, "failed to register camera callback: %s (%s)\n", cid, seekcamera_error_get_str(status));
	}

	// Start the capture session.
	// The capture session is non-blocking.
	// Several types of output imagery are configurable.
	// This sample application only outputs thermography values as single precision floating point values.
	const uint32_t frame_format = SEEKCAMERA_FRAME_FORMAT_THERMOGRAPHY_FLOAT;
	status = seekcamera_capture_session_start(camera, frame_format);

	if(status == SEEKCAMERA_SUCCESS)
	{
		fprintf(stdout, "started capture session: %s\n", cid);
		ctx->is_live = true;
	}
	else
	{
		fprintf(stderr, "failed to start capture session: %s (%s)\n", cid, seekcamera_error_get_str(status));
		ctx->is_live = false;
	}

	// Create the thermography log file.
	// Each log file is associated with a camera by its unique chip id.
	// Log files will be overwritten if the camera is repeatedly connected and disconnected -- or if the application is repeatedly launched.
	char filename[MAX_FILENAME_LENGTH] = { 0 };
	snprintf(filename, MAX_FILENAME_LENGTH, "thermography-%s.csv", cid);

	ctx->log = fopen(filename, "w");
	if(ctx->log != NULL)
	{
		fprintf(stdout, "opened log file: %s (%s)\n", cid, filename);
	}
	else
	{
		fprintf(stderr, "failed to open log file: %s\n", cid);
	}
}

// Handles camera disconnect events.
void handle_camera_disconnect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)event_status;
	(void)user_data;

	// Search for the resource pool for the context associated with the camera.
	// The context and camera can be uniquely associated using chip id.
	samplectx_t* ctx = NULL;
	for(int i = 0; i < NUM_MAX_DEVICES && (ctx == NULL); ++i)
	{
		samplectx_t* candidate_ctx = &(g_ctx_pool[i]);
		if(candidate_ctx->camera != NULL)
		{
			seekcamera_chipid_t cid;
			seekcamera_get_chipid(camera, &cid);

			seekcamera_chipid_t candidate_cid;
			seekcamera_get_chipid(candidate_ctx->camera, &candidate_cid);

			if(strcmp(cid, candidate_cid) == 0)
			{
				ctx = candidate_ctx;
			}
		}
	}

	// The camera is not associated with any context.
	// This should never happen but is accounted for nonetheless.
	if(ctx == NULL)
	{
		fprintf(stderr, "failed to find associated context\n");
		return;
	}

	// Stop the capture session.
	// Care should be taken to synchronize any state depending on the camera.
	if(ctx->is_live)
	{
		seekcamera_capture_session_stop(camera);
	}

	// Close the log.
	if(ctx->log != NULL)
	{
		fclose(ctx->log);
		ctx->log = NULL;
	}

	// Invalidate the tracked metadata.
	ctx->is_free = true;
	ctx->is_live = false;
	ctx->camera = NULL;
}

// Handles camera error events.
void handle_camera_error(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)user_data;

	seekcamera_chipid_t cid;
	seekcamera_get_chipid(camera, &cid);
	fprintf(stderr, "encountered unexpected error: %s (%s)", cid, seekcamera_error_get_str(event_status));
}

// Callback function for the Seek camera manager.
// This function fires whenever a camera event occurs for a given camera manager context.
void camera_event_callback(seekcamera_t* camera, seekcamera_manager_event_t event, seekcamera_error_t event_status, void* user_data)
{
	seekcamera_chipid_t cid;
	seekcamera_get_chipid(camera, &cid);

	fprintf(stdout, "%s: %s\n", seekcamera_manager_get_event_str(event), cid);

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
		default:
			break;
	}
}

// Application entry point.
int main(int argc, char** argv)
{
	// Install signal handlers.
	signal(SIGINT, signal_callback);
	signal(SIGTERM, signal_callback);

	// Default values for the command line arguments.
	char* discovery_mode_str = "usb";
	seekcamera_io_type_t discovery_mode = (uint32_t)SEEKCAMERA_IO_TYPE_USB;

	// Parse command line arguments.
	for(int i = 1; i < argc; ++i)
	{
		const char* arg = argv[i];
		char ch = arg[0];
		if(ch == '-' && strlen(arg) >= 2)
		{
			ch = arg[1];
			switch(ch)
			{
				case 'm':
					if(i < argc - 1)
					{
						discovery_mode_str = argv[i + 1];
						if(strcmp(discovery_mode_str, "usb") == 0)
						{
							discovery_mode = (uint32_t)SEEKCAMERA_IO_TYPE_USB;
						}
						else if(strcmp(discovery_mode_str, "spi") == 0)
						{
							discovery_mode = (uint32_t)SEEKCAMERA_IO_TYPE_SPI;
						}
						else if(strcmp(discovery_mode_str, "all") == 0)
						{
							discovery_mode = (uint32_t)SEEKCAMERA_IO_TYPE_USB | (uint32_t)SEEKCAMERA_IO_TYPE_SPI;
						}
						else
						{
							print_usage();
							return 1;
						}
					}
					else
					{
						print_usage();
						return 1;
					}
					break;
				case 'h':
					print_usage();
					return 0;
				default:
					break;
			}
		}
	}

	fprintf(stdout, "seekcamera-simple starting\n");
	fprintf(stdout, "settings\n");
	fprintf(stdout, "\t1) mode (-m): %s\n", discovery_mode_str);
	fflush(stdout);

	// Setup the global context pool.
	// Each context tracks additional application-level meta data that is associated on a per-camera basis.
	for(int i = 0; i < NUM_MAX_DEVICES; ++i)
	{
		g_ctx_pool[i].is_free = true;
		g_ctx_pool[i].is_live = false;
		g_ctx_pool[i].log = NULL;
		g_ctx_pool[i].camera = NULL;
	}

	// Create the camera manager.
	// This is the structure that owns all Seek camera devices.
	seekcamera_manager_t* manager = NULL;
	seekcamera_error_t status = seekcamera_manager_create(&manager, discovery_mode);
	if(status != SEEKCAMERA_SUCCESS)
	{
		fprintf(stderr, "failed to create camera manager: %s\n", seekcamera_error_get_str(status));
		return 1;
	}

	// Register an event handler for the camera manager.
	// The event handler will be called for every event on a per-camera basis.
	void* user_data = NULL;
	status = seekcamera_manager_register_event_callback(manager, camera_event_callback, user_data);
	if(status != SEEKCAMERA_SUCCESS)
	{
		fprintf(stderr, "failed to register camera event callback: %s\n", seekcamera_error_get_str(status));
		seekcamera_manager_destroy(&manager);
		return 1;
	}

	// Camera events are asynchronous and interrupt the current thread.
	// There are much better ways of keeping the main thread alive.
	// This is just a simple example.
	while(g_keep_running)
	{
		const int sleep_ms = 1000;
#ifdef _WIN32
		Sleep(sleep_ms);
#else
		usleep(sleep_ms * 1000);
#endif
	}

	// Cleanup the camera manager.
	// Cameras will be disconnected and invalidated.
	status = seekcamera_manager_destroy(&manager);
	if(status != SEEKCAMERA_SUCCESS)
	{
		fprintf(stderr, "failed to free camera manager: %s\n", seekcamera_error_get_str(status));
		return 1;
	}

	// Invalidate the camera contexts.
	for(int i = 0; i < NUM_MAX_DEVICES; ++i)
	{
		g_ctx_pool[i].is_free = true;
		g_ctx_pool[i].is_live = false;
		g_ctx_pool[i].log = NULL;
		g_ctx_pool[i].camera = NULL;
	}

	fprintf(stdout, "done\n");

	return 0;
}
