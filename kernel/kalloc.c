// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];//为每个cpu分配一把锁

void
kinit()
{ 
  for(int i=0;i<NCPU;i++)//初始化
    initlock(&kmem[i].lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int cur = cpuid();
  acquire(&kmem[cur].lock);
  r->next = kmem[cur].freelist;
  kmem[cur].freelist = r;
  release(&kmem[cur].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();//关闭中断,防止多个cpu同时操作
  int cur = cpuid();//当前cpu
  acquire(&kmem[cur].lock);
  r = kmem[cur].freelist;//取空闲列表的表头页
  if(r)//如果有空闲页，取出，更新空闲列表
    kmem[cur].freelist = r->next;
  else{//没有空闲页
    for(int i=0;i<NCPU;i++){//遍历其他cpu的空闲列表
      if(i == cur) continue;
      acquire(&kmem[i].lock);
      //取走空闲页
      if(kmem[i].freelist){
        r = kmem[i].freelist;//r为表头空闲页
        kmem[i].freelist = r->next;//更新空闲列表
        release(&kmem[i].lock);
        break;
      }
      release(&kmem[i].lock);
    }
  }
  release(&kmem[cur].lock);
  pop_off();//开启中断

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
