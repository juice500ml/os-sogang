#include <bitmap.h>
#include "vm/swap.h"
#include "threads/vaddr.h"
#include "devices/block.h"
#include "threads/synch.h"

static struct block *swap_device;
static struct bitmap *block_bitmap;
static struct lock swap_lock;
static const size_t BLOCKS_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;

void
swap_init (void)
{
  swap_device = block_get_by_name ("swap");
  block_bitmap = bitmap_create (block_size (swap_device) / BLOCKS_PER_PAGE);
}

// locate swap device memory at idx and write it into kpage
// delete swap device memory at idx
void
swap_in (size_t idx, void *kpage)
{
  lock_acquire(&swap_lock);

  lock_release(&swap_lock);
}

// allocate swap device memory and write kpage
// return idx of swap device memory
size_t
swap_out (void *kpage)
{
  lock_acquire(&swap_lock);

  lock_release(&swap_lock);
}
