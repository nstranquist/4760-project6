#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "config.h"

// data structures
typedef struct {
  int hello;
  int world;
} page_table;

// functions
void init_table();

#endif
