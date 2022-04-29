# ====================================
# Setup Hunter package manager
# ====================================

cmake_minimum_required(VERSION 3.2) # for Hunter
set(CMAKE_POLICY_DEFAULT_CMP0126 OLD) # for Hunter # New in version 3.21

option(HUNTER_NO_TOOLCHAIN_ID_RECALCULATION "No Toolchain-ID recalculation" OFF)

include("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.24.1.tar.gz"
    SHA1 "4942227a6e6f5e64414c55b97ef98609de199d18"
    LOCAL # load cmake/Hunter/config.cmake
)

# ====================================

project(WebServer)

# ====================================
# Setup CrowCpp web server library
# ====================================

hunter_add_package(Boost COMPONENTS system date_time)
message(STATUS "HUNTER_Boost_VERSION: ${HUNTER_Boost_VERSION}")

add_subdirectory(dependency/CrowCpp)

# ====================================

set(SOURCES
    main.cpp
)

file(GLOB CMAKE_FILES
     "CMakeLists.txt"
     "cmake/*.cmake"
     "cmake/Hunter/*.cmake"
)

file(GLOB CROWCPP_FILES
     "dependency/CrowCpp/LICENSE"
     "dependency/CrowCpp/VERSION"
     "dependency/CrowCpp/*.md"
     "dependency/CrowCpp/*.txt"
)

file(GLOB_RECURSE CROWCPP_HEADERS
     "dependency/CrowCpp/*.h"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CMAKE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/dependency/CrowCpp PREFIX CrowCpp FILES ${CROWCPP_HEADERS} ${CROWCPP_FILES})

add_executable(WebServer ${SOURCES} ${CMAKE_FILES} ${CROWCPP_HEADERS} ${CROWCPP_FILES})

target_link_libraries(WebServer	PRIVATE Crow)

# ====================================
