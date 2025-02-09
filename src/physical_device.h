#if !defined(VKSTATS_PHYSICAL_DEVICE_H)
#define VKSTATS_PHYSICAL_DEVICE_H

#include "vulkan/vulkan.h"

typedef struct
{
    VkPhysicalDevice                    physical_device;
    VkPhysicalDeviceProperties          properties;
    VkPhysicalDeviceMemoryProperties    memory_properties;
} vkstats_physical_device;

/*
* vkstats_physical_device_get()
*
* Gets a physical device based on the index into the physical device list.
*
* device: the physical device will be placed here.
* instance: the instance to get the physical device from.
* device_index: the index into Vulkan's list of physical devices to get.
*/
void vkstats_physical_device_get(vkstats_physical_device* physical_device, VkInstance instance, uint32_t device_index);

#endif
