#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .global name; \
  name: \
    mov $SYS_ ## name, %rax; \
    mov %rcx, %r10 ;\
    syscall               ;\
    ret

#define SYSCALL_NUM(name, num) \
  .global name; \
  name: \
    mov $num, %rax; \
    mov %rcx, %r10 ;\
    syscall               ;\
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
SYSCALL_NUM(__getpid_syscall, SYS_getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(vmprint)
