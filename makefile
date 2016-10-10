main: main.o uthread.o
	gcc -g -o main main.o uthread.o

main.o: main.c uthread.h
	gcc -g -c main.c

uthread.o: uthread.c uthread.h
	gcc -g -c uthread.c