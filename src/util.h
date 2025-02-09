#if !defined(VKSTATS_UTIL_H)
#define VKSTATS_UTIL_H

#include <stdio.h>
#include <string.h> /* required for memset */
#include <stdlib.h>

#include "vulkan/vulkan.h"

/* array_length()
* 
* Returns the number of elements in an array.
* 
* a: the array to get the element count for.
* 
* Returns the number of elements in the array x.
*/
#define array_length(a) (sizeof(a) / sizeof(a[0]))

/* clear_struct()
* 
* Zero-initializes a struct.
* 
* s: the struct to clear.
*/
#define clear_struct(s) memset(s, 0, sizeof(*s))

/* fatal_error()
*
* Displays a message and aborts the application.
*
* message: the message to display.
*/
static void fatal_error(const char* message)
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
static void check_result(VkResult result, const char* message)
{
    if (result != VK_SUCCESS)
    {
        fatal_error(message);
    }
}

/* count_flags()
*
* Counts the number of flags set.
*
* flags: the flag bits to count.
*
* Returns the number of bits set.
*/
static uint32_t count_flags(VkFlags flags)
{
    uint32_t count = 0;

    for (uint32_t i = 0; i < 32; i++)
    {
        if (flags & 0x01)
        {
            count++;
        }

        flags >>= 1;
    }

    return count;
}

#endif