#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <debug.h>
#include <stdbool.h>
#include <stdint.h>
#include <list.h>
#include "threads/palloc.h"

struct frame
  {
    void *kpage;
    int swap_index;
    struct list page_list;
    struct list_elem elem;
  };

struct page
  {
    void *upage;
    struct thread *th;
    bool writable;
    struct list_elem elem;
  };

void page_init (void);
bool install_page (void *upage, void *kpage, bool writable);
void add_frame (void *kpage);
void *get_frame (enum palloc_flags flags);
void *get_only_frame (enum palloc_flags flags);
void destroy_page_by_thread (struct thread *t);
struct page *find_page_by_upage (void *upage);
struct frame *find_frame_by_upage (void *upage);
struct frame *candidate_frame (void);

#endif /* vm/page.h */
