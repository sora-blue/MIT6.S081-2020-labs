// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUF_TABLE_SIZE 16

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  struct buf *buftable[BUF_TABLE_SIZE];
  struct spinlock btlock[BUF_TABLE_SIZE];
} bcache;

inline int
bthash(int bufno)
{
  return bufno % BUF_TABLE_SIZE;
}

// caller acquire lock
void
btinsert(struct buf *b, int idx)
{
  struct buf *head;

  head = bcache.buftable[idx];
  if(head == b){
    printf("Oops! Conflict!\n");
    return;
  }
  bcache.buftable[idx] = b;
  b->next = head;
}

// caller acquire lock
void
btremove(struct buf *b, int idx)
{
  struct buf *head;
  head = bcache.buftable[idx];
  if(!head){
    return;
  }
  if(head == b){
    bcache.buftable[idx] = b->next;
    return;
  }
  while(head && head->next != b){
    head = head->next;
  }
  if(head && head->next == b){
    head->next = b->next;
  }
}

void
binit(void)
{
  struct buf *b;
  int i;

  initlock(&bcache.lock, "bcache");
  
  for(i = 0 ; i < BUF_TABLE_SIZE ; i++){
    initlock(&bcache.btlock[i], "bcache");
  }

  for(b = bcache.buf ; b < bcache.buf + NBUF ; b++)
  {
    btinsert(b, 0);
    initsleeplock(&b->lock, "buffer");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int idx, i;

  idx = bthash(blockno);
  
  acquire(&bcache.btlock[idx]);

  // Is the block already cached?
  for(b = bcache.buftable[idx]; b ; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.btlock[idx]);
      
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  // * not lru for now
  // * planning for a heap per bucket
  // uint64 lru_min = -1;
  for(i = 0 ; i < BUF_TABLE_SIZE ; i++){
    if(i != idx){
      acquire(&bcache.btlock[i]);
    }
    for(b = bcache.buftable[i] ; b ; b = b->next){
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        if(i != idx){
          btremove(b, i);
          release(&bcache.btlock[i]);
          btinsert(b, idx);
        }
        release(&bcache.btlock[idx]);
        
        acquiresleep(&b->lock);
        return b;
      } 
    }
    if(i != idx){
      release(&bcache.btlock[i]);
    }
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  int idx;
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  idx = bthash(b->blockno);
  
  acquire(&bcache.btlock[idx]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->timestamp = ticks;
  }
  
  release(&bcache.btlock[idx]);
  
}

inline void
_bpins(struct buf *b, int offset)
{
  int idx; 
  idx = bthash(b->blockno);
  
  acquire(&bcache.btlock[idx]);
  b->refcnt += offset;
  release(&bcache.btlock[idx]);
  
}

void
bpin(struct buf *b) {
  _bpins(b, 1);
}

void
bunpin(struct buf *b) {
  _bpins(b, -1);
}


