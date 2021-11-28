#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
// #include <time.h>
#include <sys/time.h>
#include "config.h"
#include "clock.h"
#include "utils.h"

Clock os_clock;


void init_clock() {
  os_clock.sec = 0;
  os_clock.ns = 0;
}

Clock increment_clock_round() {
  // get random ns [0,1000] (ms)
  int ms = getRandom(MILISECONDS+1);

  // convert ms to ns
  int ns = ms * 1000000;

  // create new time with 1 + ns
  Clock time_diff = add_time_to_clock(1, ns);

  return time_diff;
}

Clock add_time_to_clock(int sec, int ns) {
  // add seconds
  os_clock.sec = os_clock.sec + sec;

  // check ns for overflow, handle accordingly
  if((os_clock.ns + ns) >= NANOSECONDS) {
    int remaining_ns = (os_clock.ns + ns) - NANOSECONDS;
    os_clock.sec += 1;
    os_clock.ns = remaining_ns;
  }
  else
    os_clock.ns += ns;
  
  printf("\n");

  printf("new time: %d sec, %d ns\n", os_clock.sec, os_clock.ns);

  Clock time_diff;
  time_diff.sec = sec;
  time_diff.ns = ns;

  return time_diff;
}