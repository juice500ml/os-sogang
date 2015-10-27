#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/user/syscall.h"
#include <stdint.h>

void syscall_init (void);

// 3.3.4 system calls implementation
int read_byte (const uint8_t *uaddr);
bool write_byte (uint8_t *udst, uint8_t byte);
void syscall_exit (int status);
pid_t syscall_exec (const char *cmdline);
int syscall_wait (pid_t pid);
int syscall_read (int fd, void *buf, unsigned size);
int syscall_write (int fd, const void *buf, unsigned size);

#endif /* userprog/syscall.h */
