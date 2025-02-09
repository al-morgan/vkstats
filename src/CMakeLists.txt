cmake_minimum_required(VERSION 3.24)
project(vkstats VERSION 0.1)

find_package(Vulkan)

add_executable(vkstats main.c config.h device.c device.h util.h)
target_include_directories(vkstats PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(vkstats ${Vulkan_LIBRARIES})
set_target_properties(vkstats PROPERTIES COMPILE_WARNING_AS_ERROR TRUE)

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(vkstats PRIVATE "/W4")
    target_compile_options(vkstats PRIVATE "/std:c11")
endif()
