cmake_minimum_required(VERSION 3.19)
# Requirements: 3.2 for Hunter; 3.15 for MSVC_RUNTIME_LIBRARY; 3.19 for less dependencies (CMP0112)
# CMP0112: Now target file component generator expressions do not add target dependencies. # New in version 3.19.

# ====================================

set(CMAKE_MESSAGE_LOG_LEVEL         "VERBOSE"          CACHE STRING "Log verbosity (STATUS | VERBOSE | DEBUG | TRACE)")

set(DEPS_REQUIRED_BOOST_VERSION     "1.78.0"           CACHE STRING "Required package version")
set(DEPS_REQUIRED_RAPIDJSON_VERSION "1.1.0-b557259-p0" CACHE STRING "Required package version")

# ====================================

# Set required C++ standard
set(CMAKE_CXX_STANDARD 17)  # C++17 for `std::optional` & etc
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# ====================================
# Setup Hunter package manager
# ====================================

include("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.24.2.tar.gz"
    SHA1 "17fd3b9f0b8c271eac9b71e30a6aa1b9fecab299"
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
# === WebServer application
# ====================================

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
# === Client application
# ====================================

set(CLIENT_BASENAME Client)
set(CLIENT_EXECUTABLE ${CLIENT_BASENAME})
if (WIN32)
    set(CLIENT_EXECUTABLE ${CLIENT_BASENAME}.exe)
endif()

add_custom_target(WebClient ALL
    COMMENT "Compile Python script to executable file"
    COMMAND python --version
    COMMAND pyinstaller --version
    COMMAND pyinstaller
        --distpath="${CMAKE_CURRENT_BINARY_DIR}"
        --name="${CLIENT_BASENAME}"
        --workpath="${CMAKE_CURRENT_BINARY_DIR}/Client.dir/pyinstaller_work"
        --specpath="${CMAKE_CURRENT_BINARY_DIR}/Client.dir/pyinstaller_spec"
        --noconfirm
        --onefile
        --console
        --noupx
        -- "${CMAKE_CURRENT_SOURCE_DIR}/client.py"
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/client.py
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${CLIENT_EXECUTABLE}
    )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CLIENT_EXECUTABLE} DESTINATION .)

install(FILES client.py DESTINATION .)

# ====================================
