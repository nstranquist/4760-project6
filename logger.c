#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "config.h"
#include "logger.h"

char* logfile = NULL;

int init_logfile(char* logfileName) {
  logfile = logfileName;

  // Test that logfile can be used
  FILE *fp = fopen(logfile, "w");
  if(fp == NULL) {
    return -1;
  }
  fprintf(fp, "Log Info for OSS Program:\n"); // clear the logfile to start
  fclose(fp);
  return 1;
}

void log_msg(const char* msg) {
  // log the message
}

// can have more helper functions like getting the formatted time, etc.
