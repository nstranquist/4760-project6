#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "config.h"
#include "clock.h"

#define TOTAL_MEMORY 256
#define TOTAL_PAGES 32
#define PAGE_SIZE 1

// 
#define BYTE_SHIFT_OCCURANCE 8

#define FREE_FRAMES_THRESHOLD 0.10 // 10% of free frames

#define N_AMOUNT 0.05 // 5% of pages, marks oldest 'n' pages for replacement, but they are also reclaimable

/**
 * You can create fixed sized arrays for page tables, assuming that each process will have a requirement of less than 32K memory, with each page being 1K
 * 
 * The page table should also have a delimiter indicating its size so that the users processes do not access memory beyond the page table limit
 * 
 * The page table should have all the required fields that may be implemented by bits (preferable) or character data types. You may want to consider the bit fields in struct to implement those bits
 * 
 * Assume that your system has a total memory of 256K. Use a bit vector to keep track of unallocated frames.
 */

// our frame table will contain one byte of reference bits per frame (which is initially set to zero). When a reference
// is made to a frame, the most significant bit of that reference byte is set to 1

// data structures
typedef struct {
  // TODO: use struct for managing...
  // - get RefByte
  // - get DirtyBit
  int reference_byte; // 0-255, init to 0
  int dirty_bit; // 0 or 1

  // int occupied;
} Frame;

typedef struct {
  int queueid;
  Clock clock;

  int size; // so each process does not access memory beyond the page table limit
  int frame_size;

  Frame pages[32];

  int frames[256]; // bitvector
} PageTable;


// functions
void init_page_table();
Frame init_frame();

// bit vector functions
void init_frames_bitvector();
int find_in_bitvector(int byte);
int remove_from_bitvector(int byte);
int add_to_bitvector(int byte);
int get_next_frame();

#endif
