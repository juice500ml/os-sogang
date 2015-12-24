#include "vm/page.h"
#include "vm/swap.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/synch.h"

// List of all frames
static struct list all_frames = LIST_INITIALIZER (all_frames);
static struct lock page_lock;
static bool is_lock_init;

void
page_init (void)
{
  is_lock_init = true;
  lock_init (&page_lock);
}

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
  ASSERT (pg_round_down(upage)==upage);
  ASSERT (pg_round_down(kpage)==kpage);
  ASSERT (is_user_vaddr(upage));
  ASSERT (is_kernel_vaddr(kpage));

//  if(!is_lock_init) page_init ();
//  lock_acquire (&page_lock);
  struct thread *th = thread_current ();
  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  if(pagedir_get_page (th->pagedir, upage) == NULL
     && pagedir_set_page (th->pagedir, upage, kpage, writable)) {
      ASSERT (find_page_by_upage(upage) == NULL);

      // new kpage-upage
      // Traverse through every pages to find kpage
      // which is used by current thread.
      struct list_elem *e;
      for(e=list_begin(&all_frames); e!=list_end(&all_frames); e=list_next(e)) {
          struct frame *f = list_entry(e, struct frame, elem);
          // If found, map kpage and upage
          if(f->kpage == kpage && f->swap_index == -1) {
              struct page *p = malloc (sizeof(struct page));
              p->upage = upage;
              p->th = thread_current ();
              p->writable = writable;

              list_push_back (&f->page_list, &p->elem);
//              lock_release (&page_lock);
              return true;
          }
      }
  }
//  lock_release (&page_lock);
  return false;
}

// Add a kpage which is used by current thread.
void
add_frame (void *kpage)
{
  ASSERT (pg_round_down(kpage)==kpage);
  ASSERT (is_kernel_vaddr(kpage));

  struct frame *f = malloc (sizeof(struct frame));
  
  // add it to all_frames
  f->kpage = kpage;
  f->swap_index = -1;
  list_init (&f->page_list);
  
  if(!is_lock_init) page_init ();
  lock_acquire (&page_lock);
  list_push_back (&all_frames, &f->elem);
  lock_release (&page_lock);
}

// wrapper for palloc
// get user frame
void *
get_frame (enum palloc_flags flags)
{
  void *kpage = get_only_frame (flags);
  if (flags == PAL_USER || flags == (PAL_USER | PAL_ZERO))
    add_frame (kpage);
  return kpage;
}

void *
get_only_frame (enum palloc_flags flags)
{
  void *kpage = palloc_get_page (flags);

  // eviction needed
  if(kpage == NULL) {
      struct frame *f = candidate_frame ();
      ASSERT (f != NULL);
      ASSERT (f->swap_index == -1);
      f->swap_index = swap_out (f->kpage);
     
//      if(!is_lock_init) page_init ();
//      lock_acquire (&page_lock);
      // clear upage
      struct list_elem *e;
      for(e=list_begin(&f->page_list); e!=list_end(&f->page_list); e=list_next(e)) {
          struct page *p = list_entry(e, struct page, elem);
          pagedir_clear_page (p->th->pagedir, p->upage);
      }
//      lock_release (&page_lock);
      // clear kpage
      palloc_free_page (f->kpage);
      // get new page
      kpage = palloc_get_page (flags);
  }
  ASSERT (kpage != NULL);
  return kpage;
}

// remove every relation within thread t
void
destroy_page_by_thread (struct thread *t)
{
  struct list_elem *fe;
  for(fe=list_begin(&all_frames); fe!=list_end(&all_frames); ) {
      struct frame *f = list_entry(fe, struct frame, elem);
      struct list_elem *ftmp = list_next(fe);
      struct list_elem *e;
      for(e=list_begin(&f->page_list); e!=list_end(&f->page_list); ) {
          struct page *p = list_entry(e, struct page, elem);
          struct list_elem *tmp = list_next(e);
          if(p->th == t) {
              pagedir_clear_page (p->th->pagedir, p->upage);
              list_remove (&p->elem);
              free (p);
          }
          e = tmp;
      }
      if(list_empty(&f->page_list)) {
          if(f->swap_index != -1)
            swap_remove (f->swap_index);
          else
            palloc_free_page (f->kpage);
          list_remove (&f->elem);
          free (f);
      }
      fe = ftmp;
  }
}

// find upage and return page
struct page *
find_page_by_upage (void *upage)
{
  ASSERT (pg_round_down(upage)==upage);
 
  struct thread *th = thread_current ();

  // Traverse through every pages to find upage
  // which is used by current thread.
  struct list_elem *fe;
  for(fe=list_begin(&all_frames); fe!=list_end(&all_frames); fe=list_next(fe)) {
      struct frame *f = list_entry(fe, struct frame, elem);
      struct list_elem *e;
      for(e=list_begin(&f->page_list); e!=list_end(&f->page_list); e=list_next(e)) {
          struct page *p = list_entry(e, struct page, elem);
          if(p->th == th && p->upage == upage) return p;
      }
  }
  return NULL;
}

// find upage and return frame
struct frame *
find_frame_by_upage (void *upage)
{
  ASSERT (pg_round_down(upage)==upage);
 
  struct thread *th = thread_current ();

  // Traverse through every pages to find upage
  // which is used by current thread.
  struct list_elem *fe;
  for(fe=list_begin(&all_frames); fe!=list_end(&all_frames); fe=list_next(fe)) {
      struct frame *f = list_entry(fe, struct frame, elem);
      struct list_elem *e;
      for(e=list_begin(&f->page_list); e!=list_end(&f->page_list); e=list_next(e)) {
          struct page *p = list_entry(e, struct page, elem);
          if(p->th == th && p->upage == upage) return f;
      }
  }
  return NULL;
}

// returns frame of eviction candidate frame
struct frame *
candidate_frame (void)
{
//  if(!is_lock_init) page_init ();
  while(true) {
      struct list_elem *fe = list_pop_front (&all_frames);
      struct frame *f = list_entry(fe, struct frame, elem);
      bool is_accessed = false;
      if(f->swap_index == -1) {
          struct page *p = list_entry (list_front(&f->page_list), struct page, elem);
          if(pagedir_is_accessed(p->th->pagedir, p->upage)) {
              is_accessed = true;
              pagedir_set_accessed(p->th->pagedir, p->upage, false);
          }
      }
      list_push_back (&all_frames, fe);
      if(f->swap_index == -1 && !is_accessed) {
          return f;
      }
  }
  return NULL;
}
