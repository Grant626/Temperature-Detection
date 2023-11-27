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
#include <csignal>
#include <cstdio>
#include <cstring>

// C++ includes
#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// Seek SDK includes
#include "seekcamera/seekcamera.h"
#include "seekcamera/seekcamera_manager.h"
#include "seekcamera/seekcamera_version.h"

// Structure representing a camera probing interface.
struct seekprober_t
{
	// Structure representing a camera probe metric.
	struct metric_t
	{
		seekcamera_chipid_t cid;
		seekcamera_serial_number_t sn;
		seekcamera_core_part_number_t cpn;
		seekcamera_firmware_version_t fw;
		seekcamera_io_properties_t io;
	};

	std::mutex metrics_mutex;
	std::vector<metric_t> metrics;
};

// Gathers camera info and stores it as a probe metric.
void seekprober_add_metric(seekprober_t* prober, seekcamera_t* camera)
{
	// Enter critical section.
	std::lock_guard<std::mutex> lock(prober->metrics_mutex);

	// Gather the probe metric.
	seekprober_t::metric_t metric{};

	// Fill the chip ID.
	seekcamera_error_t status = seekcamera_get_chipid(camera, &(metric.cid));
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to get CID: " << seekcamera_error_get_str(status) << std::endl;
	}

	// Fill the serial number.
	status = seekcamera_get_serial_number(camera, &(metric.sn));
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to get SN: " << seekcamera_error_get_str(status) << std::endl;
	}

	// Fill the core part number.
	status = seekcamera_get_core_part_number(camera, &(metric.cpn));
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to get CPN: " << seekcamera_error_get_str(status) << std::endl;
	}

	// Fill the firmware version.
	status = seekcamera_get_firmware_version(camera, &(metric.fw));
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to get FW: " << seekcamera_error_get_str(status) << std::endl;
	}

	// Fill the IO properties.
	status = seekcamera_get_io_properties(camera, &(metric.io));
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to get IO properties: " << seekcamera_error_get_str(status) << std::endl;
	}

	// Store the metric.
	// It is acceptable for one of the field gets to have failed.
	prober->metrics.push_back(metric);
}

// Prints the camera metrics to the console.
void seekprober_print_metrics(seekprober_t* prober)
{
	// Enter critical section.
	std::lock_guard<std::mutex> lock(prober->metrics_mutex);

	// Lambda for printing the table separator line.
	// The line length is computed offline to account for the longest possible line length.
	auto print_table_separator_line = []() {
		std::fprintf(stdout, "+-------------------------------------------------------------------------+\n");
	};

	// Lambda for printing the table content line.
	// The cell spacing is computed offline to account for the longest possible cell length.
	auto print_table_content_line = [](const char* cid, const char* sn, const char* cpn, const char* fw, const char* io) {
		std::fprintf(stdout, "| %-12s | %-12s | %-8s | %-11s | %-16s |\n", cid, sn, cpn, fw, io);
	};

	// Lambda for printing the SDK version header line.
	// The cell spacing is computed offline to account for the longest possible cell length.
	auto print_version_header_line = []() {
		const uint32_t major = seekcamera_version_get_major();
		const uint32_t minor = seekcamera_version_get_minor();
		const uint32_t patch = seekcamera_version_get_patch();
		const uint32_t internal = seekcamera_version_get_internal();
		std::fprintf(stdout, "| Seek Thermal SDK: %u.%u.%u.%-47u |\n", major, minor, patch, internal);
	};

	// Print the table header.
	print_table_separator_line();
	print_version_header_line();
	print_table_separator_line();
	print_table_content_line("CID", "SN", "CPN", "FW", "IO");
	print_table_separator_line();

	// Sort by CID in ascending lexicographical order.
	// This ensures that camera information is always printed in the same order between runs.
	std::sort(prober->metrics.begin(), prober->metrics.end(), [](const seekprober_t::metric_t& lhs, const seekprober_t::metric_t& rhs) {
		return std::string(lhs.cid) < std::string(rhs.cid);
	});

	// Print probe metrics.
	for(auto& metric : prober->metrics)
	{
		std::stringstream fw_stream;
		fw_stream
			<< static_cast<int>(metric.fw.product) << "."
			<< static_cast<int>(metric.fw.variant) << "."
			<< static_cast<int>(metric.fw.major) << "."
			<< static_cast<int>(metric.fw.minor);

		switch(metric.io.type)
		{
			case SEEKCAMERA_IO_TYPE_USB:
			{
				// Print table content.
				print_table_content_line(metric.cid, metric.sn, metric.cpn, fw_stream.str().c_str(), "USB");

				// Print USB bus number.
				std::stringstream bus_stream;
				bus_stream << "BUS: " << static_cast<int>(metric.io.properties.usb.bus_number);
				print_table_content_line("", "", "", "", bus_stream.str().c_str());

				// Print USB port numbers.
				// Zero (0) is reserved for invalid ports; any non-zero ports are valid.
				std::stringstream port_stream;
				port_stream << "PORTS: ";
				for(uint8_t port_number : metric.io.properties.usb.port_numbers)
				{
					if(port_number != 0)
					{
						port_stream << static_cast<int>(port_number) << ' ';
					}
				}
				print_table_content_line("", "", "", "", port_stream.str().c_str());

				break;
			}
			case SEEKCAMERA_IO_TYPE_SPI:
			{
				// Print table content.
				print_table_content_line(metric.cid, metric.sn, metric.cpn, fw_stream.str().c_str(), "SPI");

				// Print SPI bus number.
				std::stringstream bus_stream;
				bus_stream << "BUS: " << static_cast<int>(metric.io.properties.spi.bus_number);
				print_table_content_line("", "", "", "", bus_stream.str().c_str());

				// Print SPI chip select number.
				std::stringstream cs_stream;
				cs_stream << "CHIPSELECT: " << static_cast<int>(metric.io.properties.spi.cs_number);
				print_table_content_line("", "", "", "", cs_stream.str().c_str());

				break;
			}
			default:
			{
				// Print table content.
				// This case should never happen since the IO type would have to be unknown.
				print_table_content_line(metric.cid, metric.sn, metric.cpn, fw_stream.str().c_str(), "N/A");
				break;
			}
		}

		// Print the separator between this line and the next.
		print_table_separator_line();
	}
	std::fprintf(stdout, "\n\n\n\nWindow will close in 30 seconds\n");
}

// Signal handler function.
static void signal_callback(int signum)
{
	(void)signum;
	std::cout << "\ncaught ctrl+c\n"
			  << std::endl;
}

// Prints the usage instructions.
void print_usage()
{
	std::cout
		<< "Allowed options:\n"
		<< "\t-m : Discovery mode. Valid options: usb, spi, all (default: usb)\n"
		<< "\t   : Required - No\n"
		<< "\t-d : Probe duration in seconds. Valid options: > 0 (default: 1)\n"
		<< "\t   : Required - No\n"
		<< "\t-h : Displays this message\n"
		<< "\t   : Required - No"
		<< std::endl;
}

// Handles camera connect events.
void handle_camera_connect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)event_status;
	(void)user_data;

	auto* prober = (seekprober_t*)user_data;
	seekprober_add_metric(prober, camera);
}

// Callback function for the camera manager; it fires whenever a camera event occurs.
void camera_event_callback(seekcamera_t* camera, seekcamera_manager_event_t event, seekcamera_error_t event_status, void* user_data)
{
	// Handle the event type.
	switch(event)
	{
		case SEEKCAMERA_MANAGER_EVENT_CONNECT:
		case SEEKCAMERA_MANAGER_EVENT_READY_TO_PAIR:
			handle_camera_connect(camera, event_status, user_data);
			break;
		case SEEKCAMERA_MANAGER_EVENT_DISCONNECT:
		case SEEKCAMERA_MANAGER_EVENT_ERROR:
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
	std::string discovery_mode_str("usb");
	uint32_t discovery_mode = static_cast<uint32_t>(SEEKCAMERA_IO_TYPE_USB);
	int duration_sec = 1;

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
						discovery_mode_str = std::string(argv[i + 1]);
						if(discovery_mode_str == "usb")
						{
							discovery_mode = static_cast<uint32_t>(SEEKCAMERA_IO_TYPE_USB);
						}
						else if(discovery_mode_str == "spi")
						{
							discovery_mode = static_cast<uint32_t>(SEEKCAMERA_IO_TYPE_SPI);
						}
						else if(discovery_mode_str == "all")
						{
							discovery_mode = static_cast<uint32_t>(SEEKCAMERA_IO_TYPE_USB) | static_cast<uint32_t>(SEEKCAMERA_IO_TYPE_SPI);
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
				case 'd':
					if(i < argc - 1)
					{
						duration_sec = std::stoi(argv[i + 1]);
						if(duration_sec <= 0)
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

	// Print the current settings.
	std::cout
		<< "seekcamera-probe starting\n"
		<< "settings:\n"
		<< "\t1) mode (-m):          " << discovery_mode_str << '\n'
		<< "\t2) duration (-d):      " << duration_sec << std::endl;

	// Create the camera manager.
	// This is the structure that owns all Seek camera devices.
	seekcamera_manager_t* manager = nullptr;
	seekcamera_error_t status = seekcamera_manager_create(&manager, discovery_mode);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to create camera manager: " << seekcamera_error_get_str(status) << std::endl;
		return 1;
	}

	// Create the prober.
	seekprober_t prober;

	// Register an event handler for the camera manager to be called whenever a camera event occurs.
	status = seekcamera_manager_register_event_callback(manager, camera_event_callback, (void*)&prober);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to register camera event callback: %s" << std::endl;
		return 1;
	}

	// Wait for cameras to be probed.
	std::this_thread::sleep_for(std::chrono::seconds(duration_sec));

	// Teardown the camera manager.
	seekcamera_manager_destroy(&manager);

	// Print the metrics to the console.
	seekprober_print_metrics(&prober);

	// Wait for cameras to be probed.
	std::this_thread::sleep_for(std::chrono::seconds(duration_sec*30));

	return 0;
}
