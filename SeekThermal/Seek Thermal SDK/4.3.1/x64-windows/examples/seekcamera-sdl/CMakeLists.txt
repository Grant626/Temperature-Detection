#--------------------------------------------------------------------------------------------------------------------------#
#Project configuration
#--------------------------------------------------------------------------------------------------------------------------#
project(seekcamera-sdl DESCRIPTION "Seek Thermal SDK - SDL Sample")

#--------------------------------------------------------------------------------------------------------------------------#
#Executable configuration
#--------------------------------------------------------------------------------------------------------------------------#
add_executable(${PROJECT_NAME}
	src/seekcamera-sdl.cpp
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(SDL2_FOUND)
	include_directories(${SDL2_EXEC_PREFIX}/include)
endif(SDL2_FOUND)

target_link_libraries(${PROJECT_NAME}
	seekcamera
	SDL2
)

if(SDL2_FOUND)
	target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()

if(UNIX)
	target_link_libraries(${PROJECT_NAME} m)
endif()

#--------------------------------------------------------------------------------------------------------------------------#
#Runtime dependencies
#--------------------------------------------------------------------------------------------------------------------------#
if(MSVC)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:seekcamera>/seekcamera.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/seekcamera.dll
	)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:SDL2>/SDL2.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll
	)
endif()

#--------------------------------------------------------------------------------------------------------------------------#
#Install
#--------------------------------------------------------------------------------------------------------------------------#
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
