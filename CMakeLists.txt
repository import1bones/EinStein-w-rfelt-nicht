cmake_minimum_required(VERSION 3.16)
project(EinStein_Game VERSION 2.0.0 LANGUAGES CXX)

# Default install prefix: project-local 'install' directory (avoid system locations)
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install path" FORCE)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG /Od /W4 /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
endif()

# Enable testing
enable_testing()

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Option to use nlohmann::json (try find_package first, then FetchContent fallback)
option(USE_NLOHMANN_JSON "Try to use nlohmann/json for JSON handling (find or fetch)" ON)
if(USE_NLOHMANN_JSON)
    # If repository already vendors a single-header at include/third_party/json.hpp, prefer it
    if(EXISTS "${CMAKE_SOURCE_DIR}/include/third_party/json.hpp")
        message(STATUS "Found vendored single-header json.hpp; using it")
        add_compile_definitions(USE_NLOHMANN_JSON=1)
        set(HAVE_NLOHMANN_JSON TRUE)
    else()
        include(FetchContent)
        # Prefer find_package if nlohmann_json is already installed on the system
        find_package(nlohmann_json QUIET)
        if(NOT nlohmann_json_FOUND)
            # Check network/git access before attempting a remote clone to avoid hard failure
            execute_process(COMMAND git ls-remote https://github.com/nlohmann/json.git HEAD
                            RESULT_VARIABLE GIT_LS_OK
                            TIMEOUT 10
                            OUTPUT_QUIET ERROR_QUIET)
            if(GIT_LS_OK EQUAL 0)
                message(STATUS "nlohmann_json not found via find_package; Fetching from github...")
                FetchContent_Declare(
                    nlohmann_json
                    GIT_REPOSITORY https://github.com/nlohmann/json.git
                    GIT_TAG v3.11.2
                )
                FetchContent_MakeAvailable(nlohmann_json)
            else()
                message(WARNING "nlohmann_json not found and remote repository unreachable; continuing without it")
            endif()
        endif()
        if(TARGET nlohmann_json::nlohmann_json)
            set(HAVE_NLOHMANN_JSON TRUE)
            message(STATUS "nlohmann::json available and will be linked into targets")
        else()
            message(STATUS "nlohmann::json is not available; continuing without it")
        endif()
    endif()
endif()

# nlohmann_json was previously attempted via FetchContent, but to avoid CMake
# compatibility issues we vendor a small JSON header in include/third_party.

# Try to find SDL2 - make it optional for now to allow partial builds
find_package(SDL2 QUIET)
if(SDL2_FOUND)
    find_package(SDL2_ttf QUIET)
    find_package(SDL2_image QUIET)
    set(GRAPHICS_AVAILABLE TRUE)
else()
    set(GRAPHICS_AVAILABLE FALSE)
    message(WARNING "SDL2 not found - building without graphics support")
endif()

# Try to find Python for neural network support
find_package(Python3 COMPONENTS Interpreter Development QUIET)
if(Python3_FOUND)
    # Try to find pybind11
    find_package(pybind11 QUIET)
    if(pybind11_FOUND)
        set(AI_AVAILABLE TRUE)
    else()
        set(AI_AVAILABLE FALSE)
        message(WARNING "pybind11 not found - building without neural network support")
    endif()
else()
    set(AI_AVAILABLE FALSE)
    message(WARNING "Python3 not found - building without neural network support")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

if(GRAPHICS_AVAILABLE)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

if(AI_AVAILABLE)
    include_directories(${Python3_INCLUDE_DIRS})
endif()

# Core source files (always available)
set(CORE_SOURCES
    src/main.cpp
    src/core/ChessBoard.cpp
    src/utils/Config.cpp
    src/utils/CLIRenderer.cpp
    src/utils/GameSnapshot.cpp
    src/utils/Logger.cpp
    src/ai/MCTS.cpp
)

set(CORE_HEADERS
    include/core/Types.h
    include/core/ChessBoard.h
    include/utils/Config.h
    include/utils/CLIRenderer.h
    include/utils/GameSnapshot.h
    include/utils/Logger.h
    include/ai/MCTS.h
)

# AI source files (optional)
set(AI_SOURCES
    src/ai/MCTS.cpp
)

set(AI_HEADERS
    include/ai/MCTS.h
    include/ai/NeuralNetwork.h
)

# Graphics source files (optional)
set(GRAPHICS_SOURCES
    # src/graphics/Renderer.cpp  # Temporarily disabled - needs SDL2
)

set(GRAPHICS_HEADERS
    include/graphics/Renderer.h
    include/graphics/InputHandler.h
)

# Game logic source files
set(GAME_SOURCES
    src/game/GameState.cpp
)

set(GAME_HEADERS
    include/game/GameState.h
    include/game/Game.h
)

# Build the main sources list
set(ALL_SOURCES ${CORE_SOURCES})
set(ALL_HEADERS ${CORE_HEADERS})

# Add optional components based on availability
if(AI_AVAILABLE)
    list(APPEND ALL_SOURCES ${AI_SOURCES})
    list(APPEND ALL_HEADERS ${AI_HEADERS})
    add_definitions(-DEINSTEIN_AI_AVAILABLE)
endif()

if(GRAPHICS_AVAILABLE)
    list(APPEND ALL_SOURCES ${GRAPHICS_SOURCES})
    list(APPEND ALL_HEADERS ${GRAPHICS_HEADERS})
    add_definitions(-DEINSTEIN_GRAPHICS_AVAILABLE)
endif()

# Always include game logic
list(APPEND ALL_SOURCES ${GAME_SOURCES})
list(APPEND ALL_HEADERS ${GAME_HEADERS})

# Create a static library for the core/game logic so tests and multiple executables can link to it.
# Exclude the application entry point (src/main.cpp) from the library.
set(LIB_SOURCES ${ALL_SOURCES})
list(REMOVE_ITEM LIB_SOURCES src/main.cpp)

add_library(einstein_lib STATIC ${LIB_SOURCES} ${ALL_HEADERS})
set_target_properties(einstein_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Link common libraries to the library target
target_link_libraries(einstein_lib PRIVATE Threads::Threads)

if(HAVE_NLOHMANN_JSON)
    if(TARGET nlohmann_json::nlohmann_json)
        target_link_libraries(einstein_lib PRIVATE nlohmann_json::nlohmann_json)
        target_compile_definitions(einstein_lib PRIVATE USE_NLOHMANN_JSON=1)
    else()
        # Vendored single-header mode: add include directory and define macro, no linking required.
        target_include_directories(einstein_lib PRIVATE ${CMAKE_SOURCE_DIR}/include/third_party)
        target_compile_definitions(einstein_lib PRIVATE USE_NLOHMANN_JSON=1)
    endif()
endif()

# Create main executable and link against the library
add_executable(${PROJECT_NAME} src/main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${PROJECT_NAME} PRIVATE einstein_lib)

if(GRAPHICS_AVAILABLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${SDL2_LIBRARIES}
    )
    if(SDL2_TTF_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})
    endif()
    if(SDL2_IMAGE_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_LIBRARIES})
    endif()
endif()

if(AI_AVAILABLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        pybind11::embed
        ${Python3_LIBRARIES}
    )
endif()

# Platform-specific linking
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE m dl)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation")
endif()

# Copy assets and configuration
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets)
configure_file(${CMAKE_SOURCE_DIR}/assets/config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)

# Copy ML models if they exist
if(EXISTS ${CMAKE_SOURCE_DIR}/ml_models/)
    file(COPY ${CMAKE_SOURCE_DIR}/ml_models/ DESTINATION ${CMAKE_BINARY_DIR}/ml_models/)
endif()

# Copy Python dependencies if AI is available
if(AI_AVAILABLE)
    configure_file(${CMAKE_SOURCE_DIR}/requirements.txt ${CMAKE_BINARY_DIR}/requirements.txt COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/ml_models/einstein_nn.py ${CMAKE_BINARY_DIR}/einstein_nn.py COPYONLY)
endif()

# Build tests
file(GLOB UNIT_TEST_SOURCES ${CMAKE_SOURCE_DIR}/tests/unit/*.cpp)
add_executable(${PROJECT_NAME}_tests tests/test_main.cpp ${UNIT_TEST_SOURCES})
set_target_properties(${PROJECT_NAME}_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${PROJECT_NAME}_tests PRIVATE einstein_lib vmodel_framework)
target_include_directories(${PROJECT_NAME}_tests PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

# Build V-Model test framework as a reusable library so unit tests can link to it
add_library(vmodel_framework STATIC tests/framework/VModelTestFramework.cpp)
set_target_properties(vmodel_framework PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(vmodel_framework PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

# Build V-Model test runner executable and link with the framework and core library
add_executable(${PROJECT_NAME}_vmodel_tests
    tests/SimpleVModelRunner.cpp
)
set_target_properties(${PROJECT_NAME}_vmodel_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${PROJECT_NAME}_vmodel_tests PRIVATE einstein_lib vmodel_framework)
target_include_directories(${PROJECT_NAME}_vmodel_tests PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

# Register tests with CTest
add_test(NAME unit_tests COMMAND ${PROJECT_NAME}_tests)
add_test(NAME vmodel_unit_tests COMMAND ${PROJECT_NAME}_vmodel_tests --unit)
add_test(NAME vmodel_integration_tests COMMAND ${PROJECT_NAME}_vmodel_tests --integration)
add_test(NAME vmodel_system_tests COMMAND ${PROJECT_NAME}_vmodel_tests --system)
add_test(NAME vmodel_acceptance_tests COMMAND ${PROJECT_NAME}_vmodel_tests --acceptance)
add_test(NAME vmodel_all_tests COMMAND ${PROJECT_NAME}_vmodel_tests)

# Install targets
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES assets/config.json DESTINATION bin)

if(EXISTS ${CMAKE_SOURCE_DIR}/ml_models/)
    install(DIRECTORY ml_models/ DESTINATION bin/ml_models/)
endif()

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "EinStein Game")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern cross-platform Einstein würfelt nicht game with AI")
set(CPACK_PACKAGE_VENDOR "Einstein Game Project")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack)

# Print configuration summary
message(STATUS "=== Einstein Game Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Graphics support (SDL2): ${GRAPHICS_AVAILABLE}")
message(STATUS "AI support (Python/pybind11): ${AI_AVAILABLE}")
message(STATUS "Threading support: ON")
message(STATUS "Testing support: ON")
message(STATUS "==========================================")
# Optional tools: reserialize_snapshots (only available when nlohmann::json is present)
if(HAVE_NLOHMANN_JSON)
    add_executable(reserialize_snapshots src/tools/reserialize_snapshots.cpp)
    # If an imported target exists, link it; otherwise rely on vendored header include
    if(TARGET nlohmann_json::nlohmann_json)
        target_link_libraries(reserialize_snapshots PRIVATE nlohmann_json::nlohmann_json Threads::Threads)
    else()
        target_include_directories(reserialize_snapshots PRIVATE ${CMAKE_SOURCE_DIR}/include/third_party)
        target_link_libraries(reserialize_snapshots PRIVATE Threads::Threads)
    endif()
    set_target_properties(reserialize_snapshots PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
    install(TARGETS reserialize_snapshots DESTINATION bin)
else()
    message(STATUS "Reserialize tool not available: nlohmann::json not found")
endif()
# 自动复制 compile_commands.json 到项目根目录
## The project previously copied compile_commands.json to the source root after build
## but this can fail in some environments (file locks/permissions). If you need
## compile_commands.json in the repo root, run:
##   cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && cp build/compile_commands.json .

