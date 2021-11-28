#ifndef CLOCK_H
#define CLOCK_H

#define NANOSECONDS 1000000000
#define MILISECONDS 1000
#define MS_NS_CONVERSION 1000000
#define MS_TO_US 1000

typedef struct {
  int sec;
  int ns;
} Clock;

void init_clock();
Clock increment_clock_round();
Clock add_time_to_clock(int sec, int ns);


#endif