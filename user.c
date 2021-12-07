#define _GNU_SOURCE  // for asprintf
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "config.h"
#include "utils.h"
#include "page_table.h"
#include "clock.h"
#include "msgqueue.h"

#define PERCENT_READS_WRITES 20

#define PROBABILITY_INVALID_REQUEST 1 // small chance

extern PageTable *page_table;
int shmid;
// for msg functions
int size;
char *buf;

mymsg_t mymsg;

mymsg_t init_msg(int type, char msg_text);

int main(int argc, char*argv[]) {
  printf("Hello project 6 from User\n");

  // re-seed the random
  srand(time(NULL) + getpid());

  // parse cli args from 'execl'
  // - shmid
  printf("arg1: %s\n", argv[1]);

  shmid = atoi(argv[1]);
  if(!shmid) {
    printf("error: user: Invalid shmid\n");
    exit(1);
  }

  // attach shared memory
  page_table = (PageTable *)shmat(shmid, NULL, 0);
  if (page_table == (void *) -1) {
    perror("oss: user: Error: Failed to attach to shared memory");
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
      perror("oss: user: Error: Failed to remove memory segment");
    return 0;
  }

  // Notes
  // - the percentage of reads vs writes should be configurable
  
  // Start Logic

  // 1. Generate random actual byte address
    // --> from 0 to the limit of process memory (256k?)

  // generate random # to see if invalid request
  int invalid_request = getRandom(100);
  if(invalid_request < PROBABILITY_INVALID_REQUEST) {
    printf("Invalid request selected in user.c\n");

    // send the invalid request, then wait for termination from oss
    int memory_request = 305; // will be outside the valid range for page table
    
  }

  int random_address;
  do {
    random_address = getRandom(256);
  }
  while(find_in_bitvector(random_address) == 0);

  printf("generated random, unused address: %d\n", random_address);

  // generate random # to see if read or write
  int random_num = getRandom(100);
  if(random_num < PERCENT_READS_WRITES) {
    // read
    printf("read\n");
    mymsg.mtype = 1;
    mymsg.mtext[0] = 'r';
    mymsg.mtext[1] = '\0';
  }
  else {
    // write
    printf("write\n");
    mymsg.mtype = 1;
    mymsg.mtext[0] = 'w';
    mymsg.mtext[1] = '\0';
  }

  // 2. The user process will wait on its semaphore (or message queue) that will be signaled by oss
    // --> oss checks the page reference by extracting the page number from the address,
    // --> increments the clock as specified above,
    // --> and sends a signal on the semaphore if the page is valid.

  // Send page to user with msgqueue
  char *buf;
  int msg_type = 1;
  asprintf(&buf, "info-%d-%d-%d", random_address, page_table->clock.sec, page_table->clock.ns);

  if((size = msgwrite(buf, MAX_MSG_SIZE, msg_type, page_table->queueid)) == -1) {
    perror("oss: user: Error: could not send message from user to oss");
    exit(0);
    return 0;
  }

  // receive the message back
  if((size = msgrcv(page_table->queueid, &mymsg, MAX_MSG_SIZE, msg_type, 0)) == -1) {
    perror("oss: user: Error: could not receive message from oss to user");
    return 0;
  }

  printf("received message from oss: %s\n", mymsg.mtext);

  // check if valid from msg
  printf("mymsg msg (in user from oss): %s\n", mymsg.mtext);

  // 3. Processes should have a small probability to request an invalid memory request. In this case 
    // --> the process should simply make a request for some memory that is outside of its legal page table.
    // --> oss should detect this and deal with it by terminating the process.
    // --> This should be indicated in the log.

  // 4. At random times the user process will check whether it should terminate.
    // --> say every 1000 ± 100 memory references
    // --> If so, all its memory should be returned to oss
    // --> oss should be informed of its termination.
  int should_check_to_terminate = -1;
  if(should_check_to_terminate != -1) {
    int should_terminate = getRandom(200);
    int n_memory_references = 950;
    if(should_terminate + 900 > n_memory_references) {
      printf("will terminate in user.c\n");
      // send terminate message


      // free all memory and exit
      if (shmdt(page_table) == -1) {
        perror("oss: user: Error: Failed to detach shared memory");
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
          perror("oss: user: Error: Failed to remove memory segment");
        return 0;
      }
      exit(0);
      return 0;
    }
  }
  
  // sleep(1);

  printf("Exiting user...\n");

  return 0;
}

mymsg_t init_msg(int type, char msg_text/*, int pid*/) {
  mymsg_t new_msg;
  new_msg.mtype = type;
  // new_msg.mtext = msg_text;
  // new_msg.pid = pid;
  return new_msg;
}