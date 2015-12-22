#include "vm/frame.h"
#include "vm/page.h"
#include "userprog/pagedir.h"
#include "threads/palloc.h"

// wrapper for palloc
// get user frame
void *
get_frame(enum palloc_flags flags)
{
  void *kpage = palloc_get_page (flags);
  if (flags == PAL_USER || flags == (PAL_USER | PAL_ZERO))
    add_page (kpage);
  return kpage;
}
