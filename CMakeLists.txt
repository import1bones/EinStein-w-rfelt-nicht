cmake_minimum_required(VERSION 3.16)
project(EinStein_Game VERSION 2.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Enable testing
enable_testing()

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Find required packages
find_package(Threads REQUIRED)

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
)

set(CORE_HEADERS
    include/core/Types.h
    include/core/ChessBoard.h
    include/utils/Config.h
)

# AI source files (optional)
set(AI_SOURCES
    src/ai/MCTS.cpp
    src/ai/NeuralNetwork.cpp
)

set(AI_HEADERS
    include/ai/MCTS.h
    include/ai/NeuralNetwork.h
)

# Graphics source files (optional)
set(GRAPHICS_SOURCES
    src/graphics/Renderer.cpp
    src/graphics/InputHandler.cpp
)

set(GRAPHICS_HEADERS
    include/graphics/Renderer.h
    include/graphics/InputHandler.h
)

# Game logic source files
set(GAME_SOURCES
    src/game/GameState.cpp
    src/game/Game.cpp
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

# Create main executable
add_executable(${PROJECT_NAME} ${ALL_SOURCES} ${ALL_HEADERS})

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    Threads::Threads
)

if(GRAPHICS_AVAILABLE)
    target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
    )
    if(SDL2_TTF_FOUND)
        target_link_libraries(${PROJECT_NAME} ${SDL2_TTF_LIBRARIES})
    endif()
    if(SDL2_IMAGE_FOUND)
        target_link_libraries(${PROJECT_NAME} ${SDL2_IMAGE_LIBRARIES})
    endif()
endif()

if(AI_AVAILABLE)
    target_link_libraries(${PROJECT_NAME}
        pybind11::embed
        ${Python3_LIBRARIES}
    )
endif()

# Platform-specific linking
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} m dl)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreFoundation")
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
add_executable(${PROJECT_NAME}_tests tests/test_main.cpp)
set_target_properties(${PROJECT_NAME}_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${PROJECT_NAME}_tests Threads::Threads)
target_include_directories(${PROJECT_NAME}_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Build V-Model test framework
add_executable(${PROJECT_NAME}_vmodel_tests 
    tests/VModelTestRunner.cpp
    tests/framework/VModelTestFramework.cpp
)

set_target_properties(${PROJECT_NAME}_vmodel_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${PROJECT_NAME}_vmodel_tests Threads::Threads)
target_include_directories(${PROJECT_NAME}_vmodel_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

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
