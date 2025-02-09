#include "Windows.h"

#include "stopwatch.h"
#include "util.h"

void vkstats_stopwatch_init(vkstats_stopwatch* stopwatch)
{
    LARGE_INTEGER frequency;

    clear_struct(stopwatch);
    QueryPerformanceFrequency(&frequency);
    stopwatch->frequency = (double)frequency.QuadPart;
}

void vkstats_stopwatch_start(vkstats_stopwatch* stopwatch)
{
    LARGE_INTEGER now;

    QueryPerformanceCounter(&now);
    stopwatch->start_time = now.QuadPart;
}

double vkstats_stopwatch_stop(vkstats_stopwatch* stopwatch)
{
    LARGE_INTEGER now;

    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - stopwatch->start_time) / stopwatch->frequency * 1000.0;
}
