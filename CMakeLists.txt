cmake_minimum_required(VERSION 3.10)

# Set the project name

set(PROJNAME "SdfTask")
project(${PROJNAME} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the SDL2 library

if(WIN32)
    set(SDL2_DIR "E:/SDL2/cmake")
    set(SDL2_INCLUDE_DIR E:/SDL2/include)
endif()

find_package(SDL2 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslc glslangValidator shaderc_combined)

############################################################################################################################
# Enable Vulkan
#

if(Vulkan_FOUND)
    Message(STATUS "--> using package VulkanSDK (linking with ${Vulkan_LIBRARY})")
    get_directory_property(hasParent PARENT_DIRECTORY)
    if(hasParent)
    set( USING_VULKANSDK "YES" PARENT_SCOPE) # PARENT_SCOPE important to have this variable passed to parent. Here we want to notify that something used the Vulkan package
    endif()
    set( USING_VULKANSDK "YES")
    option(VK_ENABLE_BETA_EXTENSIONS "Enable beta extensions provided by the Vulkan SDK" ON)
    add_definitions(-DNVP_SUPPORTS_VULKANSDK)
    if(VK_ENABLE_BETA_EXTENSIONS)
    add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)
    endif()
    add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

    if(NOT Vulkan_BUILD_DEPENDENCIES)
    set(Vulkan_BUILD_DEPENDENCIES ON CACHE BOOL "Create dependencies on GLSL files")
    endif()

    set(Vulkan_HEADERS_OVERRIDE_INCLUDE_DIR CACHE PATH "Override for Vulkan headers, leave empty to use SDK")

    if (Vulkan_HEADERS_OVERRIDE_INCLUDE_DIR)
    set(vulkanHeaderDir ${Vulkan_HEADERS_OVERRIDE_INCLUDE_DIR})
    else()
    set(vulkanHeaderDir ${Vulkan_INCLUDE_DIR})
    endif()

    Message(STATUS "--> using Vulkan Headers from: ${vulkanHeaderDir}")
    include_directories(${vulkanHeaderDir})
    set( vulkanHeaderFiles 
    "${vulkanHeaderDir}/vulkan/vulkan_core.h")
    LIST(APPEND PACKAGE_SOURCE_FILES ${vulkanHeaderFiles})
    source_group(Vulkan FILES ${vulkanHeaderFiles})

    LIST(APPEND LIBRARIES_OPTIMIZED ${Vulkan_LIBRARY})
    LIST(APPEND LIBRARIES_DEBUG ${Vulkan_LIBRARY})

    # CMake 3.21+ finds glslangValidator and glslc for us.
    # On < 3.21, find it manually:
    if((NOT Vulkan_GLSLANG_VALIDATOR_EXECUTABLE) OR (NOT Vulkan_GLSLC_EXECUTABLE))
    get_filename_component(_Vulkan_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
    find_program(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
        NAMES glslangValidator
        HINTS ${_Vulkan_LIB_DIR}/../Bin)
    find_program(Vulkan_GLSLC_EXECUTABLE
        NAMES glslc
        HINTS ${_Vulkan_LIB_DIR}/../Bin)
    endif()
else()
     Message(STATUS "--> NOT using package VulkanSDK")
endif()

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
