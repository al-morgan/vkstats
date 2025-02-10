#include <stdio.h>

#include "windows.h"
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


    LARGE_INTEGER frequency;
    
    QueryPerformanceFrequency(&frequency);
    printf("Performance frequency: %d\n", (int)frequency.QuadPart);

    VkCommandBuffer command_buffer;
    VkBuffer source_buffer;
    VkBuffer destination_buffer;
    VkDeviceMemory source_memory;
    VkDeviceMemory destination_memory;
    uint32_t size = 12;
    VkResult result;
    VkSemaphore starter_pistol;
    VkSemaphore finish_line;

    VkCommandBufferAllocateInfo cb_ci = { 0 };
    cb_ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cb_ci.commandBufferCount = 1;
    cb_ci.commandPool = device.command_pools[0];
    result = vkAllocateCommandBuffers(device.device, &cb_ci, &command_buffer);
    check_result(result, "Could not allocate command buffer!");

    VkSemaphoreTypeCreateInfo st_ci = { 0 };
    st_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    st_ci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo s_ci = { 0 };
    s_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    s_ci.pNext = &st_ci;
    result = vkCreateSemaphore(device.device, &s_ci, NULL, &starter_pistol);
    check_result(result, "Could not create semaphore!");
    result = vkCreateSemaphore(device.device, &s_ci, NULL, &finish_line);
    check_result(result, "Could not create semaphore!");

    for (uint32_t i = 0; i < 10000; i++)
    {
        size += 4;

        VkBufferCreateInfo b_ci = { 0 };
        b_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        b_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        b_ci.pQueueFamilyIndices = &device.queue_family_indices[0];
        b_ci.queueFamilyIndexCount = 1;
        b_ci.size = size;
        b_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
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

        VkCommandBufferBeginInfo cb_bi = { 0 };
        cb_bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cb_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(command_buffer, &cb_bi);
        VkBufferCopy buffer_copy = { 0 };
        buffer_copy.size = size;
        vkCmdCopyBuffer(command_buffer, source_buffer, destination_buffer, 1, &buffer_copy);
        vkEndCommandBuffer(command_buffer);

        uint64_t foo = i + 1;

        VkTimelineSemaphoreSubmitInfo ts_si = { 0 };
        ts_si.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        ts_si.pWaitSemaphoreValues = &foo;
        ts_si.waitSemaphoreValueCount = 1;
        ts_si.pSignalSemaphoreValues = &foo;
        ts_si.signalSemaphoreValueCount = 1;

        VkPipelineStageFlags bar = VK_PIPELINE_STAGE_TRANSFER_BIT;
        
        VkSubmitInfo si = { 0 };
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.pNext = &ts_si;
        si.pCommandBuffers = &command_buffer;
        si.commandBufferCount = 1;
        si.pWaitSemaphores = &starter_pistol;
        si.waitSemaphoreCount = 1;
        si.pSignalSemaphores = &finish_line;
        si.signalSemaphoreCount = 1;
        si.pWaitDstStageMask = &bar;

        LARGE_INTEGER start_time;
        LARGE_INTEGER end_time;

        vkQueueSubmit(device.queues[0], 1, &si, VK_NULL_HANDLE);

        VkSemaphoreSignalInfo s_si = { 0 };
        s_si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        s_si.value = i + 1;
        s_si.semaphore = starter_pistol;

        VkSemaphoreWaitInfo s_wi = { 0 };
        s_wi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        s_wi.pSemaphores = &finish_line;
        s_wi.pValues = &foo;
        s_wi.semaphoreCount = 1;

        QueryPerformanceCounter(&start_time);
        vkSignalSemaphore(device.device, &s_si);
        vkWaitSemaphores(device.device, &s_wi, UINT64_MAX);
        QueryPerformanceCounter(&end_time);
        vkDeviceWaitIdle(device.device);

        uint64_t elapsed = end_time.QuadPart - start_time.QuadPart;
        printf("%d %d\n", size, (int)elapsed);

        vkFreeMemory(device.device, source_memory, NULL);
        vkFreeMemory(device.device, destination_memory, NULL);
        vkDestroyBuffer(device.device, source_buffer, NULL);
        vkDestroyBuffer(device.device, destination_buffer, NULL);
    }

    vkDestroySemaphore(device.device, starter_pistol, NULL);
    vkDestroySemaphore(device.device, finish_line, NULL);

    vkstats_device_destroy(&device);
    vkstats_instance_destroy(&instance);
}
