#if !defined(VKSTATS_INSTANCE_H)
#define VKSTATS_INSTANCE_H

#include "vulkan/vulkan.h"

typedef struct
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT messenger;
} vkstats_instance;

/* vkstats_instance_create()
* 
* Creates a vkstats_instance, which encapsulates the VkInstance.
* 
* instance: the created instance will be placed here.
*/
void vkstats_instance_create(vkstats_instance *instance);

/* vkstats_instance_create()
*
* Destroys a vkstats_instance.
*
* instance: the instance to destroy.
*/
void vkstats_instance_destroy(vkstats_instance* instance);

#endif