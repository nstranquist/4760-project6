#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "msgqueue.h"

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)


int initqueue(int key) {
  int queueid = msgget(key, PERMS | IPC_CREAT);

  if (queueid == -1) {
    perror("os: Error: could not initialize queue\n");
    return -1;
  }

  return queueid;
}

int remmsgqueue(int queueid) {
  return msgctl(queueid, IPC_RMID, NULL);
}

int msgwrite(void *buf, int len, int msg_type, int queueid) {     /* output buffer of specified length */
  int error = 0;
  mymsg_t *mymsg;

  if ((mymsg = (mymsg_t *)malloc(sizeof(mymsg_t) + len - 1)) == NULL) {
    perror("oss: Error: Could not allocate space for message");
    return -1;
  }

  memcpy(mymsg->mtext, buf, len);

  mymsg->mtype = msg_type; // 1 or 2
  if (msgsnd(queueid, mymsg, len, 0) == -1) {
    perror("oss: Error: Could not send the message");
    error = errno;
  }

  free(mymsg);

  if (error) {
    errno = error;
    return -1;
  }

  return 0;
}