cmake_minimum_required( VERSION 3.20 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( house-dancer )

set( VERSION_MAJOR "25" )
set( VERSION_MINOR "07" )
set( VERSION_PATCH "01" )

option(ENABLE_VIDEO "Show video" ON)
set(CMAKE_C_COMPILER /usr/bin/gcc-11 CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER /usr/bin/g++-11 CACHE PATH "" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g -std=c++20")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++20")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")

set( CMAKE_BUILD_TYPE Debug )
#set( CMAKE_BUILD_TYPE Release )

get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../work/Cinder" ABSOLUTE )
get_filename_component( ARG_ASSETS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../assets" ABSOLUTE )

set(cinder_DIR ${CINDER_PATH}/lib/linux/x86_64/ogl/Release)
include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

set(SRC_FILES
	src/HouseDancerApp.cpp
	src/SavitzkyGolayFilter.h
	src/SavitzkyGolayFilter.cpp
	#include/Resources.h
)

set(RESOURCE_FILES
	#resources/Resources.rc
)

function(makeGroups fileList)
	foreach(FILE ${fileList}) 
	    # Get the directory of the source file
	    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
	    # Remove common directory prefix to make the group
	    string(REPLACE "${APP_PATH}" "" GROUP "${PARENT_DIR}")
	    # Make sure we are using windows slashes
	    string(REPLACE "/" "\\" GROUP "${GROUP}")
	    source_group("${GROUP}" FILES "${FILE}")
	endforeach()
endfunction()

makeGroups("${SRC_FILES}")
makeGroups("${RESOURCE_FILES}")

set(INC_PATHS
	include
	src
	third_party/spdlog/include
)

set(BLOCK_FILES
	blocks/Cinder-Link
)
if( WIN32 )
	set( BLOCK_FILES
	    ${BLOCK_FILES}
		blocks/Cinder-KCB2
	)	
endif( WIN32 )

set(LIB_FILES
    #${KINECTSDK20_DIR}/lib/${PlatformTarget}/
    #KCBv2.lib
    #"C:/Program Files/Microsoft SDKs/Kinect/v2.0_1409/lib/x64/kinect20.lib"
    #"C:/Program Files/Microsoft SDKs/Kinect/v2.0_1409/lib/x64/Kinect20.Face.lib"
)

ci_make_app(
    INCLUDES    ${INC_PATHS}
	SOURCES     ${SRC_FILES}
	LIBRARIES   ${LIB_FILES} 
	BLOCKS      ${BLOCK_FILES}
	RESOURCES   ${RESOURCE_FILES}
	CINDER_PATH ${CINDER_PATH}
)

#!!! Why do we need to do this???
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" )
set_property(TARGET Cinder-KCB2 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" )
set_property(TARGET Cinder-Link PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" )

if( ${BUILD_INSTALLER} )
	include( InstallRequiredSystemLibraries )
	set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "House Dancer")
	set( CPACK_PACKAGE_VENDOR "Clint" )
	set( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../../README.md" )
	#set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt" )
	set( CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR} )
	set( CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR} )
	set( CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH} )
	set( CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}" )
	if( WIN32 AND NOT UNIX )
		# There is a bug in NSI that does not handle full UNIX paths properly.
		# Make sure there is at least one set of four backlashes.
		set( CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
		set( CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\house-dancer.exe")
		set( CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} house dancer")
		set( CPACK_NSIS_HELP_LINK "https://github.com/clinthidinger/HouseDancer")
		set( CPACK_NSIS_URL_INFO_ABOUT "https://github.com/clinthidinger/HouseDancer")
		set( CPACK_NSIS_CONTACT "clint@somejunk.com" )
		set( CPACK_NSIS_MODIFY_PATH ON )
	else()
		#set(CPACK_STRIP_FILES "${CMAKE_CURRENT_BINARY_DIR}/Debug/${PROJECT_NAME}/${PROJECT_NAME}")
  		#set(CPACK_SOURCE_STRIP_FILES "")
	endif()
	set( CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_NAME} )

	set( CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME} )
	#install( TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin/${PROJECT_NAME} )
	#set( CPACK_GENERATOR "DEB" )
	#set( CPACK_DEBIAN_PACKAGE_MAINTAINER "Clint Hidinger" ) # required
	include( CPack )

endif( ${BUILD_INSTALLER} )
