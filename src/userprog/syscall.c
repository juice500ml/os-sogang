#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "lib/kernel/list.h"

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
    case  0   : break;
    case  1   : syscall_exit(*(int*)(esp+4)); break;
    case  2   : break;
    case  3   : break;
    case  4   : break;
    case  5   : break;
    case  6   : break;
    case  7   : break;
    case  8   : break;
    case  9   : f->eax = syscall_write(*(int*)(esp+4), *(const void**)(esp+8), *(unsigned*)(esp+12)); break;
    case  10  : break;
    case  11  : break;
    case  12  : break;
    case  20  : break;
    case  21  : f->eax = *(int*)(esp+4) + *(int*)(esp+8) + *(int*)(esp+12) + *(int*)(esp+16); break;
    default   : syscall_exit(-1);
  }
}

// 3.1.5 accessing user memory: p.27 example code
int
read_byte (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
      : "=&a" (result) : "m" (*uaddr));
  return result;
}

// 3.1.5 accessing user memory: p.27 example code
bool
write_byte (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
      : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

void
syscall_exit (int status)
{
  int i;
  char *name = thread_current()->name;
  for(i=0;i<16;++i)
    if(name[i]==' ') break;
  name[i] = '\0';
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit();
  return;
}

pid_t
syscall_exec (const char *cmdline UNUSED)
{

}

int
syscall_wait (pid_t pid UNUSED)
{

}

int
syscall_read (int fd UNUSED, void *buf UNUSED, unsigned size UNUSED)
{

}

int
syscall_write (int fd, const void *buf, unsigned size)
{
  if(fd==1)
    {
      unsigned i;
      for(i=0;i<size;++i)
        if(!*((char*)(buf)+i)) break;
      putbuf(buf, i);
      return i;
    }
}
