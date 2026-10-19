#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
// Assignment 5: User-space system call stubs
SYSCALL(shm_get)
SYSCALL(sem_init)
SYSCALL(sem_up)
SYSCALL(sem_down)
// Assignment 7: User-space system call stubs
SYSCALL(clone)
SYSCALL(join)
SYSCALL(sem_wait)
SYSCALL(sem_signal)
SYSCALL(set_priority)
SYSCALL(get_priority)
// Assignment 8: User-space system call stubs
SYSCALL(va2pa)
SYSCALL(getmeminfo)
SYSCALL(vmprint)

