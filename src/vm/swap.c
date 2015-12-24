#include <bitmap.h>
#include "vm/swap.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "devices/block.h"

#include <stdio.h>

static struct block *swap_device;
static struct bitmap *block_bitmap;
static const int BLOCKS_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;
static struct lock swap_lock;

void
swap_init (void)
{
  swap_device = block_get_role (BLOCK_SWAP);
  block_bitmap = bitmap_create (block_size (swap_device) / BLOCKS_PER_PAGE);
  lock_init (&swap_lock);
}

// locate swap device memory at swap_index and write it into kpage
// delete swap device memory at swap_index
void
swap_in (int swap_index, void *kpage)
{
  // lazy loading
  if(swap_device==NULL) swap_init();
  
  ASSERT (swap_index != -1);
  ASSERT (bitmap_test(block_bitmap, swap_index));
  
  int i;

  lock_acquire (&swap_lock);
//  printf("[BLIN] start %d\n",swap_index);
  for(i=0;i<BLOCKS_PER_PAGE;++i)
    block_read (swap_device, swap_index * BLOCKS_PER_PAGE + i,
                kpage + BLOCK_SECTOR_SIZE * i);
  
  bitmap_set (block_bitmap, swap_index, false);
//  printf("[BLIN] done\n");
  lock_release (&swap_lock);
}

// allocate swap device memory and write kpage
// return swap_index of swap device memory
int
swap_out (void *kpage)
{
  // lazy loading
  if(swap_device==NULL) swap_init();
  
  ASSERT (!bitmap_all(block_bitmap,0,bitmap_size(block_bitmap)));
  
  int i, swap_index;

  lock_acquire (&swap_lock);
  swap_index = bitmap_scan_and_flip (block_bitmap, 0, 1, false);
  lock_release (&swap_lock);

  ASSERT ((size_t) swap_index < BITMAP_ERROR);
//  printf("[BLKW] start %d\n",swap_index);
  for(i=0;i<BLOCKS_PER_PAGE;++i)
    block_write (swap_device, swap_index * BLOCKS_PER_PAGE + i,
                 kpage + BLOCK_SECTOR_SIZE * i);
//  printf("[BLKW] done\n");

  return swap_index;
}

// delete swap device memory at swap_index
void 
swap_remove (int swap_index)
{
  // lazy loading
  if(swap_device==NULL) swap_init();
  ASSERT (swap_index != -1);
  
  lock_acquire (&swap_lock);
  bitmap_set (block_bitmap, swap_index, false);
  lock_release (&swap_lock);
}
