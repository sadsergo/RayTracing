#include "render_gpu.h"

namespace GPU
{
VkCommandBuffer AllocateAndBeginOneTimeCommandBuffer(VkDevice device, VkCommandPool cmdPool)
{
    VkCommandBufferAllocateInfo cmdAllocInfo{.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                            .commandPool        = cmdPool,
                                            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                            .commandBufferCount = 1};
    VkCommandBuffer             cmdBuffer;
    NVVK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmdBuffer));
    VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    NVVK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
    return cmdBuffer;
}

void EndSubmitWaitAndFreeCommandBuffer(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkCommandBuffer& cmdBuffer)
{
    NVVK_CHECK(vkEndCommandBuffer(cmdBuffer));
    VkSubmitInfo submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cmdBuffer};
    NVVK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    NVVK_CHECK(vkQueueWaitIdle(queue));
    vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
}

void RenderGPU_SDF(std::vector<float> sdf, size_t size)
{

}

struct Camera 
{
  glm::vec3 position;
  glm::vec3 target;
  float aspect;
  float fov;
};

struct Light
{
  glm::vec3 position;
};

static const uint64_t render_width     = 800;
static const uint64_t render_height    = 600;
static const uint32_t workgroup_width  = 16;
static const uint32_t workgroup_height = 8;

void RenderGPU_Grid(int argc, char **args)
{
  Camera camera;
  camera.position = glm::vec3(2, 2, 2);
  camera.target = glm::vec3(0, 0, 0);
  camera.aspect = (float)render_width / render_height;
  camera.fov = std::numbers::pi / 4.0f;

  Light light {{3, 0, 100}};

  std::vector<float> sdf;
  // get_sphere_sdf(sdf, 32);

  // Create the Vulkan context, consisting of an instance, device, physical device, and queues.
  nvvk::ContextCreateInfo deviceInfo;  // One can modify this to load different extensions or pick the Vulkan core version
  deviceInfo.apiMajor = 1;             // Specify the version of Vulkan we'll use
  deviceInfo.apiMinor = 2;
  // Required by KHR_acceleration_structure; allows work to be offloaded onto background threads and parallelized
  deviceInfo.addDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
  // VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
  // deviceInfo.addDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, false, &asFeatures);
  // VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
  // deviceInfo.addDeviceExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME, false, &rayQueryFeatures);
  
  // Add the required device extensions for Debug Printf. If this is confusing,
  // don't worry; we'll remove this in the next chapter.
  deviceInfo.addDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
  VkValidationFeatureEnableEXT validationFeatureToEnable = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
  VkValidationFeaturesEXT      validationInfo{.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
                                              .enabledValidationFeatureCount = 1,
                                              .pEnabledValidationFeatures    = &validationFeatureToEnable};
  deviceInfo.instanceCreateInfoExt = &validationInfo;
#ifdef _WIN32
  _putenv_s("DEBUG_PRINTF_TO_STDOUT", "1");
#else   // If not _WIN32
  static char putenvString[] = "DEBUG_PRINTF_TO_STDOUT=1";
  putenv(putenvString);
#endif  // _WIN32

  nvvk::Context context;     // Encapsulates device state in a single object
  context.init(deviceInfo);  // Initialize the context

  // Create the allocator
  nvvk::ResourceAllocatorDedicated allocator;
  allocator.init(context, context.m_physicalDevice);

  VkCommandPool           cmdPool;
  // Create the command pool
  VkCommandPoolCreateInfo cmdPoolInfo{.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  //
                                      .queueFamilyIndex = context.m_queueGCT};
  NVVK_CHECK(vkCreateCommandPool(context, &cmdPoolInfo, nullptr, &cmdPool));

  VkDeviceSize       bufferSizeBytes = render_width * render_height * 3 * sizeof(float);
  // Create a buffer
  VkBufferCreateInfo bufferCreateInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                      .size  = bufferSizeBytes,
                                      .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
  // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT means that the CPU can read this buffer's memory.
  // VK_MEMORY_PROPERTY_HOST_CACHED_BIT means that the CPU caches this memory.
  // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT means that the CPU side of cache management
  // is handled automatically, with potentially slower reads/writes.
  nvvk::Buffer buffer = allocator.createBuffer(bufferCreateInfo,                         //
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT       //
                                                   | VK_MEMORY_PROPERTY_HOST_CACHED_BIT  //
                                                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  nvvk::Buffer cameraBuffer;
  void* cameraData;
  VkDeviceSize cameraBufferSizeBytes = sizeof(Camera);

  {
    cameraBuffer = allocator.createBuffer(
      cameraBufferSizeBytes,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    cameraData = allocator.map(cameraBuffer);
    memcpy(cameraData, &camera, cameraBufferSizeBytes);
  }

  nvvk::Buffer lightBuffer;
  void* lightData;
  VkDeviceSize lightBufferSizeBytes = sizeof(Light);

  {
    lightBuffer = allocator.createBuffer(
      lightBufferSizeBytes,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    lightData = allocator.map(lightBuffer);
    memcpy(lightData, &light, lightBufferSizeBytes);
  }

  nvvk::Buffer sdfBuffer;
  VkDeviceSize sdfBufferSizeBytes = sizeof(float) * sdf.size();

  {
     // Start a command buffer for uploading the buffer
    VkCommandBuffer uploadCmdBuffer = AllocateAndBeginOneTimeCommandBuffer(context, cmdPool);
    const VkBufferUsageFlags usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    
    sdfBuffer = allocator.createBuffer(uploadCmdBuffer, sdf, usage);

    EndSubmitWaitAndFreeCommandBuffer(context, context.m_queueGCT, cmdPool, uploadCmdBuffer);
  }

  const std::string        exePath(args[0], std::string(args[0]).find_last_of("/\\") + 1);
  std::vector<std::string> searchPaths = {exePath + PROJECT_RELDIRECTORY, exePath + PROJECT_RELDIRECTORY "..",
                                          exePath + PROJECT_RELDIRECTORY "../..", exePath + PROJECT_NAME};

  // Here's the list of bindings for the descriptor set layout, from raytrace.comp.glsl:
  nvvk::DescriptorSetContainer descriptorSetContainer(context);
  descriptorSetContainer.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
  descriptorSetContainer.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
  descriptorSetContainer.addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
  descriptorSetContainer.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);

  // Create a layout from the list of bindings
  descriptorSetContainer.initLayout();
  // Create a descriptor pool from the list of bindings with space for 1 set, and allocate that set
  descriptorSetContainer.initPool(1);
  // Create a simple pipeline layout from the descriptor set layout:
  descriptorSetContainer.initPipeLayout();

  // Write a single descriptor in the descriptor set.
  VkDescriptorBufferInfo descriptorBufferInfo{.buffer = buffer.buffer,    // The VkBuffer object
                                              .range  = bufferSizeBytes};  // The length of memory to bind; offset is 0.
  VkDescriptorBufferInfo cameraDescriptorBufferInfo{.buffer = cameraBuffer.buffer,    // The VkBuffer object
                                                    .range  = cameraBufferSizeBytes};  // The length of memory to bind; offset is 0.

  VkDescriptorBufferInfo lightDescriptorBufferInfo{.buffer = lightBuffer.buffer,    // The VkBuffer object
                                                   .range  = lightBufferSizeBytes};  // The length of memory to bind; offset is 0.
  
  VkDescriptorBufferInfo sdfDescriptorBufferInfo{.buffer = sdfBuffer.buffer,
                                                .range  = sdfBufferSizeBytes};

  std::array<VkWriteDescriptorSet, 4> writeDescriptorSets;
  writeDescriptorSets[0] = descriptorSetContainer.makeWrite(0 /*set index*/, 0 /*binding*/, &descriptorBufferInfo);
  writeDescriptorSets[1] = descriptorSetContainer.makeWrite(0 /*set index*/, 1 /*binding*/, &cameraDescriptorBufferInfo);
  writeDescriptorSets[2] = descriptorSetContainer.makeWrite(0 /*set index*/, 2 /*binding*/, &lightDescriptorBufferInfo);
  writeDescriptorSets[3] = descriptorSetContainer.makeWrite(0 /*set index*/, 3 /*binding*/, &sdfDescriptorBufferInfo);

  vkUpdateDescriptorSets(context,              // The context
                         static_cast<uint32_t>(writeDescriptorSets.size()), 
                         writeDescriptorSets.data(),  // An array of VkWriteDescriptorSet objects
                         0, nullptr);          // An array of VkCopyDescriptorSet objects (unused)

  // Shader loading and pipeline creation
  VkShaderModule rayTraceModule =
      nvvk::createShaderModule(context, nvh::loadFile("shaders/raytrace.comp.glsl.spv", true, searchPaths));

  // Describes the entrypoint and the stage to use for this shader module in the pipeline
  VkPipelineShaderStageCreateInfo shaderStageCreateInfo{.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                        .stage  = VK_SHADER_STAGE_COMPUTE_BIT,
                                                        .module = rayTraceModule,
                                                        .pName  = "main"};

  // Create the compute pipeline
  VkComputePipelineCreateInfo pipelineCreateInfo{.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                                 .stage  = shaderStageCreateInfo,
                                                 .layout = descriptorSetContainer.getPipeLayout()};
  // Don't modify flags, basePipelineHandle, or basePipelineIndex
  VkPipeline computePipeline;
  NVVK_CHECK(vkCreateComputePipelines(context,                 // Device
                                      VK_NULL_HANDLE,          // Pipeline cache (uses default)
                                      1, &pipelineCreateInfo,  // Compute pipeline create info
                                      nullptr,                 // Allocator (uses default)
                                      &computePipeline));      // Output

  // Allocate a command buffer
  VkCommandBufferAllocateInfo cmdAllocInfo{.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                           .commandPool        = cmdPool,
                                           .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                           .commandBufferCount = 1};
  VkCommandBuffer             cmdBuffer;
  NVVK_CHECK(vkAllocateCommandBuffers(context, &cmdAllocInfo, &cmdBuffer));

  // Begin recording
  VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                     .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
  NVVK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));

  // Bind the compute shader pipeline
  vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
  // Bind the descriptor set
  VkDescriptorSet descriptorSet = descriptorSetContainer.getSet(0);
  vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, descriptorSetContainer.getPipeLayout(), 0, 1,
                          &descriptorSet, 0, nullptr);

  // Run the compute shader with enough workgroups to cover the entire buffer:
  vkCmdDispatch(cmdBuffer, (uint32_t(render_width) + workgroup_width - 1) / workgroup_width,
                (uint32_t(render_height) + workgroup_height - 1) / workgroup_height, 1);

  // Add a command that says "Make it so that memory writes by the compute shader
  // are available to read from the CPU." (In other words, "Flush the GPU caches
  // so the CPU can read the data.") To do this, we use a memory barrier.
  // This is one of the most complex parts of Vulkan, so don't worry if this is
  // confusing! We'll talk about pipeline barriers more in the extras.
  VkMemoryBarrier memoryBarrier{.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                                .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,  // Make shader writes
                                .dstAccessMask = VK_ACCESS_HOST_READ_BIT};    // Readable by the CPU
  vkCmdPipelineBarrier(cmdBuffer,                                             // The command buffer
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,                  // From the compute shader
                       VK_PIPELINE_STAGE_HOST_BIT,                            // To the CPU
                       0,                                                     // No special flags
                       1, &memoryBarrier,                                     // An array of memory barriers
                       0, nullptr, 0, nullptr);                               // No other barriers

  // End recording
  NVVK_CHECK(vkEndCommandBuffer(cmdBuffer));

  // Submit the command buffer
  VkSubmitInfo submitInfo{.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,  //
                          .commandBufferCount = 1,                              //
                          .pCommandBuffers    = &cmdBuffer};
  NVVK_CHECK(vkQueueSubmit(context.m_queueGCT, 1, &submitInfo, VK_NULL_HANDLE));

  // Wait for the GPU to finish
  NVVK_CHECK(vkQueueWaitIdle(context.m_queueGCT));

  // Get the image data back from the GPU
  void* data = allocator.map(buffer);
  // stbi_write_hdr("out.hdr", render_width, render_height, 3, reinterpret_cast<float*>(data));
  allocator.unmap(buffer);

  vkDestroyPipeline(context, computePipeline, nullptr);
  vkDestroyShaderModule(context, rayTraceModule, nullptr);
  descriptorSetContainer.deinit();
  vkFreeCommandBuffers(context, cmdPool, 1, &cmdBuffer);
  vkDestroyCommandPool(context, cmdPool, nullptr);
  
  allocator.destroy(buffer);
  allocator.destroy(sdfBuffer);
  allocator.destroy(cameraBuffer);
  allocator.destroy(lightBuffer);
  
  allocator.deinit();
  
  context.deinit();  // Don't forget to clean up at the end of the program!
}
};

