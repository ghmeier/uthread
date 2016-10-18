main: main.o uthread.o
	gcc -lpthread -g -o main main.o uthread.o

main.o: main.c uthread.h
	gcc -lpthread -g -c main.c

uthread.o: uthread.c uthread.h
	gcc -lpthread -g -c uthread.c