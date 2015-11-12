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
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "threads/synch.h"

static void syscall_handler (struct intr_frame *);
// find file in filelist of current thread and return
static struct file *find_file (int fd);
// used lock for file synch
static struct lock filelock;

void
syscall_init (void) 
{
  lock_init(&filelock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  if(f->esp==NULL || !is_user_vaddr(f->esp))
    syscall_exit(-1);
  
  // TODO: clean up this mess.
  int syscall_number = *((int*)f->esp);
  switch(syscall_number)
  {
    case  SYS_HALT  : syscall_halt(); break;
    case  SYS_EXIT  : if(!is_user_vaddr(f->esp+4)) syscall_exit(-1);
                      syscall_exit(*(int*)(f->esp+4)); break;
    case  SYS_EXEC  : if(!is_user_vaddr(f->esp+4)) syscall_exit(-1);
                      f->eax = syscall_exec(*(const char**)(f->esp+4)); break;
    case  SYS_WAIT  : if(!is_user_vaddr(f->esp+4)) syscall_exit(-1);
                      f->eax = syscall_wait(*(pid_t*)(f->esp+4)); break;
    case  SYS_CREATE: if(!is_user_vaddr(f->esp+8)) syscall_exit(-1);
                      f->eax = syscall_create(*(const char**) (f->esp+4),
                                              *(unsigned*) (f->esp+8)); break;
    case  SYS_REMOVE: if(!is_user_vaddr(f->esp+4)) syscall_exit(-1);
                      f->eax = syscall_remove(*(const char**)(f->esp+4)); break;
    case  SYS_OPEN  : if(!is_user_vaddr(f->esp+4)) syscall_exit(-1);
                      f->eax = syscall_open(*(const char**)(f->esp+4)); break;
    case  SYS_FILESIZE  : if(!is_user_vaddr(f->esp+4)) syscall_exit(-1);
                      f->eax = syscall_filesize(*(int*)(f->esp+4)); break;
    case  SYS_READ  : if(!is_user_vaddr(f->esp+12)) syscall_exit(-1);
                      f->eax = syscall_read(*(int*)(f->esp+4),
                                       *(void**)(f->esp+8),
                                       *(unsigned*)(f->esp+12)); break;
    case  SYS_WRITE : if(!is_user_vaddr(f->esp+12)) syscall_exit(-1);
                      f->eax = syscall_write(*(int*)(f->esp+4),
                                       *(const void**)(f->esp+8),
                                       *(unsigned*)(f->esp+12)); break;
    case  SYS_SEEK  : if(!is_user_vaddr(f->esp+8)) syscall_exit(-1);
                      syscall_seek(*(int*)(f->esp+4),
                                   *(unsigned*)(f->esp+8)); break;
    case  SYS_TELL  : if(!is_user_vaddr(f->esp+4)) syscall_exit(-1);
                      f->eax = syscall_tell(*(int*)(f->esp+4)); break;
    case  SYS_CLOSE : if(!is_user_vaddr(f->esp+4)) syscall_exit(-1);
                      syscall_close(*(int*)(f->esp+4)); break;
    case  SYS_FIBO  : if(!is_user_vaddr(f->esp+4)) syscall_exit(-1);
                      f->eax = syscall_fibonacci(*(int*)(f->esp+4)); break;
    case  SYS_SUM4  : if(!is_user_vaddr(f->esp+16)) syscall_exit(-1);
                      f->eax = syscall_sum_of_four_integers(*(int*)(f->esp+4),
                                                            *(int*)(f->esp+8),
                                                            *(int*)(f->esp+12),
                                                            *(int*)(f->esp+16)); break;
    default         : syscall_exit(-1);
  }
}

// finder for file in list
static struct file *
find_file (int fd)
{
  struct thread *current = thread_current();
  struct list_elem *e, *found = NULL;

  // look for fd in filelist
  for(e=list_begin(&current->filelist); e!=list_end(&current->filelist); e = list_next(e))
    {
      if(list_entry(e, struct filewrapper, fileelem)->fd==fd)
        {
          found = e;
          break;
        }
    }
  // not found
  if(found == NULL) return NULL;

  return list_entry(found, struct filewrapper, fileelem)->f;

}

void
syscall_halt (void)
{
  // 3.3.4 description from 3.3.4 System Calls void halt(void).
  shutdown_power_off();
  return;
}

void
syscall_exit (int status)
{
  int i;
  struct thread *current = thread_current();
  char *name = current->name;

  // name has all the parameters too, so ignore others.
  for(i=0;i<16;++i)
    if(name[i]==' ') break;
  name[i] = '\0';

  // traverse through child list to exit for child before parent exit.
  struct list_elem *e;
  for(e=list_begin(&current->childlist); e!=list_end(&current->childlist); e = list_next(e))
    {
      struct thread *child = list_entry(e, struct thread, childelem);
      sema_up(&child->sema_wait);
    }

  // exit print.
  printf("%s: exit(%d)\n", current->name, status);
  current->is_done = true;
  current->return_status = status;
  // wait for parent to take over the result
  sema_down(&current->sema_wait);
  // call thread_exit to clean.
  thread_exit();
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

bool
syscall_create (const char *file, unsigned initial_size)
{
  if(!is_user_vaddr(file) || file==NULL)
    syscall_exit(-1);
  
  lock_acquire(&filelock);  
  bool ret = filesys_create(file, initial_size);
  lock_release(&filelock);

  return ret;
}

bool
syscall_remove (const char *file)
{
  if(!is_user_vaddr(file))
    syscall_exit(-1);
  
  lock_acquire(&filelock);  
  bool ret = filesys_remove(file);
  lock_release(&filelock);
  
  return ret;
}


int
syscall_open (const char *file)
{
  if(!is_user_vaddr(file) || file==NULL)
    syscall_exit(-1);
 
  lock_acquire(&filelock);  
  struct file *f = filesys_open(file);
  lock_release(&filelock);
  
  // open failed
  if (f==NULL) return -1;
  
  struct thread *t = thread_current();
 
  // MAX FILE DESC.
  if(t->nextfd > MAX_FILE_FD) return -1;
  
  struct filewrapper *fw = malloc(sizeof(struct filewrapper));
  
  fw->f = f;
  fw->fd = t->nextfd++;
  
  list_push_back(&t->filelist, &fw->fileelem);
  return fw->fd;
}

int
syscall_filesize (int fd)
{
  struct file *f = find_file(fd);
  if(f==NULL) return -1;
  
  lock_acquire(&filelock);  
  int ret = file_length(f);
  lock_release(&filelock);

  return ret;
}

int
syscall_read (int fd, void *buf, unsigned size)
{
  if(!is_user_vaddr(buf) || !is_user_vaddr(buf+size))
    syscall_exit(-1);
  // stdin
  if(fd==0)
    {
      unsigned i;
      for(i=0;i<size;++i)
        *((uint8_t*) buf + i) = input_getc();
      return size;
    }
  else
    {
      struct file *f = find_file(fd);
      if(f==NULL) return -1;
  
      lock_acquire(&filelock);  
      int ret = file_read(f, buf, size);
      lock_release(&filelock);

      return ret;
    }
  return -1;
}

int
syscall_write (int fd, const void *buf, unsigned size)
{
  if(!is_user_vaddr(buf) || !is_user_vaddr(buf+size))
    syscall_exit(-1);
  // stdout
  if(fd==1)
    {
      unsigned i;
      for(i=0;i<size;++i)
        if(!*((char*)(buf)+i)) break;
      putbuf(buf, i);
      return i;
    }
  else
    {
      struct file *f = find_file(fd);
      if(f==NULL) return -1;
      
      lock_acquire(&filelock);  
      int ret = file_write(f, buf, size);
      lock_release(&filelock);

      return ret;
    }
  return -1;
}

void
syscall_seek (int fd, unsigned position)
{
  struct file *f = find_file(fd);
  if(f==NULL) return;
  
  lock_acquire(&filelock);  
  file_seek(f, position);
  lock_release(&filelock);
}

int
syscall_tell (int fd)
{  struct file *f = find_file(fd);
  if(f==NULL) return -1;
 
  lock_acquire(&filelock);  
  int ret = file_tell(f);
  lock_release(&filelock);
  
  return ret;
}

void
syscall_close (int fd)
{
  struct thread *current = thread_current();
  struct list_elem *e, *found = NULL;
  
  // look for fd in filelist
  for(e=list_begin(&current->filelist); e!=list_end(&current->filelist); e = list_next(e))
    {
      if(list_entry(e, struct filewrapper, fileelem)->fd==fd)
        {
          found = e;
          break;
        }
    }
  // not found
  if(found == NULL) return;
  
  // if possible, swap with last list entry
  e = list_rbegin(&current->filelist);
  if(e!=list_end(&current->filelist))
    {
      list_insert(found, e);
      list_entry(e, struct filewrapper, fileelem)->fd = fd;
    }
  list_remove(found);
  free(list_entry(found, struct filewrapper, fileelem));
  current->nextfd--;
}

int
syscall_fibonacci (int n)
{
  int f[3] = {0,1,0};
  int i;
  for(i=2;i<=n;++i)
    f[i%3] = f[(i+1)%3] + f[(i+2)%3];
  return f[n%3];
}

int
syscall_sum_of_four_integers (int a, int b, int c, int d)
{
  return a+b+c+d;
}
