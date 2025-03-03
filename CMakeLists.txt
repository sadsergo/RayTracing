cmake_minimum_required(VERSION 3.10)

# Set the project name
project(SdfTask CXX)

# Find the SDL2 library

if(WIN32)
    set(SDL2_DIR "E:/SDL2/cmake")
    set(SDL2_INCLUDE_DIR E:/SDL2/include)
endif()

find_package(SDL2 REQUIRED)

# Include SDL2 headers
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/LiteMath/external)

add_compile_definitions(USE_STB_IMAGE)

# Add the executable
add_executable(render
    main.cpp
    mesh.cpp
    Render/render.cpp
    Render/bvh.cpp
    model_structs/model.cpp
    LiteMath/Image2d.cpp)

# Link the SDL2 library to the executable
target_link_libraries(render ${SDL2_LIBRARIES})

# Set path to executable
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
