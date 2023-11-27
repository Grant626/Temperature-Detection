/*Copyright (c) [2021] [Seek Thermal, Inc.]

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
#include <stdlib.h>
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

// Define the global variables.
int g_num_warmup_seconds = 60;
volatile bool g_keep_running = true;

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
	fprintf(stdout, "Allowed options:\n");
	fprintf(stdout, "\t-m : Discovery mode. Valid options: usb, spi, all (default: usb)\n");
	fprintf(stdout, "\t   : Required - No\n");
	fprintf(stdout, "\t-t : Number of seconds to wait for the camera to warm up before storing the flat scene correction (default: 60)\n");
	fprintf(stdout, "\t   : Required - No\n");
	fprintf(stdout, "\t-h : Displays this message\n");
	fprintf(stdout, "\t   : Required - No\n");
}

// Requests the program to exit.
void request_exit()
{
	fprintf(stdout, "exiting...\n");
	fflush(stdout);

	g_keep_running = false;
}

// Handles store progress updates.
void handle_store_progress_update(size_t progress, void* user_data)
{
	(void)user_data;

	if(progress == 100)
	{
		fprintf(stdout, "flat scene correction store %zu %% complete\n", progress);
	}
	else
	{
		fprintf(stdout, "flat scene correction store %zu %% complete\r", progress);
	}
	fflush(stdout);
}

// Handled delete progress updates.
void handle_delete_progress_update(size_t progress, void* user_data)
{
	(void)user_data;

	if(progress == 100)
	{
		fprintf(stdout, "flat scene correction delete %zu %% complete\n", progress);
	}
	else
	{
		fprintf(stdout, "flat scene correction delete %zu %% complete\r", progress);
	}
	fflush(stdout);
}

// Handles camera connect events.
void handle_camera_connect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)event_status;
	(void)user_data;

	if(!g_keep_running)
	{
		return;
	}

	seekcamera_chipid_t cid;
	seekcamera_error_t status = seekcamera_get_chipid(camera, &cid);
	if(status != SEEKCAMERA_SUCCESS)
	{
		fprintf(stderr, "failed to get CID: %s\n", seekcamera_error_get_str(status));
		request_exit();
		return;
	}

	char response[sizeof(cid)] = { 0 };

	// Prompt the user to choose a flat scene correction action.
	fprintf(stdout, "flat scene correction action (valid options: delete, store, quit): ");
	fflush(stdout);

	memset(response, 0, sizeof(response));
	if(fgets(response, sizeof(response), stdin) == NULL)
	{
		fprintf(stderr, "failed to get response\n");
		request_exit();
		return;
	}

	response[strcspn(response, "\n")] = 0;

	const bool do_delete = strncmp(response, "delete", sizeof(response)) == 0;
	const bool do_store = strncmp(response, "store", sizeof(response)) == 0;
	const bool do_quit = strncmp(response, "quit", sizeof(response)) == 0;

	if(!do_delete && !do_store && !do_quit)
	{
		fprintf(stderr, "unrecognized option: %s\n", response);
		request_exit();
		return;
	}

	if(do_quit)
	{
		request_exit();
		return;
	}

	fprintf(stdout, "confirm CID (%s): ", cid);
	fflush(stdout);

	memset(response, 0, sizeof(response));
	if(fgets(response, sizeof(response), stdin) == NULL)
	{
		fprintf(stderr, "failed to get response\n");
		request_exit();
		return;
	}

	response[strcspn(response, "\n")] = 0;

	// User failed to validate CID.
	if(strncmp(response, cid, sizeof(response)) != 0)
	{
		fprintf(stderr, "invalid CID: %s\n", response);
		request_exit();
		return;
	}

	// Delete the flat scene correction.
	if(do_delete)
	{
		status = seekcamera_delete_flat_scene_correction(
			camera,
			SEEKCAMERA_FLAT_SCENE_CORRECTION_ID_0,
			handle_delete_progress_update,
			NULL);

		if(status != SEEKCAMERA_SUCCESS)
		{
			fprintf(stderr, "failed to delete flat scene correction: %s\n", seekcamera_error_get_str(status));
			request_exit();
			return;
		}

		request_exit();
	}

	// Store the flat scene correction.
	if(do_store)
	{
		// Start the capture session.
		// The camera must be imaging to store a flat scene correction -- any frame format can be used.
		// An optional frame available callback may be registered apriori to allow visual diagnostics.
		fprintf(stdout, "starting capture session\n");
		fflush(stdout);

		status = seekcamera_capture_session_start(camera, SEEKCAMERA_FRAME_FORMAT_THERMOGRAPHY_FLOAT);
		if(status != SEEKCAMERA_SUCCESS)
		{
			fprintf(stderr, "failed to start capture session: %s\n", seekcamera_error_get_str(status));
			request_exit();
			return;
		}

		// Wait for the camera to warm up.
		for(int i = g_num_warmup_seconds; i >= 0 && g_keep_running; --i)
		{
			if(i == 0)
			{
				fprintf(stdout, "warming up.. %i seconds remaining  \n", i);
			}
			else
			{
				fprintf(stdout, "warming up.. %i seconds remaining  \r", i);
			}
			fflush(stdout);

			const int sleep_ms = 1000;
#ifdef _WIN32
			Sleep(sleep_ms);
#else
			usleep(sleep_ms * 1000);
#endif
		}

		// Verify the loop was not exited prematurely.
		if(!g_keep_running)
		{
			return;
		}

		// Store the flat scene correction.
		fprintf(stdout, "storing flat scene correction\n");
		fflush(stdout);

		status = seekcamera_store_flat_scene_correction(
			camera,
			SEEKCAMERA_FLAT_SCENE_CORRECTION_ID_0,
			handle_store_progress_update,
			NULL);

		if(status != SEEKCAMERA_SUCCESS)
		{
			fprintf(stderr, "failed to store flat scene correction: %s\n", seekcamera_error_get_str(status));
			request_exit();
			return;
		}

		// Stop the capture session.
		fprintf(stdout, "stopping capture session\n");
		status = seekcamera_capture_session_stop(camera);
		if(status != SEEKCAMERA_SUCCESS)
		{
			fprintf(stderr, "failed to stop capture session: %s\n", seekcamera_error_get_str(status));
			request_exit();
			return;
		}

		request_exit();
	}
}

// Callback function for the camera manager; it fires whenever a camera event occurs.
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
		default:
			break;
	}
}

// Application entry point
int main(int argc, char** argv)
{
	// Install signal handlers.
	signal(SIGINT, signal_callback);
	signal(SIGTERM, signal_callback);

	// Default values for the command line arguments.
	char* discovery_mode_str = "usb";
	uint32_t discovery_mode = (uint32_t)SEEKCAMERA_IO_TYPE_USB;
	g_num_warmup_seconds = 60;

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
				case 't':
					if(i < argc - 1)
					{
						g_num_warmup_seconds = atoi(argv[i + 1]);
						if(g_num_warmup_seconds <= 0)
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

	fprintf(stdout, "seekcamera-fsc starting\n");
	fprintf(stdout, "settings\n");
	fprintf(stdout, "\t1) mode (-m): %s\n", discovery_mode_str);
	fprintf(stdout, "\t2) warmup time (-t): %i\n", g_num_warmup_seconds);

	// Create the camera manager.
	// This is the structure that owns all Seek camera devices.
	seekcamera_manager_t* manager = NULL;
	seekcamera_error_t status = seekcamera_manager_create(&manager, discovery_mode);
	if(status != SEEKCAMERA_SUCCESS)
	{
		fprintf(stderr, "failed to create camera manager: %s\n", seekcamera_error_get_str(status));
		return 1;
	}

	fprintf(stdout, "discovering cameras\n");

	// Register an event handler for the camera manager.
	// The event handler will be called for every event on a per-camera basis.
	status = seekcamera_manager_register_event_callback(manager, camera_event_callback, NULL);
	if(status != SEEKCAMERA_SUCCESS)
	{
		fprintf(stderr, "failed to register camera event callback: %s\n", seekcamera_error_get_str(status));
		seekcamera_manager_destroy(&manager);
		return 1;
	}

	// Camera events are asynchronous and interrupt the current thread.
	// There are much better ways of keeping the main thread alive. This is
	// just a simple example.
	while(g_keep_running)
	{
		const int sleep_ms = 1000;
#ifdef _WIN32
		Sleep(sleep_ms);
#else
		usleep(sleep_ms * 1000);
#endif
	}

	// Cleanup the camera manager. Cameras will be disconnected and invalidated.
	status = seekcamera_manager_destroy(&manager);
	if(status != SEEKCAMERA_SUCCESS)
	{
		fprintf(stderr, "failed to free camera manager: %s\n", seekcamera_error_get_str(status));
		return 1;
	}

	fprintf(stdout, "done\n");

	return 0;
}
