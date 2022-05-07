cmake_minimum_required(VERSION 3.15) # 3.2 for Hunter; 3.15 for MSVC_RUNTIME_LIBRARY
set(CMAKE_POLICY_DEFAULT_CMP0126 OLD) # for Hunter # New in version 3.21

# ====================================

set(CMAKE_MESSAGE_LOG_LEVEL         "VERBOSE"          CACHE STRING "Log verbosity (STATUS | VERBOSE | DEBUG | TRACE)")

set(DEPS_REQUIRED_BOOST_VERSION     "1.78.0"           CACHE STRING "Required package version")
set(DEPS_REQUIRED_RAPIDJSON_VERSION "1.1.0-b557259-p0" CACHE STRING "Required package version")

# ====================================
# Setup Hunter package manager
# ====================================

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
# Setup RapidJSON library
# ====================================

hunter_add_package(RapidJSON)
message(STATUS "HUNTER_RapidJSON_VERSION: ${HUNTER_RapidJSON_VERSION}")

cmake_policy(SET CMP0074 NEW) # for RapidJSON
find_package(RapidJSON CONFIG REQUIRED)
message(VERBOSE "RapidJSON_VERSION: ${RapidJSON_VERSION}")

# ====================================

# Set required C++ standard
set(CMAKE_CXX_STANDARD 20) # C++17 for `std::optional` & etc; c++20 for `std::string_view` in `std::unordered_map::find`
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(SOURCES
    main.cpp
    BackgroundWorker.cpp
    DataEngine.cpp
    DataSerializer.cpp
    Logger.cpp
    HttpServer.cpp
    HttpServerHelpers.cpp
    StatisticsPrinter.cpp
)

set(HEADERS
    BackgroundWorker.h
    DataEngine.h
    DataSerializer.h
    Logger.h
    HttpServer.h
    HttpServerHelpers.h
    StatisticsPrinter.h
)

set(UTILS_HEADERS
    utils/stdlib.h
    utils/stl.h
)

set(CMAKE_FILES
    CMakeLists.txt
    cmake/HunterGate.cmake
    cmake/Hunter/config.cmake
)

set(CROWCPP_FILES
    dependency/CrowCpp/CMakeLists.txt
    dependency/CrowCpp/LICENSE
    dependency/CrowCpp/README.md
    dependency/CrowCpp/VERSION
)

file(GLOB_RECURSE CROWCPP_HEADERS
    dependency/CrowCpp/*.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CMAKE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/dependency/CrowCpp PREFIX CrowCpp FILES ${CROWCPP_HEADERS} ${CROWCPP_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/utils PREFIX utils FILES ${UTILS_HEADERS})

add_executable(WebServer ${SOURCES} ${HEADERS} ${UTILS_HEADERS} ${CMAKE_FILES} ${CROWCPP_HEADERS} ${CROWCPP_FILES})

target_include_directories(WebServer PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

target_link_libraries(WebServer PRIVATE Crow)

# Link to CRT statically in MSVC:
set_property(TARGET WebServer PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Added to simplify detection build artifacts by CI
install(TARGETS WebServer RUNTIME DESTINATION .)

# ====================================
