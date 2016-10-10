# uthreads library

A many-to-many mapping of user threads to kernel threads.

## Usage Instructions
```c
$ make
$ ./main
```

The provided makefile compiles `uthread.c` and `main.c` to `uthread.o` and `main.o` which are then used for `main`. 

## Notes

`uthread.h` contains the function definitions provided in the spec.

`uthread.c` contains some additional function proptotypes as well as impimentation details for the user thread library. 

`main.c` has the example provided in the project spec of thread usage

There's been an inconsistent bug in the code where the `clone()` command can
corrupt the stored global variable `uthread_ptr`. I'd assume this behavior is a 
reason to avoid using global variables in multithreaded programs, especially 
ones where you have to manually manage the memory. I've done my best to mitigate
the issues throughout. From what I've seen, the issue occurs when either trying 
to `getpid()` from a cloned process, or using `clone()` sometimes. From the 
research I've looked at, there doesn't seem like an easy way around this. So, 
it's probably best to use `pthreads` for production applications ;)

Moral of the story, if you get a Seg-Fault, try to run it 5-10 more times. :)