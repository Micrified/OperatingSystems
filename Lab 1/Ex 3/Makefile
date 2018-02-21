CC=gcc
CFLAGS=-std=c99 -Wall -Werror -Wunused-function 
all: duplicates.c 
	${CC} ${CFLAGS} -o duplicates duplicates.c

clean:
	rm -f *.o
	rm -f *~
	rm -rf *.dSYM
	rm -f *.output
	rm -f *.out
	rm -f duplicates