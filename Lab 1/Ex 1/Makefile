CC=gcc
CFLAGS=-std=c99 -Wall -Werror -Wunused-function 
all: shell.c 
	${CC} ${CFLAGS} -o execute shell.c

clean:
	rm -f *.o
	rm -f *~
	rm -rf *.dSYM
	rm -f *.output
	rm -f *.out
	rm -f execute