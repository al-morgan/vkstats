#include <stdio.h>

#include "vulkan/vulkan.h"

#include "instance.h"
#include "physical_device.h"
#include "device.h"

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
    vkstats_device_builder_init(&device_builder, physical_device.physical_device);
    vkstats_device_builder_add_queue(&device_builder, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
    vkstats_device_builder_add_queue(&device_builder, VK_QUEUE_TRANSFER_BIT);
    vkstats_device_builder_build(&device_builder, &device);

    vkstats_device_destroy(&device);
    vkstats_instance_destroy(&instance);
}
