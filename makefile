manin: test.o uthread.o
	gcc -lpthread -g -o test test.o uthread.o

main.o: test.c uthread.h
	gcc -lpthread -g -c test.c

uthread.o: uthread.c uthread.h
	gcc -lpthread -g -c uthread.c
