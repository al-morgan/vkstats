#include "vulkan/vulkan.h"

#include "config.h"
#include "util.h"
#include "instance.h"


static VkDebugUtilsMessengerEXT create_messenger(VkInstance instance);
static void destroy_messenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger);
VkDebugUtilsMessengerCreateInfoEXT get_messenger_create_info(void);
VkBool32 debug_messenger(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);
void check_layer(const char* layer_name);

void vkstats_instance_create(vkstats_instance* instance)
{
    VkResult result;
    VkDebugUtilsMessengerCreateInfoEXT debug_utils;
    VkInstanceCreateInfo instance_ci = { 0 };

    debug_utils = get_messenger_create_info();
    const char* enabled_extensions[] = { "VK_EXT_debug_utils" };
    const char* enabled_layers[] = { "VK_LAYER_KHRONOS_validation" };

    check_layer("VK_LAYER_KHRONOS_validation");

    VkApplicationInfo application_info = { 0 };
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.apiVersion = VK_API_VERSION_1_3;

    instance_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_ci.pNext = &debug_utils;
    instance_ci.enabledLayerCount = array_length(enabled_layers);
    instance_ci.ppEnabledLayerNames = enabled_layers;
    instance_ci.ppEnabledExtensionNames = enabled_extensions;
    instance_ci.enabledExtensionCount = array_length(enabled_extensions);
    instance_ci.pApplicationInfo = &application_info;
    result = vkCreateInstance(&instance_ci, NULL, &instance->instance);
    check_result(result, "Could not create instance!");

    instance->messenger = create_messenger(instance->instance);
}

void vkstats_instance_destroy(vkstats_instance* instance)
{
    destroy_messenger(instance->instance, instance->messenger);
    vkDestroyInstance(instance->instance, NULL);
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
