#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <sys/time.h>

/*
 Timer class (wont work on windows).
 */

struct Timer {	
    timeval start_time;
	
    // starts the timer
    void start() {
        gettimeofday(&start_time, NULL);
    }

    // returns how long it has been since the timer was last started in milliseconds
    uintmax_t getTime() {
        timeval current_time;
        gettimeofday(&current_time, NULL);
        return (current_time.tv_sec - start_time.tv_sec) * 1000 +
               (current_time.tv_usec - start_time.tv_usec) / 1000;
    } 
};

#endif //TIMER_H_
