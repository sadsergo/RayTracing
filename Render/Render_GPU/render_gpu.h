#pragma once

#include <nvh/fileoperations.hpp>  // For nvh::loadFile
#include <nvvk/context_vk.hpp>
#include <nvvk/descriptorsets_vk.hpp>     // For nvvk::DescriptorSetContainer
#include <nvvk/error_vk.hpp>              // For NVVK_CHECK
#include <nvvk/raytraceKHR_vk.hpp>        // For nvvk::RaytracingBuilderKHR
#include <nvvk/resourceallocator_vk.hpp>  // For NVVK memory allocators
#include <nvvk/shaders_vk.hpp>            // For nvvk::createShaderModule

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include <stb_image_write.h>

namespace GPU
{
VkCommandBuffer AllocateAndBeginOneTimeCommandBuffer(VkDevice device, VkCommandPool cmdPool);
void EndSubmitWaitAndFreeCommandBuffer(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkCommandBuffer& cmdBuffer);
};