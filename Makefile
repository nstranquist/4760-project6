CC = gcc
.SUFFIXES: .c .o .h

all: oss user

oss: oss.o user.o page_table.o clock.o logger.o utils.o
	gcc -Wall -g -o oss oss.o page_table.o clock.o logger.o utils.o

user: user.o page_table.o logger.o utils.o
	gcc -Wall -g -o user user.o page_table.o logger.o utils.o

.c.o:
	$(CC) -g -c $<

clean:
	rm -f *.o oss user
