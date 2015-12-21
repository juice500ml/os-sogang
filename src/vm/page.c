#include "vm/page.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails.
   Moved to here from process.h */
bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *th = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (th->pagedir, upage) == NULL
          && pagedir_set_page (th->pagedir, upage, kpage, writable));
}
