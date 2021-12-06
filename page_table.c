#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "page_table.h"

PageTable *page_table;

// functions
void init_page_table() {
  printf("init page_table\n");

  page_table->size = 256;
  page_table->frame_size = 32;

  // for each page, init frame to 0
  for(int i=0; i<page_table->size; i++) {
    page_table->pages[i] = init_frame();
  }

  printf("frames bit at 56: %d\n", page_table->pages[56].reference_byte);
  init_frames_bitvector();
}

Frame init_frame() {
  Frame frame;
  frame.reference_byte = 0;
  frame.dirty_bit = 0;
  return frame;
}

// bitvector functions
void init_frames_bitvector() {
  // init the frames, all to 0
  for(int i=0; i<page_table->size; i++) {
    page_table->frames[i] = -1;
  }
}

int find_in_bitvector(int byte) {
  if(byte > page_table->size || byte < 0) {
    fprintf(stderr, "error: byte %d out of range\n", byte);
    return -1;
  }

  // check if byte is in bitvector
  int found = -1;

  for(int i=0; i<page_table->size; i++) {
    if(page_table->frames[i] == byte) {
      found = 0;
      return found;
    }
  }

  return found;
}

int remove_from_bitvector(int byte) {
  if(byte > page_table->size || byte < 0) {
    fprintf(stderr, "error: byte out of range\n");
    return -1;
  }
  
  // will remove first occurence only
  int found = -1;

  for(int i=0; i<page_table->size; i++) {
    if(page_table->frames[i] == byte) {
      found = 0;
      page_table->frames[i] = -1;
      return found;
    }
  }

  return found;
}

int add_to_bitvector(int byte) {
  if(byte > page_table->size || byte < 0) {
    fprintf(stderr, "error: byte out of range\n");
    return -1;
  }
  
  // will add to the first empty spot
  int found = -1;

  for(int i=0; i<page_table->size; i++) {
    if(page_table->frames[i] == -1) {
      found = 0;
      page_table->frames[i] = byte;
      return found;
    }
  }

  return found;
}

int get_next_frame() {
  int found = -1;

  // will get the next frame value by index
  for(int i=0; i<page_table->size; i++) {
    if(page_table->frames[i] == -1) {
      found = i;
      page_table->frames[i] = i;
      return found;
    }
  }

  return found;
}
