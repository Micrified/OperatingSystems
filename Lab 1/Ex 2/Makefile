CC=gcc
CFLAGS=-std=c99 -Wall -Werror -Wunused-function 
all: ring.c 
	${CC} ${CFLAGS} -o ring ring.c

clean:
	rm -f *.o
	rm -f *~
	rm -rf *.dSYM
	rm -f *.output
	rm -f *.out
	rm -f ring