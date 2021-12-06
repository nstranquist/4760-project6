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

  
  // ... start logic


  
  // sleep(1);

  printf("Exiting user...\n");

  return 0;
}
