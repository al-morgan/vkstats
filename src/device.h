#if !defined(VKSTATS_DEVICE_H)
#define VKSTATS_DEVICE_H

#include <stdint.h>

#include "vulkan/vulkan.h"

#include "config.h"
#include "physical_device.h"

typedef struct
{
    VkDevice        device;
    VkQueue         queues[MAX_QUEUES];
    uint32_t        queue_count;
    uint32_t        queue_family_indices[MAX_QUEUES];
    VkCommandPool   command_pools[MAX_POOLS];
    uint32_t        device_local_memory_index;
    uint32_t        host_visible_memory_index;
} vkstats_device;

typedef struct
{
    VkDevice                    device;
    vkstats_physical_device*    physical_device;
    VkQueueFlags                queues[MAX_QUEUES];
    uint32_t                    queue_count;
} vkstats_device_builder;

/* vkstats_device_builder_init()
* 
* Initialize a device builder.
* 
* builder: the builder to initialize.
* physical_device: the physical device to create the device for.
*/
void vkstats_device_builder_init(vkstats_device_builder* builder, vkstats_physical_device* physical_device);

/* vkstats_device_builder_add_queue()
* 
* Adds a queue to be created to a physical device. A queue will be created for
* a queue family that matches all of the specified flags and a minimal amount
* of unrequested flags.
* 
* builder: the builder to add a queue to.
* flags: required flags for the queue.
*/
void vkstats_device_builder_add_queue(vkstats_device_builder* builder, VkQueueFlags flags);

/* vkstats_device_builder_build()
* 
* Finalizes the build, creating the vkstats_device.
* 
* builder: the builder to build from.
* device: the destination device.
*/
void vkstats_device_builder_build(vkstats_device_builder* builder, vkstats_device* device);

/* vkstats_device_destroy()
* 
* Destroys a vkstats_device.
* 
* device: the vkstats_device to destroy.
*/
void vkstats_device_destroy(vkstats_device* device);

#endif