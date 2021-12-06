CC = gcc
.SUFFIXES: .c .o .h

all: oss user

oss: oss.o user.o page_table.o clock.o logger.o utils.o msgqueue.o semaphore_manager.o
	gcc -Wall -g -o oss oss.o page_table.o clock.o logger.o utils.o msgqueue.o semaphore_manager.o

user: user.o page_table.o logger.o utils.o msgqueue.o semaphore_manager.o
	gcc -Wall -g -o user user.o page_table.o logger.o utils.o msgqueue.o semaphore_manager.o

.c.o:
	$(CC) -g -c $<

clean:
	rm -f *.o oss user
