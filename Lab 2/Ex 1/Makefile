CC=gcc
CFLAGS=-O2 -Wall -Wunused-function 
all: scanner parser strtab.h strtab.c queue.h queue.c util.h util.c eval.c
	${CC} ${CFLAGS} -g lex.yy.c iwish.tab.c strtab.c queue.c util.c eval.c -o iwish -ll -lm

scanner: iwish.lex
	flex iwish.lex
parser: iwish.y
	bison -d -v iwish.y
clean:
	rm -f iwish.tab.c
	rm -f iwish.tab.h
	rm -f lex.yy.c
	rm -f *.o
	rm -f *~
	rm -rf *.dSYM
	rm -f *.output
	rm -f *.out
	rm iwish
