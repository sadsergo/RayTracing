cmake_minimum_required(VERSION 3.10)

# Set the project name

set(PROJNAME "SdfTask")
project(${PROJNAME} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DPROJECT_RELDIRECTORY="${TO_CURRENT_SOURCE_DIR}/")
add_definitions(-DPROJECT_NAME="${name}")

# Find the SDL2 library

if(WIN32)
    set(SDL2_DIR "E:/SDL2/cmake")
    set(SDL2_INCLUDE_DIR E:/SDL2/include)
endif()

find_package(SDL2 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslc glslangValidator shaderc_combined)

# Include SDL2 headers
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/external/LiteMath/external)
include_directories(${CMAKE_SOURCE_DIR}/external/LiteMath)

add_compile_definitions(USE_STB_IMAGE)

############################################################################################################################
# Link nvpro_core
#

if(NOT BASE_DIRECTORY)
  find_path(BASE_DIRECTORY
    NAMES external/nvpro_core/cmake/setup.cmake
    PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. 
    REQUIRED
    DOC "Directory containing nvpro_core"
    )
endif()

set (BASE_DIRECTORY ${BASE_DIRECTORY}/external)
if(EXISTS ${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
  include(${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
else()
  message(FATAL_ERROR "could not find base directory, please set BASE_DIRECTORY to folder containing nvpro_core")
endif()

add_subdirectory(${BASE_DIRECTORY}/nvpro_core ${CMAKE_BINARY_DIR}/nvpro_core)

############################################################################################################################
# Add the executable
#
add_executable(render
    main.cpp
    structs/mesh.cpp
    structs/grid.cpp
    structs/octree.cpp
    Render/Render_CPU/render.cpp
    Render/Render_CPU/bvh.cpp
    Render/Render_GPU/render_gpu.cpp
    external/LiteMath/Image2d.cpp)

# Link the SDL2 library to the executable
target_link_libraries(render ${SDL2_LIBRARIES} OpenMP::OpenMP_CXX nvpro_core)

# Set path to executable
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
