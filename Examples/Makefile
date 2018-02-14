CC=cc
CFLAGS=-D_MINIX
all: brk execve fork group signal1 signal2 suspend \
     kill sigmask suicide pipe stdio files lseek \
     stat dup fcntl domount dounmount

brk: brk.c
	${CC} ${CFLAGS} -o brk brk.c
execve: execve.c
	${CC} ${CFLAGS} -o execve execve.c
fork: fork.c
	${CC} ${CFLAGS} -o fork fork.c
group: group.c
	${CC} ${CFLAGS} -o group group.c
signal1: signal1.c
	${CC} ${CFLAGS} -o signal1 signal1.c
signal2: signal2.c
	${CC} ${CFLAGS} -o signal2 signal2.c
sigmask: sigmask.c
	${CC} ${CFLAGS} -o sigmask sigmask.c
suspend: suspend.c
	${CC} ${CFLAGS} -o suspend suspend.c
kill: kill.c
	${CC} ${CFLAGS} -o kill kill.c
suicide: suicide.c
	${CC} ${CFLAGS} -o suicide suicide.c
pipe: pipe.c
	${CC} ${CFLAGS} -o pipe pipe.c
stdio: stdio.c
	${CC} ${CFLAGS} -o stdio stdio.c
files: files.c
	${CC} ${CFLAGS} -o files files.c
lseek: lseek.c
	${CC} ${CFLAGS} -o lseek lseek.c
stat: stat.c
	${CC} ${CFLAGS} -o stat stat.c
dup: dup.c
	${CC} ${CFLAGS} -o dup dup.c
fcntl: fcntl.c
	${CC} ${CFLAGS} -o fcntl fcntl.c
domount: domount.c
	${CC} ${CFLAGS} -o domount domount.c
dounmount: dounmount.c
	${CC} ${CFLAGS} -o dounmount dounmount.c
clean:
	rm -f *~
	rm -f brk execve fork group signal1 signal2 suspend
	rm -f kill sigmask suicide pipe stdio files lseek
	rm -f stat dup fcntl domount dounmount
