#ifndef CONFIG_H
#define CONFIG_H

#define NUMBER_PROCESSES 20

#define MAX_TOTAL_PROCESSES 40
#define MAX_RUNNING_PROCESSES 18

#define MAX_SECONDS 50 // 5

#define NANOSECONDS 1000000000
#define MILISECONDS 1000
#define MS_NS_CONVERSION 1000000

#define LOGFILE_MAX_LINES 100000

#define MS_TO_US 1000 // convert MS to US for usleep() function in user.c

typedef struct {
  int sec;
  int ns;
} Clock;

#endif
