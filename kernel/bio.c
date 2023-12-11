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
#define BUCKETS 13
#define HASH(x) (x % BUCKETS)

struct
{
  struct spinlock global_lock;
  struct spinlock bucket_lock[BUCKETS];
  struct buf buf[NBUF];
  struct buf head[BUCKETS];
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
} bcache;

void binit(void)
{
  struct buf *b;

  initlock(&bcache.global_lock, "bcache");

  // Create linked list of buffers
  for (int i = 0; i < BUCKETS; i++)
  {
    initlock(&bcache.bucket_lock[i], "bcache.bucket");
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }
  for (b = bcache.buf; b < bcache.buf + NBUF; b++)
  {
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }
}

void move_buffer_to_bucket(struct buf *b, int hash)
{
  // int ehash = HASH(b->blockno);
  // // remove from original bucket
  // buf->next->prev = &bcache.head[ehash];
  // bcache.head[ehash].next = buf->next;
  // // add to new bucket
  // buf->next = bcache.head[hash].next;
  // buf->prev = &bcache.head[hash];
  // bcache.head[hash].next = buf;
  // buf->next->prev = buf;

  b->next->prev = b->prev;
  b->prev->next = b->next;
  b->next = bcache.head[hash].next;
  b->prev = &bcache.head[hash];
  bcache.head[hash].next->prev = b;
  bcache.head[hash].next = b;
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer

static struct buf *
bget(uint dev, uint blockno)
{
  struct buf *b;

  int hash = HASH(blockno);

  acquire(&bcache.bucket_lock[hash]);

  // Is the block already cached?
  for (b = bcache.head[hash].next; b != &bcache.head[hash]; b = b->next)
  {
    if (b->dev == dev && b->blockno == blockno)
    {
      b->refcnt++;
      release(&bcache.bucket_lock[hash]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bcache.bucket_lock[hash]);
  acquire(&bcache.global_lock);
  acquire(&bcache.bucket_lock[hash]);

  for (b = bcache.head[hash].next; b != &bcache.head[hash]; b = b->next)
  {
    if (b->dev == dev && b->blockno == blockno)
    {
      b->refcnt++;
      release(&bcache.bucket_lock[hash]);
      release(&bcache.global_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.

  for (int i = 0; i < NBUF; i++)
  {
    b = &bcache.buf[i];
    int ehash = HASH(b->blockno);
    if (ehash != hash)
    {
      acquire(&bcache.bucket_lock[ehash]);
    }
    if (b->refcnt == 0)
    {
      if (ehash != hash)
      {
        move_buffer_to_bucket(b, hash);
        release(&bcache.bucket_lock[ehash]);
      }
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.global_lock);
      release(&bcache.bucket_lock[hash]);
      acquiresleep(&b->lock);
      return b;
    }
    if (ehash != hash)
    {
      release(&bcache.bucket_lock[ehash]);
    }
  }
  release(&bcache.bucket_lock[hash]);
  release(&bcache.global_lock);

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf *
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if (!b->valid)
  {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void bwrite(struct buf *b)
{
  if (!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void brelse(struct buf *b)
{
  if (!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int hash = HASH(b->blockno);

  acquire(&bcache.bucket_lock[hash]);
  b->refcnt--;
  release(&bcache.bucket_lock[hash]);
}

void bpin(struct buf *b)
{
  acquire(&bcache.bucket_lock[HASH(b->blockno)]);
  b->refcnt++;
  release(&bcache.bucket_lock[HASH(b->blockno)]);
}

void bunpin(struct buf *b)
{
  acquire(&bcache.bucket_lock[HASH(b->blockno)]);
  b->refcnt--;
  release(&bcache.bucket_lock[HASH(b->blockno)]);
}
