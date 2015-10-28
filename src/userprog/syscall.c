#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "lib/kernel/list.h"
#include "devices/input.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "process.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  int syscall_number = *((int*)f->esp);
  // unknown issue: bit shifted 16 bits. don't know why.
  void *esp = f->esp + 16;
  switch(syscall_number)
  {
    case  SYS_HALT  : syscall_halt(); break;
    case  SYS_EXIT  : syscall_exit(*(int*)(f->esp+4)); break;
    case  SYS_EXEC  : f->eax = syscall_exec(*(const char**)(f->esp+4)); break;
    case  SYS_WAIT  : syscall_wait(*(pid_t*)(f->esp+4)); break;
    case  SYS_READ  : f->eax = syscall_read(*(int*)(esp+4),
                                       *(void**)(esp+8),
                                       *(unsigned*)(esp+12)); break;
    case  SYS_WRITE : f->eax = syscall_write(*(int*)(esp+4),
                                       *(const void**)(esp+8),
                                       *(unsigned*)(esp+12)); break;
    case  SYS_FIBO  : f->eax = syscall_fibonacci(*(int*)(f->esp+4)); break;
    case  SYS_SUM4  : f->eax = syscall_sum_of_four_integers(*(int*)(esp+8), *(int*)(esp+12), *(int*)(esp+16), *(int*)(esp+20)); break;
    default   : syscall_exit(-1);
  }
}

void
syscall_halt (void)
{
  shutdown_power_off();
}

void
syscall_exit (int status)
{
  int i;
  struct thread *current = thread_current();
  char *name = current->name;
  for(i=0;i<16;++i)
    if(name[i]==' ') break;
  name[i] = '\0';
  printf("%s: exit(%d)\n", current->name, status);
  current->return_status = status;
  sema_up(&current->parent->sema);
  printf("sema normal!\n");
  thread_exit();
  printf("syscall_exit normal!\n");
  return;
}

pid_t
syscall_exec (const char *cmdline)
{
  if(!is_user_vaddr(cmdline))
    syscall_exit(-1);
  return process_execute(cmdline);
}

int
syscall_wait (pid_t pid)
{
  return process_wait(pid);
}

int
syscall_read (int fd, void *buf, unsigned size)
{
  if(!is_user_vaddr(buf) || !is_user_vaddr(buf+size))
    syscall_exit(-1);
  if(fd==0)
    {
      unsigned i;
      for(i=0;i<size;++i)
        *((uint8_t*) buf + i) = input_getc();
      return size;
    }
  return -1;
}

int
syscall_write (int fd, const void *buf, unsigned size)
{
  if(!is_user_vaddr(buf) || !is_user_vaddr(buf+size))
    syscall_exit(-1);
  if(fd==1)
    {
      unsigned i;
      for(i=0;i<size;++i)
        if(!*((char*)(buf)+i)) break;
      putbuf(buf, i);
      return i;
    }
  return -1;
}

int
syscall_fibonacci (int n)
{
  int f[3] = {0,1,0};
  int i;
  for(i=2;i<=n;++i)
    f[n%3] = f[(n+1)%3] + f[(n+2)%3];
  printf("%d:result\n",f[n%3]);
  return f[n%3];
}

int
syscall_sum_of_four_integers (int a, int b, int c, int d)
{
  return a+b+c+d;
}
