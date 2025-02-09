#include <stdio.h>

#include "vulkan/vulkan.h"

#include "instance.h"
#include "physical_device.h"
#include "device.h"
#include "util.h"

/* main
* 
* argc: argument count.
* argv: argument values.
*/
int main(int argc, char** argv)
{
    argc; argv;

    vkstats_instance instance;
    vkstats_instance_create(&instance);

    vkstats_physical_device physical_device;
    vkstats_physical_device_get(&physical_device, instance.instance, 0);
    printf("Using physical device: %s\n", physical_device.properties.deviceName);

    vkstats_device device;
    vkstats_device_builder device_builder;
    vkstats_device_builder_init(&device_builder, &physical_device);
    vkstats_device_builder_add_queue(&device_builder, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
    vkstats_device_builder_add_queue(&device_builder, VK_QUEUE_TRANSFER_BIT);
    vkstats_device_builder_build(&device_builder, &device);

    VkCommandBuffer command_buffer;
    VkBuffer source_buffer;
    VkBuffer destination_buffer;
    VkDeviceMemory source_memory;
    VkDeviceMemory destination_memory;
    uint32_t size = 12;
    VkResult result;

    VkCommandBufferAllocateInfo cb_ci = { 0 };
    cb_ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cb_ci.commandBufferCount = 1;
    cb_ci.commandPool = device.command_pools[0];
    result = vkAllocateCommandBuffers(device.device, &cb_ci, &command_buffer);
    check_result(result, "Could not allocate command buffer!");

    VkBufferCreateInfo b_ci = { 0 };
    b_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    b_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    b_ci.pQueueFamilyIndices = &device.queue_family_indices[0];
    b_ci.queueFamilyIndexCount = 1;
    b_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    b_ci.size = size;
    result = vkCreateBuffer(device.device, &b_ci, NULL, &source_buffer);
    check_result(result, "Could not create buffer!");

    b_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    result = vkCreateBuffer(device.device, &b_ci, NULL, &destination_buffer);
    check_result(result, "Could not create buffer!");

    VkMemoryAllocateInfo m_ai = { 0 };
    m_ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    m_ai.allocationSize = size;
    m_ai.memoryTypeIndex = device.host_visible_memory_index;
    result = vkAllocateMemory(device.device, &m_ai, NULL, &source_memory);
    check_result(result, "Could not allocate memory!");

    m_ai.memoryTypeIndex = device.device_local_memory_index;
    result = vkAllocateMemory(device.device, &m_ai, NULL, &destination_memory);
    check_result(result, "Could not allocate memory!");

    result = vkBindBufferMemory(device.device, source_buffer, source_memory, 0);
    check_result(result, "Could not bind buffer memory!");
    result = vkBindBufferMemory(device.device, destination_buffer, destination_memory, 0);
    check_result(result, "Could not bind buffer memory!");

    vkFreeMemory(device.device, source_memory, NULL);
    vkFreeMemory(device.device, destination_memory, NULL);
    vkDestroyBuffer(device.device, source_buffer, NULL);
    vkDestroyBuffer(device.device, destination_buffer, NULL);



    vkstats_device_destroy(&device);
    vkstats_instance_destroy(&instance);
}
