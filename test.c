#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "uthread.h"

void th1() {
    int i;
    for(i=0;i<8;i++){
        printf("Thread  1:  run.\n");
        sleep(1);   //note: during  sleep   the user-level  thread  is  still   mapped
        printf("Thread  1:  yield.\n");
        uthread_yield();
    }
    printf("Thread  1:  exit.\n");
    uthread_exit();
}

void th2() {
    int i;
    for(i=0;i<4;i++){
        printf("Thread  2:  run.\n");
        sleep(2);
        printf("Thread  2:  yield.\n");
        uthread_yield();
    }
    printf("Thread  2:  exit.\n");
    uthread_exit();
}

void th3()  {
    int i;
    for(i=0;i<2;i++){
        printf("Thread  3:  run.\n");
        sleep(4);
        printf("Thread  3:  yield.\n");
        uthread_yield();
    }
    printf("Thread  3:  exit.\n");
    uthread_exit();
}

// provided example usage
int main() {
    uthread_init(5);
    uthread_create(th1);
    uthread_create(th2);
    uthread_create(th3);
    
    uthread_create(th1);
    uthread_create(th1);
    uthread_create(th2);

    uthread_create(th3);
    uthread_create(th1);
    uthread_create(th1);
    uthread_create(th1);
    uthread_create(th2);

    uthread_create(th3);
    uthread_create(th1);    
    uthread_exit();
    return 0;
}
