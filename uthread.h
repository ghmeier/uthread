/* Required specs for project 1 */
void uthread_init(int numKernelThreads);
int  uthread_create(void (* func)());
void uthread_yield();
void uthread_exit();
