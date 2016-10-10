#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h> 
#include <ucontext.h>
#include <sys/types.h>
#include <time.h>

// used so we don't have to dynamically expand the
// queue which seemed out of the scope of the project
#define DEFAULT_Q 20

// structure for all user thread related data
typedef struct uthread{
    int priority;
    time_t start;
    int pid;
    void (*func)();
    ucontext_t* ctx;
} uthread;

// maintained priority queue for waiting user threads
typedef struct uthread_q{
    int size;
    int max;
    int* q;
} uthread_q;

// managed active and waiting threads
typedef struct uthread_t{
    uthread_q* q;
    uthread** threads;
    int size;
    int kactive;
    int kmax;
} uthread_t;
uthread_t* uthread_ptr;

/* private initialization functions */
uthread_q*  uthread_q_init();

/* queue management functions */
void        enqueue(uthread_q* q_ptr, int t_index);
int         dequeue(uthread_q* q_ptr);
void        swap(uthread_q* q_ptr, int i, int j);
int         pri(uthread_q* q_ptr, int i);

// thread list functions
uthread*    get_uthread(int t_index);
int         get_uthread_from_pid(int pid);
void        start_thread(int t_index);

// cleanup functions
void        uthread_release();
void        uthread_q_release(uthread_q* q_ptr);

void uthread_init(int numKernelThreads) {
    /* allocate global thread pointer */
    uthread_ptr = (uthread_t*)malloc(sizeof(uthread_t));
    /* allocate thread list */
    uthread_ptr->threads = (uthread**)malloc(DEFAULT_Q*sizeof(uthread*));
    /* initial size should be zero */
    uthread_ptr->size = 0;
    uthread_ptr->kactive = 1;
    uthread_ptr->kmax = numKernelThreads;

    uthread_ptr->q = uthread_q_init();
}

int uthread_create(void (* func)()) {
    uthread* t = (uthread*)malloc(sizeof(uthread));
    t->priority = uthread_ptr->q->size;
    t->pid = -1;
    t->func = func;

    // initialize a context for the user thread
    t->ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
    getcontext(t->ctx);
    t->ctx->uc_stack.ss_sp = (char*)malloc(16*16384);
    t->ctx->uc_stack.ss_size = 16*16384; 
    makecontext(t->ctx,t->func,0);

    // place thread reference in the thread list
    int t_index = uthread_ptr->size;
    uthread_ptr->threads[t_index] = t;
    uthread_ptr->size++;

    if (uthread_ptr->kactive < uthread_ptr->kmax){
         // start a kernal thread if there are open, active threads
        start_thread(t_index);
    } else {
        // place the context in the queue otherwise
        enqueue(uthread_ptr->q, t_index);
    }
}

void uthread_exit() {
    // if the queue is empty, we can exit the thread
    if (uthread_ptr->q->size == 0) {  
        uthread_ptr->kactive--;
        // clean up if there are no more active kthreads
        // adds some instability and debug difficutly, so 
        // it's commented out
        //if (uthread_ptr->kactive == 0) {
        //    uthread_release();
        //}
        exit(0);
    }

    // set a thread's id, and start time
    uthread* t = get_uthread(dequeue(uthread_ptr->q));
    t->pid = getpid();
    t->start = time(NULL);
    // set context to this one since we're done with the old
    setcontext(t->ctx);
}

void uthread_yield() {
    int t_index = get_uthread_from_pid(getpid());
    time_t y_time = time(NULL);

    if (t_index == -1) {
        // this *should* only happen if cloning or the queue get messed up
        printf("ERROR: no thread for pid %d\n",getpid());
        return;
    }

    // reset values of the current thread, for the queue
    uthread* t = get_uthread(t_index);
    t->pid = -1;
    t->priority += y_time - t->start;
    enqueue(uthread_ptr->q, t_index);

    // get the next highest priority thread and activate it
    int next_t_index = dequeue(uthread_ptr->q);
    uthread* next_t = get_uthread(next_t_index);
    next_t->pid = getpid();
    next_t->start = y_time;

    // swap the current and next thread
    swapcontext(t->ctx,next_t->ctx);
}

// just to make syntax clearer throughout
uthread* get_uthread(int t_index) {
    return uthread_ptr->threads[t_index];
}

// searches through uthread references for one matching the
// provided pid
int get_uthread_from_pid(int pid) {
    for (int i=0;i<uthread_ptr->size;i++) {
        if (get_uthread(i)->pid == pid) {
            return i;
        }
    }

    return -1;
}

uthread_q* uthread_q_init(){
    uthread_q* q = (uthread_q*)malloc(sizeof(uthread_q));
    q->max = DEFAULT_Q;
    q->size = 0;
    q->q = (int*)malloc(q->max*sizeof(int*));
}

void enqueue(uthread_q* q_ptr, int t_index) {
    // we can't add to a full queue
    if (q_ptr->size >= q_ptr->max) {
        printf("ERROR: queue is full, increase DEFAULT_Q.\n");
        return;
    }

    // basic percolate up for a heap based on priority.
    int i=q_ptr->size;
    int parent = (i-1)/2;
    q_ptr->q[i] = t_index;
    q_ptr->size++;
    while (i>0 && pri(q_ptr,i) < pri(q_ptr,parent)) {
        swap(q_ptr,i,parent);
        i = parent;
        parent = (i-1)/2;
    }
}

int dequeue(uthread_q* q_ptr) {
    //empty queue is empty
    if (q_ptr->size == 0){
        return -1;
    }

    // basic pop then percolate down based on priority
    int t_index = q_ptr->q[0];
    q_ptr->q[0] = q_ptr->q[q_ptr->size-1];
    q_ptr->size--;
    int i=0;
    while (i < q_ptr->size-1) {
        if (q_ptr->size <= 2*i+2 && pri(q_ptr,i) > pri(q_ptr,2*i+1)) {
            swap(q_ptr,i,2*i+1);
            i=2*i+1;
        }else if (pri(q_ptr,2*i+1) < pri(q_ptr,2*i+2)) {
            swap(q_ptr,i,2*i+1);
            i = 2*i+1;
        }else{
            swap(q_ptr,i,2*i+2);
            i = 2*i+2;
        }
    }
    return t_index;
}

// swap two items in the queue at positions i & j
void swap(uthread_q* q_ptr, int i, int j) {
    int p = q_ptr->q[i];
    q_ptr->q[i] = q_ptr->q[j];
    q_ptr->q[j] = p;
}

// a shortcut for getting a uthread's priority
int pri(uthread_q* q_ptr, int i) {
    return get_uthread(q_ptr->q[i])->priority;
}

void start_thread(int t_index) {
    uthread* t = get_uthread(t_index);
    if (t->pid != -1) {
        /* t is already running */
        return;
    }

    // this kthread is going to be active, so show it
    uthread_ptr->kactive++;
    // create a new kthread to run this uthread
    t->pid = clone((int(*)(void*))t->func, t->ctx->uc_stack.ss_sp,CLONE_VM, NULL);
    if (t->pid == -1) {
        // some notification if we error when cloning
        printf("ERROR: failed to clone thread\n");
        enqueue(uthread_ptr->q,t_index);
        return;
    }
}

void uthread_release() {
    for (int i=0;i<uthread_ptr->size;i++) {
        free(uthread_ptr->threads[i]->ctx->uc_stack.ss_sp);
        free(uthread_ptr->threads[i]->ctx);
        free(uthread_ptr->threads[i]);
    }
    free(uthread_ptr->threads);
    uthread_q_release(uthread_ptr->q);
    free(uthread_ptr);
}

void uthread_q_release(uthread_q* q_ptr) {
    free(q_ptr->q);
    free(q_ptr);
}
