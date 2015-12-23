#include "vm/page.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/malloc.h"

// List of all pages
static struct list all_pages = LIST_INITIALIZER (all_pages);

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails.
   Moved to here from userprog/process.h. */
bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *th = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  if(pagedir_get_page (th->pagedir, upage) == NULL
     && pagedir_set_page (th->pagedir, upage, kpage, writable)) {
      // Traverse through every pages to find kpage
      // which is used by current thread.
      struct list_elem *e;
      for(e=list_begin(&all_pages); e!=list_end(&all_pages); e=list_next(e)) {
          struct page *p = list_entry(e, struct page, elem);
          // If found, map kpage and upage
          if(p->kpage==kpage && p->th == th) {
              p->upage = upage;
              p->writable = writable;
              return true;
          }
      }
  }
  return false;
}

// Add a kpage which is used by current thread.
void
add_page (void *kpage)
{
  struct page *p = NULL;
  
  // find for empty entry
  struct list_elem *e;
  for(e=list_begin(&all_pages); e!=list_end(&all_pages); e=list_next(e)) {
    struct page *tmp = list_entry(e, struct page, elem);
    if(tmp->th==NULL) {
        p = tmp;
        break;
    }
  }

  if(p==NULL) p = malloc(sizeof(struct page));
  p->upage = NULL;
  p->kpage = kpage;
  p->th = thread_current ();
  list_push_back (&all_pages, &p->elem);
}

// remove every relation within thread t
void
destroy_page_by_thread (struct thread *t)
{
  struct list_elem *e;
  for(e=list_begin(&all_pages); e!=list_end(&all_pages); e=list_next(e)) {
      struct page *p = list_entry(e, struct page, elem);
      // if p->th is NULL, this entry is empty
      if(p->th == t) p->th = NULL;
  }
}
