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

#define NBUCKETS 13

struct buf_bucket {
  struct buf head;  
  struct spinlock lock;
};

struct {
  struct buf buf[NBUF];
  struct buf_bucket buckets[NBUCKETS];
} bcache;

void
binit(void)
{
  struct buf *b;
  char lockname[8];

  for (int i = 0; i < NBUCKETS; i++) {
    snprintf(lockname, sizeof(lockname), "bcache_%d", i);
    initlock(&bcache.buckets[i].lock, lockname);

    bcache.buckets[i].head.prev = &bcache.buckets[i].head;
    bcache.buckets[i].head.next = &bcache.buckets[i].head;
  }

  for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
    b->next = bcache.buckets[0].head.next;
    b->prev = &bcache.buckets[0].head;
    initsleeplock(&b->lock, "buffer");
    bcache.buckets[0].head.next->prev = b;
    bcache.buckets[0].head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int index = blockno % NBUCKETS;

  acquire(&bcache.buckets[index].lock);

  // Is the block already cached?
  for (b = bcache.buckets[index].head.next; b != &bcache.buckets[index].head; b = b->next) {
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;

      //更新时间戳
      acquire(&tickslock);
      b->timestamp = ticks;
      release(&tickslock);

      release(&bcache.buckets[index].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  b = 0;
  struct buf* tmp;

  for (int i = index,cycle=0;cycle!=NBUCKETS; i = (i + 1) % NBUCKETS) {
    cycle++;

    if (i != index) {
      if (!holding(&bcache.buckets[i].lock))
        acquire(&bcache.buckets[i].lock);
      else
        continue;
    }

    for (tmp = bcache.buckets[i].head.next; tmp != &bcache.buckets[i].head; tmp = tmp->next) {
      if (tmp->refcnt == 0 && (b == 0 || tmp->timestamp < b->timestamp))
        b = tmp;
    }

    if (b) {
      if (i != index) {
        b->next->prev = b->prev;
        b->prev->next = b->next;
        release(&bcache.buckets[i].lock);

        b->next = bcache.buckets[index].head.next;
        b->prev = &bcache.buckets[index].head;
        bcache.buckets[index].head.next->prev = b;
        bcache.buckets[index].head.next = b;
      }

      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;

      acquire(&tickslock);
      b->timestamp = ticks;
      release(&tickslock);

      release(&bcache.buckets[index].lock);
      acquiresleep(&b->lock);
      return b;
    }
    else {
      if (i != index)
        release(&bcache.buckets[i].lock);
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
  if(!holdingsleep(&b->lock))
    panic("brelse");

  int index = (b->blockno) % NBUCKETS;

  releasesleep(&b->lock);

  acquire(&bcache.buckets[index].lock);
  b->refcnt--;

  acquire(&tickslock);
  b->timestamp = ticks;
  release(&tickslock);

  
  release(&bcache.buckets[index].lock);
}

void
bpin(struct buf *b) {
  int index = (b->blockno) % NBUCKETS;

  acquire(&bcache.buckets[index].lock);
  b->refcnt++;
  release(&bcache.buckets[index].lock);
}

void
bunpin(struct buf *b) {
  int index = (b->blockno) % NBUCKETS;

  acquire(&bcache.buckets[index].lock);
  b->refcnt--;
  release(&bcache.buckets[index].lock);
}


