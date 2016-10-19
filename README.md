# uthreads library

A many-to-many mapping of user threads to kernel threads.

## Usage Instructions
```c
$ make
$ ./test
```

The provided makefile compiles `uthread.c` and `main.c` to `uthread.o` and `main.o` which are then used for `main`. 

## Notes

`uthread.h` contains the function definitions provided in the spec.

`uthread.c` contains some additional function proptotypes as well as impimentation details for the user thread library. 

`test.c` has the example provided in the project spec of thread usage

