#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "config.h"
#include "clock.h"

// data structures
typedef struct {
  int queueid;
  Clock clock;

  int pages[32];
} PageTable;

// functions
void init_page_table();

#endif
