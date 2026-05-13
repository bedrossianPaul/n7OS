#ifndef __SYSCALL_DEFS_H__
#define __SYSCALL_DEFS_H__

#define NB_SYSCALL 10

int sys_example();
int sys_shutdown(int n);
int sys_write(const char *str, int len);
int sys_sleep(int ms);
int sys_getpid();
int sys_exit();
int sys_read(char *buffer, int size);
int sys_fork(char *name, void* fn);
int sys_kill(int pid);
int sys_time(long *tloc);

typedef int (*fn_ptr)();
extern fn_ptr syscall_table[NB_SYSCALL];

void add_syscall(int num, fn_ptr function);

#endif
