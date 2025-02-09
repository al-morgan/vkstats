#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vulkan/vulkan.h"

#define MAX_INSTANCE_LAYER_PROPERTIES 13
#define MAX_PHYSICAL_DEVICES 2

#define TRUE 1
#define FALSE 0

#define array_length(x) (sizeof(x) / sizeof(x[0]))

void fatal_error(const char* message);
void check_result(VkResult result, const char* message);
static VkBool32 debug_messenger(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);
static VkInstance create_instance(void);
static void destroy_instance(VkInstance instance);
static VkDebugUtilsMessengerCreateInfoEXT get_messenger_create_info(void);
static VkDebugUtilsMessengerEXT create_messenger(VkInstance instance);
static void destroy_messenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger);
static VkPhysicalDevice get_physical_device(VkInstance instance, uint32_t device_index);

/* main
* 
* argc: argument count.
* argv: argument values.
*/
int main(int argc, char** argv)
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT messenger;
    VkPhysicalDevice physical_device;

    argc; argv;

    instance = create_instance();
    messenger = create_messenger(instance);

    physical_device = get_physical_device(instance, 0);

    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
    printf("Using physical device: %s\n", physical_device_properties.deviceName);

    destroy_messenger(instance, messenger);
    destroy_instance(instance);
}

/* fatal_error()
* 
* Displays a message and aborts the application.
* 
* message: the message to display.
*/
void fatal_error(const char* message)
{
    printf("%s\n", message);
    exit(-1);
}

/* check_result()
* 
* Aborts the application if a result is not VK_SUCCESS.
* 
* result: the result to check.
* message: the message to display if the check fails.
*/
void check_result(VkResult result, const char *message)
{
    if (result != VK_SUCCESS)
    {
        fatal_error(message);
    }
}

/* debug_messenger()
* 
* Debug messenger callback.
* 
* severity: the severity of the message.
* type: the type of the message.
* callback_data: Vulkan-supplied callback data.
* user_data: user-supplied callback data.
* 
* Returns true if the application should abort.
*/
VkBool32 debug_messenger(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
    severity; types; user_data;

    printf("%s\n", callback_data->pMessage);
    return VK_FALSE;
}

/* get_messenger_create_info()
* 
* Gets messenger data so it can be supplied during messenger creation as well
* as instance creation.
* 
* Returns a complete VkDebugUtilsMessengerCreateInfoEXT object.
*/
VkDebugUtilsMessengerCreateInfoEXT get_messenger_create_info(void)
{
    VkDebugUtilsMessengerCreateInfoEXT debug_utils = { 0 };

    debug_utils.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_utils.pfnUserCallback = debug_messenger;
    debug_utils.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_utils.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    return debug_utils;
}

/* check_layer()
*
* Ensures that a Vulkan layer is available. Aborts the application if not.
* 
* layer_name: the layer to check for.
*/
void check_layer(const char* layer_name)
{
    uint32_t i;
    VkResult result;
    uint32_t property_count;
    VkLayerProperties properties[MAX_INSTANCE_LAYER_PROPERTIES];

    vkEnumerateInstanceLayerProperties(&property_count, NULL);

    if (property_count > MAX_INSTANCE_LAYER_PROPERTIES)
    {
        fatal_error("Maximum instance layer properties too small.");
    }

    result = vkEnumerateInstanceLayerProperties(&property_count, properties);
    check_result(result, "Could not enumerate instance layer properties!");

    for (i = 0; i < property_count; i++)
    {
        if (strcmp(layer_name, properties[i].layerName) == 0)
        {
            return;
        }
    }

    fatal_error("Could not find required layer!");
}

/* create_instance()
* 
* Creates a Vulkan instance.
* 
* Returns a new VkInstance.
*/
static VkInstance create_instance(void)
{
    VkResult result;
    VkInstance instance;
    VkDebugUtilsMessengerCreateInfoEXT debug_utils;
    VkInstanceCreateInfo instance_ci = { 0 };
        
    debug_utils = get_messenger_create_info();
    const char* enabled_extensions[] = { "VK_EXT_debug_utils" };
    const char* enabled_layers[] = { "VK_LAYER_KHRONOS_validation" };

    check_layer("VK_LAYER_KHRONOS_validation");

    instance_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_ci.pNext = &debug_utils;
    instance_ci.enabledLayerCount = array_length(enabled_layers);
    instance_ci.ppEnabledLayerNames = enabled_layers;
    instance_ci.ppEnabledExtensionNames = enabled_extensions;
    instance_ci.enabledExtensionCount = array_length(enabled_extensions);
    result = vkCreateInstance(&instance_ci, NULL, &instance);
    check_result(result, "Could not create instance!");

    return instance;
}

/* destroy_instance()
* 
* Destroys a Vulkan instance.
* 
* instance: the instance to destroy.
*/
static void destroy_instance(VkInstance instance)
{
    vkDestroyInstance(instance, NULL);
}

/* create_messenger()
* 
* Create a debug messenger.
* 
* instance: The Vulkan instance to create the messenger for.
* 
* Returns a VkDebugUtilsMessengerEXT handle.
*/
static VkDebugUtilsMessengerEXT create_messenger(VkInstance instance)
{
    VkResult result;
    VkDebugUtilsMessengerEXT messenger;
    VkDebugUtilsMessengerCreateInfoEXT debug_utils;
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
    
    debug_utils = get_messenger_create_info();
    vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    result = vkCreateDebugUtilsMessengerEXT(instance, &debug_utils, NULL, &messenger);
    check_result(result, "Could not create debug utils messenger!");

    return messenger;
}

/* destroy_messenger()
* 
* Destroy a debug messenger.
* 
* instance: the Vulkan instance that was used to create the messenger.
* messenger: the messenger to destroy.
*/
static void destroy_messenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
{
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

    vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT(instance, messenger, NULL);
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
