cmake_minimum_required(VERSION 3.5)
project(Link)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#   ___        _   _
#  / _ \ _ __ | |_(_) ___  _ __  ___
# | | | | '_ \| __| |/ _ \| '_ \/ __|
# | |_| | |_) | |_| | (_) | | | \__ \
#  \___/| .__/ \__|_|\___/|_| |_|___/
#       |_|

# Note: Please use the LINK_* prefix for all project-specific options

if(UNIX)
  option(LINK_ENABLE_ASAN "Build with Address Sanitizier (ASan)" OFF)
  option(LINK_BUILD_JACK "Build example applications with JACK support" OFF)
endif()

if(WIN32)
  option(LINK_BUILD_ASIO "Build example applications with ASIO driver" ON)
  option(LINK_BUILD_VLD "Build with VLD support (VLD must be installed separately)" OFF)
endif()

option(LINK_BUILD_TESTS "Build unit test binaries" OFF)

#  ____       _   _
# |  _ \ __ _| |_| |__  ___
# | |_) / _` | __| '_ \/ __|
# |  __/ (_| | |_| | | \__ \
# |_|   \__,_|\__|_| |_|___/
#

# Other CMake files must be included only after declaring build options
include(cmake_include/ConfigureCompileFlags.cmake)
include(cmake_include/CatchConfig.cmake)
include(AbletonLinkConfig.cmake)
include(extensions/abl_link/abl_link.cmake)

add_subdirectory(include)
if(LINK_BUILD_TESTS)
add_subdirectory(src)
endif()
add_subdirectory(examples)
add_subdirectory(extensions/abl_link)

#  ____
# / ___| _   _ _ __ ___  _ __ ___   __ _ _ __ _   _
# \___ \| | | | '_ ` _ \| '_ ` _ \ / _` | '__| | | |
#  ___) | |_| | | | | | | | | | | | (_| | |  | |_| |
# |____/ \__,_|_| |_| |_|_| |_| |_|\__,_|_|   \__, |
#                                             |___/

message(STATUS "Build options")

get_cmake_property(all_variables VARIABLES)
string(REGEX MATCHALL "(^|;)LINK_[A-Z_]+" link_variables "${all_variables}")
foreach(variable ${link_variables})
  message("   ${variable}: ${${variable}}")
endforeach()

message(STATUS "Build configuration")

if(CMAKE_BUILD_TYPE)
  message("   Build type: ${CMAKE_BUILD_TYPE}")
else()
  message("   Build type: Set by IDE")
endif()

