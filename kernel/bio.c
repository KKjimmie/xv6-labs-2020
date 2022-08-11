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

#define NBUCKET 13

struct bufbucket
{
  struct buf head;
  struct spinlock lock;
};

struct {
  struct buf buf[NBUF];
  struct bufbucket bucket[NBUCKET];
} bcache;

void
binit(void)
{
  struct buf *b;
  char lockname[10];
  char *fmt = "bcache_%d";

  for(int i = 0; i < NBUCKET; i++){
    // initialize lock
    snprintf(lockname, sizeof(lockname), fmt, i);
    initlock(&bcache.bucket[i].lock, "lockname");
    bcache.bucket[i].head.prev = &bcache.bucket[i].head;
    bcache.bucket[i].head.next = &bcache.bucket[i].head;
  }

  // link the buffers to table[0]
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.bucket[0].head.next;
    b->prev = &bcache.bucket[0].head;
    initsleeplock(&b->lock, "buffer");
    bcache.bucket[0].head.next->prev = b;
    bcache.bucket[0].head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int bucketindex = blockno%NBUCKET;

  acquire(&bcache.bucket[bucketindex].lock);

  // Is the block already cached?
  for(b = bcache.bucket[bucketindex].head.next; b != &bcache.bucket[bucketindex].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      // update timestamp
      b->timestamp = ticks;
      release(&bcache.bucket[bucketindex].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  struct buf *tmpbuf;
  b = 0;
  // Recycle the least recently used (LRU) unused buffer according to timestamp.
  // search from current tableindex.
  for(int i = bucketindex,cycle = 0; cycle < 13; i = (i + 1)%NBUCKET, cycle ++){
    if(!holding(&bcache.bucket[i].lock)){
      acquire(&bcache.bucket[i].lock);
    }
    for(tmpbuf = bcache.bucket[i].head.prev; tmpbuf != &bcache.bucket[i].head; tmpbuf = tmpbuf->prev){
      if((tmpbuf->refcnt == 0) && (b==0 || tmpbuf->timestamp < b->timestamp)) {
        b = tmpbuf;
      }
    }
    if(b){
      if(i != bucketindex){
        b->next->prev = b->prev;
        b->prev->next = b->next;
        release(&bcache.bucket[i].lock);
        // insert into cuttent tableindex
        b->next = bcache.bucket[bucketindex].head.next;
        b->prev = &bcache.bucket[bucketindex].head;
        bcache.bucket[bucketindex].head.next->prev = b;
        bcache.bucket[bucketindex].head.next = b;
      }
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      b->timestamp = ticks;
      release(&bcache.bucket[bucketindex].lock);
      acquiresleep(&b->lock);
      return b;
    } else {
      if(i != bucketindex){
        release(&bcache.bucket[i].lock);
      }
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
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  int bucketindex = b->blockno%NBUCKET;

  releasesleep(&b->lock);

  acquire(&bcache.bucket[bucketindex].lock);
  b->refcnt--;
  // update timestamp
  b->timestamp = ticks;
  
  release(&bcache.bucket[bucketindex].lock);
}

void
bpin(struct buf *b) {
  int bucketindex = b->blockno%NBUCKET;
  acquire(&bcache.bucket[bucketindex].lock);
  b->refcnt++;
  release(&bcache.bucket[bucketindex].lock);
}

void
bunpin(struct buf *b) {
  int bucketindex = b->blockno%NBUCKET;
  acquire(&bcache.bucket[bucketindex].lock);
  b->refcnt--;
  release(&bcache.bucket[bucketindex].lock);
}


