include(ExternalProject)

cmake_minimum_required(VERSION 3.10)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

project(Project seekcamera_examples)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
	set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
else()
	include(GNUInstallDirs)
	if(APPLE)
		set(CMAKE_OSX_DEPLOYMENT_TARGET 10.8)
	endif()
endif()

#Configure for Release/Debug
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	if(NOT MSVC)
		add_compile_options(-O3 -fPIC -fno-builtin -fno-exceptions -ftree-vectorize -fomit-frame-pointer ${CFLAGS})
		add_link_options( -s ${LDFLAGS} -Wl,--as-needed )
	else()
		add_compile_options(/W4)
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(NOT MSVC)
		add_compile_options(-O0 -g3 -fPIC -Wall -Wextra -pedantic -Wshadow -Wabi ${CFLAGS})
		add_link_options( ${LDFLAGS} )
	else()
		add_compile_options(/W4)
	endif()
endif()

#Download Dependencies
if(MSVC)
	#SDL2
	ExternalProject_Add(SDL
		PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/depends
		URL https://www.libsdl.org/release/SDL2-devel-2.0.10-VC.zip
		URL_HASH SHA256=F1930CF5033725B0E7829C7368F70F65A993BC946CA1197C36888CCECBF07891
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	add_library(SDL2 SHARED IMPORTED)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set_target_properties(SDL2 PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/lib/x64/SDL2.lib)
		set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/lib/x64/SDL2.dll)
	endif()
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set_target_properties(SDL2 PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/lib/x86/SDL2.lib)
		set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/lib/x86/SDL2.dll)
	endif()
	set(SDL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/include)
	file(MAKE_DIRECTORY ${SDL_INCLUDE})
	target_include_directories(SDL2 INTERFACE ${SDL_INCLUDE})
endif()

#seekcamera
if(NOT TARGET seekcamera)
	if(MSVC)
		add_library(seekcamera SHARED IMPORTED)
		target_include_directories(seekcamera INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
		set_target_properties(seekcamera PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../lib/seekcamera.dll)
		set_target_properties(seekcamera PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/../lib/seekcamera.lib)
	else()
		add_library(seekcamera SHARED IMPORTED)
		target_include_directories(seekcamera INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
		set_target_properties(seekcamera PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libseekcamera.so)
	endif()
endif()

#seekcamera-cal
if(NOT TARGET seekcamera-cal AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/seekcamera-cal)
	add_subdirectory(seekcamera-cal)
endif()

#seekcamera-fsc
if(NOT TARGET seekcamera-fsc AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/seekcamera-fsc)
	add_subdirectory(seekcamera-fsc)
endif()

#seekcamera-fw-update
if(NOT TARGET seekcamera-fw-update AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/seekcamera-fw-update)
	add_subdirectory(seekcamera-fw-update)
endif()

#seekcamera-probe
if(NOT TARGET seekcamera-probe AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/seekcamera-probe)
	add_subdirectory(seekcamera-probe)
endif()

#seekcamera-sdl
if(NOT TARGET seekcamera-sdl AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/seekcamera-sdl)
	add_subdirectory(seekcamera-sdl)
	if(MSVC AND ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.6.0")
		set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT seekcamera-sdl)
	endif()
endif()

#seekcamera-simple
if(NOT TARGET seekcamera-simple AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/seekcamera-simple)
	add_subdirectory(seekcamera-simple)
endif()

#seekcamera-frame-formats
if(NOT TARGET seekcamera-frame-formats AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/seekcamera-frame-formats)
	add_subdirectory(seekcamera-frame-formats)
endif()
