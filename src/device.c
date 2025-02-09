#include <stdlib.h>

#include "vulkan/vulkan.h"

#include "config.h"
#include "device.h"
#include "util.h"

void vkstats_device_builder_init(vkstats_device_builder* builder, VkPhysicalDevice physical_device)
{
    clear_struct(builder);
    builder->physical_device = physical_device;
}

void vkstats_device_builder_add_queue(vkstats_device_builder* builder, VkQueueFlags flags)
{
    if (builder->queue_count < array_length(builder->queues))
    {
        builder->queues[builder->queue_count] = flags;
        builder->queue_count++;
    }
    else
    {
        fatal_error("Maximum queues is too small!");
    }
}

void vkstats_device_builder_build(vkstats_device_builder* builder, vkstats_device* device)
{
    VkResult result;
    const float queue_priority = 0.0f;
    uint32_t queue_family_property_count = 1;
    VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];
    VkDeviceQueueCreateInfo queue_create_infos[MAX_QUEUES] = { 0 };

    vkGetPhysicalDeviceQueueFamilyProperties(builder->physical_device, &queue_family_property_count, NULL);

    if (queue_family_property_count > MAX_QUEUE_FAMILIES)
    {
        fatal_error("Max queue families is too small!");
    }

    vkGetPhysicalDeviceQueueFamilyProperties(builder->physical_device, &queue_family_property_count, queue_family_properties);

    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    for (uint32_t i = 0; i < builder->queue_count; i++)
    {
        uint32_t best_queue_family_index = UINT_MAX;
        uint32_t best_extra_flags = UINT_MAX;

        for (uint32_t j = 0; j < queue_family_property_count; j++)
        {
            if ((queue_family_properties[j].queueFlags & builder->queues[i]) != builder->queues[i])
            {
                continue;
            }

            VkFlags extra_flags = queue_family_properties[j].queueFlags ^ builder->queues[i];
            uint32_t extra_flag_count = count_flags(extra_flags);

            if (extra_flag_count < best_extra_flags)
            {
                best_queue_family_index = j;
                best_extra_flags = extra_flag_count;
            }
        }

        if (best_queue_family_index == UINT_MAX)
        {
            fatal_error("Could not create required queues!");
        }

        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = best_queue_family_index;
        queue_create_infos[i].queueCount = 1;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    VkDeviceCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.queueCreateInfoCount = builder->queue_count;

    result = vkCreateDevice(builder->physical_device, &create_info, NULL, &device->device);
    check_result(result, "Could not create device!");

    device->queue_count = builder->queue_count;

    for (uint32_t i = 0; i < builder->queue_count; i++)
    {
        vkGetDeviceQueue(device->device, queue_create_infos[i].queueFamilyIndex, 0, &device->queues[i]);
    }

    VkCommandPoolCreateInfo command_pool_ci = { 0 };
    command_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    for (uint32_t i = 0; i < builder->queue_count; i++)
    {
        command_pool_ci.queueFamilyIndex = queue_create_infos[i].queueFamilyIndex;
        result = vkCreateCommandPool(device->device, &command_pool_ci, NULL, &device->command_pools[i]);
        check_result(result, "Failed to create command pool!");
    }
}

void vkstats_device_destroy(vkstats_device* device)
{
    for (uint32_t i = 0; i < device->queue_count; i++)
    {
        vkDestroyCommandPool(device->device, device->command_pools[i], NULL);
    }

    vkDestroyDevice(device->device, NULL);
}
