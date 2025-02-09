#include "vulkan/vulkan.h"

#include "config.h"
#include "util.h"
#include "physical_device.h"

/*
* get_physical_device()
*
* Gets a physical device based on the index into the physical device list.
*
* instance: the instance to get the physical device from.
* device_index: the index into Vulkan's list of physical devices to return.
*
* Returns a VkPhysicalDevice.

*/
void vkstats_physical_device_get(vkstats_physical_device* physical_device, VkInstance instance, uint32_t device_index)
{
    uint32_t physical_device_count;
    VkPhysicalDevice physical_devices[MAX_PHYSICAL_DEVICES];

    vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);

    if (physical_device_count > MAX_PHYSICAL_DEVICES)
    {
        fatal_error("Maximum physical devices is too small!");
    }

    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);

    if (device_index < physical_device_count)
    {
         physical_device->physical_device = physical_devices[device_index];
    }
    else
    {
        fatal_error("Invalid physical device!");
    }

    vkGetPhysicalDeviceProperties(physical_device->physical_device, &physical_device->properties);
    vkGetPhysicalDeviceMemoryProperties(physical_device->physical_device, &physical_device->memory_properties);
}
