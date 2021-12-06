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
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <getopt.h>
#include "config.h"
#include "page_table.h"
#include "logger.h"
#include "utils.h"
#include "clock.h"
#include "msgqueue.h"
#include "semaphore_manager.h"

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

// variables
extern PageTable *page_table;
extern char *logfile; // oss.log
int shmid;
int semid;
struct sembuf semsignal[1];
struct sembuf semwait[1];

Clock next_fork;
Clock time_diff;


// functions
void cleanup();
int detachandremove(int shmid, void *shmaddr);
void print_memory_layout();
// void generate_report();

static void myhandler(int signum) {
  // is ctrl-c interrupt
  if(signum == SIGINT)
    perror("\noss: Ctrl-C Interrupt Detected. Shutting down gracefully...");
  // is timer interrupt
  else if(signum == SIGALRM)
    perror("\noss: Info: The time for this program has expired. Shutting down gracefully...");
  else {
    perror("\noss: Warning: Only Ctrl-C and Timer signal interrupts are being handled.");
    return; // ignore the interrupt, do not exit
  }

  fprintf(stderr, "interrupt handler\n");

  // generate_report();

  cleanup();
  
  pid_t group_id = getpgrp();
  if(group_id < 0)
    perror("oss: Info: group id not found");
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

// The statistics of interest are:
// • Number of memory accesses per second
// • Number of page faults per memory access
// • Average memory access speed
// • Number of seg faults per memory access

int main(int argc, char*argv[]) {
  printf("Hello project 6!\n");

  int option;
  int n_programs = -1;
  int nextFork;
  int process_count = 0;
  int total_processes_count = 0;
  char *logfileName = NULL;
  int current_sec = 0; // to detect every 1 logical sec to print out the current memory layout


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
        logfileName = optarg;
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

  if(logfileName == NULL) {
    logfileName = "oss.log";
  }

  printf("N programs: %d\n", n_programs);


  // Setup Timers and Interrupts
  if (setupinterrupt() == -1) {
    perror("oss: Error: Could not run setup the interrupt handler.");
    return -1;
  }
  if (setupitimer(MAX_SECONDS) == -1) {
    perror("oss: Error: Could not setup the interval timer.");
    return -1;
  }

  // Setup intterupt handler
  signal(SIGINT, myhandler);

  // init logfile
  if(init_logfile(logfileName) == -1) {
    perror("oss: Error: Could not init logfile");
    return 1;
  }

  // seed the random
  srand(time(NULL));


  // allocate shared memory
  shmid = shmget(IPC_PRIVATE, sizeof(PageTable), PERMS | 0666);
  if (shmid == -1) {
    perror("oss: Error: Failed to create shared memory segment for page table");
    return -1;
  }
  
  // attach shared memory
  page_table = (PageTable *)shmat(shmid, NULL, 0);
  if (page_table == (void *) -1) {
    perror("oss: Error: Failed to attach page table to shared memory");
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
      perror("oss: Error: Failed to remove memory segment containing page table");
    return -1;
  }

  // Initialize Message Queue
  int queueid = initqueue(IPC_PRIVATE);
  if(queueid == -1) {
    perror("oss: Error: Failed to initialize message queue\n");
    cleanup();
    return -1;
  }
  page_table->queueid = queueid;

  // Create semaphore containing a single element
  if((semid = semget(IPC_PRIVATE, 1, PERMS)) == -1) {
    perror("oss: Error: Failed to create private semaphore\n");
    return 1;
  }

  setsembuf(semwait, 0, -1, 0); // decrement first element of semwait
  setsembuf(semsignal, 0, 1, 0); // increment first element of semsignal

  // initialize semaphore before use
  if(initelement(semid, 0, 1) == -1) {
    perror("oss: Error: Failed to init semaphore element value to 1\n");
    if(removesem(semid) == -1)
      perror("oss: Error: Failed to remove failed semaphore\n");
    return 1;
  }

  // Start program timer
  alarm(MAX_SECONDS);
  

  // init page table
  init_page_table();

  // init clock
  page_table->clock = init_clock();


  // generate next time // as cs
  // wait_sem(semid, semwait, 1);
  next_fork = generate_next_child_fork();
  // signal_sem(semid, semsignal, 1);

  // Check that there's never more than n_programs processes in the system at once
  // --> Use semaphore.

  // oss should also print its memory map every logical second showing the allocation of frames.
  // --> You can display unallocated frames by a period (·) and allocated frame by a +.

  // Main Logic loop
  while(total_processes_count < MAX_TOTAL_PROCESSES_MOCK) { // MAX_TOTAL_PROCESSES
    fprintf(stderr, "\nIn loop! %d total processes, %d running\n", total_processes_count, process_count);

    // manage / evaluate clock
    if(wait_time_is_up(next_fork) == -1) {
      fprintf(stderr, "oss is waiting to generate fork new child process\n");
      increment_clock_round();
      if(page_table->clock.sec > current_sec) {
        printf("logical second increase detected!\n");
        print_memory_layout();
        current_sec = page_table->clock.sec;
      }

      continue;
    }

    // before forking, check if current active processes < 18
    // IF >= 18, report this, increment the clock, and continue the loop
    if(process_count >= MAX_RUNNING_PROCESSES) {
      printf("oss: Warning: Max active processes reached. Skipping this round\n");

      increment_clock_round();
      if(page_table->clock.sec > current_sec) {
        printf("logical second increase detected!\n");
        print_memory_layout();
        current_sec = page_table->clock.sec;
      }
      continue;
    }
    if(total_processes_count > MAX_TOTAL_PROCESSES_MOCK) {
      printf("oss: Warning: Max total processes reached. Skipping this round\n");
      continue;
    }

    printf("It is time to fork a child!\n");

    // increase total processes before child fork
    process_count++;
    total_processes_count++;

    // fork, excl, etc.
    pid_t child_pid = fork();

    if (child_pid == -1) {
      perror("oss: Error: Failed to fork a child process");
      cleanup();
      return -1;
    }

    if (child_pid == 0) {
      printf("will execute child\n");

      // parse shmid
      int shmid_length = snprintf( NULL, 0, "%d", shmid );
      char* shmid_str = malloc( shmid_length + 1 );
      snprintf( shmid_str, shmid_length + 1, "%d", shmid );

      // execl: NOTE: CAN PASS THE NS AND MS HERE
      execl("./user", "./user", shmid_str, (char *) NULL); // 1 arg: pass shmid
      perror("oss: Error: Child failed to execl");
      cleanup();
      exit(0);
    }
    else {
      // parent waits inside loop for child to finish
      int status;
      pid_t wpid = waitpid(child_pid, &status, WNOHANG);

      if (wpid == -1) {
        perror("oss: Error: Failed to wait for child");
        cleanup();
        return 1;
      }
      else if(wpid == 0) {
        fprintf(stderr, "child is still running\n");

      }
      else {
        fprintf(stderr, "A child has finished\n");

        // When a process terminates, oss should log its termination in the log file and also indicate its effective memory access time.


        process_count--; // when the process as finished
      }
    }

    time_diff = increment_clock_round();
    if(page_table->clock.sec > current_sec) {
      printf("logical second increase detected!\n");
      print_memory_layout();
      current_sec = page_table->clock.sec;
    }

    sleep(1); // to debug
  }

  // wait for all children to finish
  while(wait(NULL) > 0) {
    printf("oss: Info: Waiting for all children to finish...\n");
  }

  // generate_report();

  cleanup();

  return 0;
}

// void generate_report() {
//   printf("generating report...\n");
// }

void cleanup() {
  fprintf(stderr, "oss: cleaning up shared memory and shared system data structures...\n");

  // remove msgqueue
  if(remmsgqueue(page_table->queueid) == -1) {
    perror("oss: Error: Failed to remove message queue");
  }
  else printf("success remove msgqueue\n");

  // remove semaphore
  if(removesem(semid) == -1) {
    perror("runsim: Error: Failed to remove semaphore");
  }
  else printf("sucess remove sem\n");

  // remove shared memory
  if(detachandremove(shmid, page_table) == -1) {
    perror("oss: Error: Failure to detach and remove memory for page table");
  }
  else printf("success detatch\n");
}

// From textbook
int detachandremove(int shmid, void *shmaddr) {
  int error = 0;

  if (shmdt(shmaddr) == -1) {
    fprintf(stderr, "oss: Error: Can't detach memory\n");
    error = errno;
  }
  
  if ((shmctl(shmid, IPC_RMID, NULL) == -1) && !error) {
    fprintf(stderr, "oss: Error: Can't remove shared memory\n");
    error = errno;
  }

  if (!error)
    return 0;

  errno = error;

  return -1;
}

void print_memory_layout() {
  // "current memory layout at time xxx:xxxx is:"
  // - Occupied, RefByte, DirtyBit
  // - Frame 0, Frame 1, Frame 2, Frame...

  // field values:
  // - "no/yes"
  // - 0-256
  // - 0-1

  // print current memory layout
  printf("Current memory layout at time %d:%d is:\n", page_table->clock.sec, page_table->clock.ns);
  printf("-\t\tOccupied\tRefByte\tDirtyBit\n");

  for(int i = 0; i < page_table->frame_size; i++) {
    char *occupied;
    if(find_in_bitvector(page_table->pages[i].reference_byte) == 0) {
      occupied = "yes";
    }
    else {
      occupied = "no";
    }
    printf("Frame %d:\t\t%s\t%d\t%d\n", i, occupied, page_table->pages[i].reference_byte, page_table->pages[i].dirty_bit);
  }
}

// NEED FOR MESSAGE HANDLER:
// For each message type, a certain "handler" function is setup to receive and handle the message appropriately
// May have several different message types, and they should all be documented.
// Could attach pid of process wanting message, and check if matches, and if doesn't match, then ignore, put msg back in queue, and read next
