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
  int syscall_number = *((int*)f->eip);
  switch(syscall_number)
  {
    case  0   : break;
    case  1   : break;
    case  2   : break;
    case  3   : break;
    case  4   : break;
    case  5   : break;
    case  6   : break;
    case  7   : break;
    case  8   : break;
    case  9   : break;
    case  10  : break;
    case  11  : break;
    case  12  : break;
    case  20  : break;
    case  21  : break;
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
  struct list_elem *l = &(thread_current()->allelem);
  
  // find head of thread list
  while(l->prev != NULL) l = list_prev(l);
  // traverse through all thread
  for(l=list_next(l); l->next != NULL; )
  {
    struct list_elem *next = l=list_next(l);
    list_remove(l);
    free(l);
  }

  thread_exit();
  return;
}

pid_t
syscall_exec (const char *cmdline)
{

}

int
syscall_wait (pid_t pid)
{

}

bool
syscall_create (const char *file, unsigned initial_size)
{

}

bool
syscall_remove (const char *file)
{

}

int
syscall_open (const char *file)
{

}

int
syscall_filesize (int fd)
{

}

int
syscall_read (int fd, void *buf, unsigned size)
{

}

int
syscall_write (int fd, const void *buf, unsigned size)
{

}

void
syscall_seek (int fd, unsigned position)
{

}

unsigned
syscall_tell (int fd)
{

}

void
syscall_close (int fd)
{

}
