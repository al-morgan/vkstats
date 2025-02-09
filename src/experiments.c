#include <stdio.h>

#include "vulkan/vulkan.h"

#include "device.h"
#include "util.h"
#include "stopwatch.h"
#include "experiments.h"

void vkstats_experiment_queue_transfer_speed(vkstats_device *device, uint32_t queue_index)
{
    VkResult result;

    vkstats_stopwatch stopwatch;
    vkstats_stopwatch_init(&stopwatch);

    printf("\n");
    printf("Running queue transfer speed experiment.\n");
    printf("Queue flags:\n");

    if (device->queue_flags[queue_index] & VK_QUEUE_GRAPHICS_BIT)
    {
        printf("Graphics\n");
    }
    if (device->queue_flags[queue_index] & VK_QUEUE_COMPUTE_BIT)
    {
        printf("Compute\n");
    }
    if (device->queue_flags[queue_index] & VK_QUEUE_TRANSFER_BIT)
    {
        printf("Transfer\n");
    }
    if (device->queue_flags[queue_index] & VK_QUEUE_SPARSE_BINDING_BIT)
    {
        printf("Sparse binding\n");
    }
    if (device->queue_flags[queue_index] & VK_QUEUE_PROTECTED_BIT)
    {
        printf("Protected\n");
    }
    if (device->queue_flags[queue_index] & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
    {
        printf("Video decode\n");
    }
    if (device->queue_flags[queue_index] & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
    {
        printf("Video encode\n");
    }
    if (device->queue_flags[queue_index] & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
    {
        printf("Optical flow\n");
    }

    printf("\n");

    /*
    * Create the command buffer to use.
    */
    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo cb_ci = { 0 };
    cb_ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cb_ci.commandBufferCount = 1;
    cb_ci.commandPool = device->command_pools[queue_index];
    result = vkAllocateCommandBuffers(device->device, &cb_ci, &command_buffer);
    check_result(result, "Could not allocate command buffer!");

    /*
    * Create a timeline semaphore to trigger/wait on the queue.
    */
    VkSemaphore semaphore;
    VkSemaphoreTypeCreateInfo st_ci = { 0 };
    st_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    st_ci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo s_ci = { 0 };
    s_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    s_ci.pNext = &st_ci;
    result = vkCreateSemaphore(device->device, &s_ci, NULL, &semaphore);
    check_result(result, "Could not create semaphore!");

    uint64_t semaphore_value = 0;

    /*
    * TODO: Check maximum allocation size and stop there.
    */
    for (VkDeviceSize size = 4; size <= UINT64_C(2) * UINT64_C(1024) * UINT64_C(1024) * UINT64_C(1024); size *= 2)
    {
        /*
        * Create source and destination buffers.
        */
        VkBuffer destination_buffer;
        VkBuffer source_buffer;
        VkBufferCreateInfo b_ci = { 0 };
        b_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        b_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        b_ci.pQueueFamilyIndices = &device->queue_family_indices[queue_index];
        b_ci.queueFamilyIndexCount = 1;
        b_ci.size = size;
        b_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        result = vkCreateBuffer(device->device, &b_ci, NULL, &source_buffer);
        check_result(result, "Could not create buffer!");

        b_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        result = vkCreateBuffer(device->device, &b_ci, NULL, &destination_buffer);
        check_result(result, "Could not create buffer!");

        /*
        * Create memory for the buffers.
        */
        VkDeviceMemory source_memory;
        VkDeviceMemory destination_memory;
        VkMemoryAllocateInfo m_ai = { 0 };
        m_ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        m_ai.allocationSize = size;
        m_ai.memoryTypeIndex = device->host_visible_memory_index;
        result = vkAllocateMemory(device->device, &m_ai, NULL, &source_memory);
        check_result(result, "Could not allocate memory!");

        m_ai.memoryTypeIndex = device->device_local_memory_index;
        result = vkAllocateMemory(device->device, &m_ai, NULL, &destination_memory);
        check_result(result, "Could not allocate memory!");

        result = vkBindBufferMemory(device->device, source_buffer, source_memory, 0);
        check_result(result, "Could not bind buffer memory!");
        result = vkBindBufferMemory(device->device, destination_buffer, destination_memory, 0);
        check_result(result, "Could not bind buffer memory!");

        /*
        * Create  command buffer and issue the copy command.
        */
        VkCommandBufferBeginInfo cb_bi = { 0 };
        cb_bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cb_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(command_buffer, &cb_bi);
        VkBufferCopy buffer_copy = { 0 };
        buffer_copy.size = size;
        vkCmdCopyBuffer(command_buffer, source_buffer, destination_buffer, 1, &buffer_copy);
        vkEndCommandBuffer(command_buffer);

        /*
        * Timeline semaphore value increases by two every loop. One for the
        * queue to wait upon and one to signal when complete.
        */
        semaphore_value += 2;

        uint64_t wait_value = semaphore_value;
        uint64_t signal_value = semaphore_value + 1;

        /*
        * Don't start the queue until the semaphore is signaled, and set a
        * different signal when it's done. That will allow us to carefully
        * control the start/end time from the host.
        */
        VkTimelineSemaphoreSubmitInfo ts_si = { 0 };
        ts_si.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        ts_si.pWaitSemaphoreValues = &wait_value;
        ts_si.waitSemaphoreValueCount = 1;
        ts_si.pSignalSemaphoreValues = &signal_value;
        ts_si.signalSemaphoreValueCount = 1;

        VkPipelineStageFlags wait_destination_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkSubmitInfo si = { 0 };
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.pNext = &ts_si;
        si.pCommandBuffers = &command_buffer;
        si.commandBufferCount = 1;
        si.pWaitSemaphores = &semaphore;
        si.waitSemaphoreCount = 1;
        si.pSignalSemaphores = &semaphore;
        si.signalSemaphoreCount = 1;
        si.pWaitDstStageMask = &wait_destination_stage_mask;

        /*
        * Wait until everything is quiet and submit the queue.
        */
        vkDeviceWaitIdle(device->device);
        vkQueueSubmit(device->queues[queue_index], 1, &si, VK_NULL_HANDLE);

        /*
        * Set up the semaphores to use for the test. We will signal the wait
        * value to trigger the queue, and then wait upon the signal value.
        */
        VkSemaphoreSignalInfo s_si = { 0 };
        s_si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        s_si.value = wait_value;
        s_si.semaphore = semaphore;

        VkSemaphoreWaitInfo s_wi = { 0 };
        s_wi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        s_wi.pSemaphores = &semaphore;
        s_wi.pValues = &signal_value;
        s_wi.semaphoreCount = 1;

        /*
        * Run the experiment.
        */
        double elapsed;
        vkstats_stopwatch_start(&stopwatch);
        vkSignalSemaphore(device->device, &s_si);
        vkWaitSemaphores(device->device, &s_wi, UINT64_MAX);
        elapsed = vkstats_stopwatch_stop(&stopwatch);
        vkDeviceWaitIdle(device->device);

        printf("Uploading %u bytes: %.2fms\n", (uint32_t)size, elapsed);

        vkFreeMemory(device->device, source_memory, NULL);
        vkFreeMemory(device->device, destination_memory, NULL);
        vkDestroyBuffer(device->device, source_buffer, NULL);
        vkDestroyBuffer(device->device, destination_buffer, NULL);

    }

    vkFreeCommandBuffers(device->device, device->command_pools[queue_index], 1, &command_buffer);
    vkDestroySemaphore(device->device, semaphore, NULL);
}