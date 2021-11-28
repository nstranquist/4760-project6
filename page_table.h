#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "config.h"

// data structures
typedef struct {
  int hello;
  int world;
} PageTable;

// functions
void init_page_table();

#endif
