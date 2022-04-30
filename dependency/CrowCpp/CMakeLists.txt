#####################################
# Define Project-Wide Settings
#####################################
cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

# Define the project name and language
project(Crow
	LANGUAGES CXX
)

# Set required C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Default to build type "Release" unless tests are being built
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

#####################################
# Define Options
#####################################
option(CROW_ENABLE_SSL         "Enable SSL capabilities (OpenSSL)"      OFF)
option(CROW_ENABLE_COMPRESSION "Enable compression capabilities (ZLIB)" OFF)

#####################################
# Define Targets
#####################################
add_library(Crow INTERFACE)
add_library(Crow::Crow ALIAS Crow)

target_include_directories(Crow
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

find_package(Boost 1.64 COMPONENTS system date_time REQUIRED)
message(VERBOSE "Boost_VERSION: ${Boost_VERSION}")

find_package(Threads REQUIRED)

target_link_libraries(Crow
	INTERFACE
		Boost::boost Boost::system Boost::date_time
		Threads::Threads
)

if(CROW_ENABLE_COMPRESSION)
	find_package(ZLIB REQUIRED)
	target_link_libraries(Crow INTERFACE ZLIB::ZLIB)
	target_compile_definitions(Crow INTERFACE CROW_ENABLE_COMPRESSION)
endif()

if(CROW_ENABLE_SSL)
	find_package(OpenSSL REQUIRED)
	target_link_libraries(Crow INTERFACE OpenSSL::SSL)
	target_compile_definitions(Crow INTERFACE CROW_ENABLE_SSL)
endif()

#####################################
