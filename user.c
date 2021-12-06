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

#define PROBABILITY_INVALID_REQUEST 5 // small chance

extern PageTable *page_table;
int shmid;

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


  // 2. The user process will wait on its semaphore (or message queue) that will be signaled by oss
    // --> oss checks the page reference by extracting the page number from the address,
    // --> increments the clock as specified above,
    // --> and sends a signal on the semaphore if the page is valid.

  // 3. Processes should have a small probability to request an invalid memory request. In this case 
    // --> the process should simply make a request for some memory that is outside of its legal page table.
    // --> oss should detect this and deal with it by terminating the process.
    // --> This should be indicated in the log.

  // 4. At random times the user process will check whether it should terminate.
    // --> say every 1000 ± 100 memory references
    // --> If so, all its memory should be returned to oss
    // --> oss should be informed of its termination.

  
  // sleep(1);

  printf("Exiting user...\n");

  return 0;
}
