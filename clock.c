#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
// #include <time.h>
#include <sys/time.h>
#include "config.h"
#include "page_table.h"
#include "clock.h"
#include "utils.h"


extern PageTable *page_table;

Clock init_clock() {
  Clock os_clock;
  os_clock.sec = 0;
  os_clock.ns = 0;

  return os_clock;
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
  page_table->clock.sec = page_table->clock.sec + sec;

  // check ns for overflow, handle accordingly
  if((page_table->clock.ns + ns) >= NANOSECONDS) {
    int remaining_ns = (page_table->clock.ns + ns) - NANOSECONDS;
    page_table->clock.sec += 1;
    page_table->clock.ns = remaining_ns;
  }
  else
    page_table->clock.ns += ns;
  
  printf("\n");

  printf("new time: %d sec, %d ns\n", page_table->clock.sec, page_table->clock.ns);

  Clock time_diff;
  time_diff.sec = sec;
  time_diff.ns = ns;

  return time_diff;
}

int wait_time_is_up(Clock next_fork) {
  // implement as critical section
  // compare next_sec and next_ns with what's in the process table
  if(next_fork.sec < page_table->clock.sec) {
    return 0;
  }
  if(next_fork.sec == page_table->clock.sec) {
    if(next_fork.ns < page_table->clock.ns) {
      return 0;
    }
  }

  return -1; // -1 means not
}

Clock generate_next_child_fork() {
  Clock next_fork;
  int random_ms = getRandom(500) + 1; // 1-500 milliseconds
  int ns = random_ms * MS_NS_CONVERSION;
  // set next__fork to current clock
  next_fork.sec = page_table->clock.sec;
  next_fork.ns = page_table->clock.ns;
  if((next_fork.ns + ns) > NANOSECONDS) {
    int remainder_ns = next_fork.ns + ns;
    next_fork.sec++;
    next_fork.ns = remainder_ns;
  }
  else
    next_fork.ns = next_fork.ns + ns;

  return next_fork;
}