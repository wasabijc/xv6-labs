#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if (argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;

  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

#ifdef LAB_PGTBL
int sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  // 提取参数
  struct proc *p = myproc();
  uint64 usrpge_ptr; // 待检测页表起始指针
  int npage;         // 待检测页表个数
  uint64 useraddr;   // 写入用户内存的地址
  argaddr(0, &usrpge_ptr);
  argint(1, &npage);
  argaddr(2, &useraddr);
  if (npage > 64) // 页面数上限
  {
    return -1;
  }
  uint64 bitmap = 0; // 存储页面访问位
  uint64 mask = 1;   // 位操作掩码
  uint64 complement = PTE_A;
  complement = ~complement; // comp是PTE_A的补码
  int count = 0;

  for (uint64 page = usrpge_ptr; page < usrpge_ptr + npage * PGSIZE; page += PGSIZE)
  {
    // walk函数返回页表项指针
    pte_t *pte = walk(p->pagetable, page, 0);
    if (*pte & PTE_A) // 该页表项被访问过
    {
      bitmap = bitmap | (mask << count); // 更新位图，对应位设置1
      *pte = (*pte) & complement;        // 清除页表项的PTE_A位
    }
    count++;
  }
  copyout(p->pagetable, useraddr, (char *)&bitmap, sizeof(bitmap));
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
