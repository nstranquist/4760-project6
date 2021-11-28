#define _GNU_SOURCE // for asprintf
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <getopt.h>
#include "config.h"
#include "page_table.h"

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

// variables
extern PageTable *page_table;
int shmid;
char *logfile = NULL; // oss.log

// functions
void generate_report();
void cleanup();

static void myhandler(int signum) {
  // is ctrl-c interrupt
  if(signum == SIGINT)
    perror("\noss: Ctrl-C Interrupt Detected. Shutting down gracefully...\n");
  // is timer interrupt
  else if(signum == SIGALRM)
    perror("\noss: Info: The time for this program has expired. Shutting down gracefully...\n");
  else {
    perror("\noss: Warning: Only Ctrl-C and Timer signal interrupts are being handled.\n");
    return; // ignore the interrupt, do not exit
  }

  fprintf(stderr, "interrupt handler\n");

  generate_report();

  cleanup();
  
  pid_t group_id = getpgrp();
  if(group_id < 0)
    perror("oss: Info: group id not found\n");
  else
    killpg(group_id, signum);


  kill(getpid(), SIGKILL);
	exit(0);
  signal(SIGQUIT, SIG_IGN);
}

// interrupt handling
static int setupitimer(int sleepTime) {
  struct itimerval value;
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_usec = 0;
  value.it_value.tv_sec = sleepTime; // alarm
  value.it_value.tv_usec = 0;
  return (setitimer(ITIMER_PROF, &value, NULL));
}

static int setupinterrupt(void) {
  struct sigaction act;
  act.sa_handler = myhandler;
  act.sa_flags = 0;
  return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL) || sigaction(SIGALRM, &act, NULL));
}

static int timerHandler(int s) {
  int errsave;
  errsave = errno;
  write(STDERR_FILENO, "The time limit was reached\n", 1);
  errno = errsave;
}

int main(int argc, char*argv[]) {
  printf("Hello project 6!\n");

  int option;
  int n_programs = -1;
  int nextFork;

  // parse CLI arguments
  // - run oss with at least '-p' for number of processes. default to 20
  while((option = getopt(argc, argv, "hp:l:")) != -1) {
    switch(option) {
      case 'h':
        printf("\nProgram Help:\n");
        printf("Usage: ./oss [-h] [-p n] [-l f]\n");
        printf("Where h is help, n is the number of processes, and f is the logfile name\n");
        printf("If none are specified, it will resort to the defaults: n=20 processes and f=oss.log\n\n");
        return 0;
        break;
      case 'p':
        if(!atoi(optarg)) {
          perror("oss: Error: Cant convert number of processes to a number");
          return 1;
        }
        n_programs = atoi(optarg);
        if(n_programs < 0) {
          perror("oss: Error: number of programs cannot be less than 0");
          return 1;
        }
        break;
      case 'l':
        logfile = optarg;
        break;
      default:
        printf("default option reached\n");
        break;
    }
  }

  if(n_programs == -1) {
    printf("Setting n_programs to default\n");
    n_programs = NUMBER_PROCESSES;
  }

  if(logfile == NULL) {
    logfile = "oss.log";
  }

  printf("N programs: %d\n", n_programs);


  // Setup Timers and Interrupts
  if (setupinterrupt() == -1) {
    perror("oss: Error: Could not run setup the interrupt handler.\n");
    return -1;
  }
  if (setupitimer(MAX_SECONDS) == -1) {
    perror("oss: Error: Could not setup the interval timer.\n");
    return -1;
  }

  // Setup intterupt handler
  signal(SIGINT, myhandler);

  // setup logfile
  // Test that logfile can be used
  FILE *fp = fopen(logfile, "w");
  if(fp == NULL) {
    perror("oss: Error: Could not open log file for writing");
    return 1;
  }
  fprintf(fp, "Log Info for OSS Program:\n"); // clear the logfile to start
  fclose(fp);

  // seed the random
  srand(time(NULL));


  // allocate shared memory
  // shmid = shmget(IPC_PRIVATE, sizeof(PageTable), PERMS | 0666);
  // if (shmid == -1) {
  //   perror("oss: Error: Failed to create shared memory segment for page table\n");
  //   return -1;
  // }


  generate_report();

  cleanup();

  return 0;
}

void generate_report() {
  printf("generating report...\n");
}

void cleanup() {
  printf("cleaning up...\n");
}