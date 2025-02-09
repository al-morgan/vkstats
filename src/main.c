#include <stdio.h>

#include "vulkan/vulkan.h"

#include "instance.h"
#include "config.h"
#include "device.h"
#include "util.h"

static VkPhysicalDevice get_physical_device(VkInstance instance, uint32_t device_index);

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

    VkPhysicalDevice physical_device = get_physical_device(instance.instance, 0);

    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
    printf("Using physical device: %s\n", physical_device_properties.deviceName);

    vkstats_device_builder device_builder;
    vkstats_device device;

    vkstats_device_builder_init(&device_builder, physical_device);
    vkstats_device_builder_add_queue(&device_builder, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
    vkstats_device_builder_add_queue(&device_builder, VK_QUEUE_TRANSFER_BIT);
    vkstats_device_builder_build(&device_builder, &device);

    vkstats_device_destroy(&device);
    vkstats_instance_destroy(&instance);
}

/* get_physical_device()
* 
* Gets a physical device based on the index into the physical device list.
* 
* instance: the instance to get the physical device from.
* device_index: the index into Vulkan's list of physical devices to return.
* 
* Returns a VkPhysicalDevice.

*/
static VkPhysicalDevice get_physical_device(VkInstance instance, uint32_t device_index)
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
        return physical_devices[device_index];
    }
    
    fatal_error("Invalid physical device!");
    return VK_NULL_HANDLE;
}
