#if !defined(VKSTATS_STOPWATCH_H)
#define VKSTATS_STOPWATCH_H

#include <stdint.h>

typedef struct
{
    uint64_t    start_time;
    double      frequency;
} vkstats_stopwatch;

/*
* vkstats_stopwatch_init()
* 
* Initialize a stopwatch.
* 
* stopwatch: the stopwatch to initialize.
*/
void vkstats_stopwatch_init(vkstats_stopwatch* stopwatch);

/*
* vkstats_stopwatch_start()
*
* Starts a stopwatch.
*
* stopwatch: the stopwatch to start.
*/
void vkstats_stopwatch_start(vkstats_stopwatch* stopwatch);

/*
* vkstats_stopwatch_stop()
*
* Stops a stopwatch, returning the elapsed time since it was started, in
* milliseconds.
*
* stopwatch: the stopwatch to stop.
* 
* Returns the elapsed time in milliseconds.
*/
double vkstats_stopwatch_stop(vkstats_stopwatch* stopwatch);

#endif
