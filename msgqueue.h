#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#define MAX_MSG_SIZE 4096

typedef struct mymsg_t {
  // message defaults
  long mtype;
  char mtext[MAX_MSG_SIZE];
} mymsg_t;



int initqueue(int key);
int remmsgqueue(int queueid);

int msgwrite(void *buf, int len, int msg_type, int queueid);


#endif