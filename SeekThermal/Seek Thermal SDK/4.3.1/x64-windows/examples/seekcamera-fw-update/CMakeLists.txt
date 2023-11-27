#--------------------------------------------------------------------------------------------------------------------------#
#Project configuration
#--------------------------------------------------------------------------------------------------------------------------#
project(seekcamera-fw-update DESCRIPTION "Seek Thermal SDK - Firmware Update Sample")

#--------------------------------------------------------------------------------------------------------------------------#
#Executable configuration
#--------------------------------------------------------------------------------------------------------------------------#
add_executable(${PROJECT_NAME}
	src/seekcamera-fw-update.c
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
	seekcamera
)

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
endif()

#--------------------------------------------------------------------------------------------------------------------------#
#Install
#--------------------------------------------------------------------------------------------------------------------------#
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
