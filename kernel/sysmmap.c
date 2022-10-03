#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "fcntl.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

struct vma *valloc(struct proc *p)
{
  struct vma *vpool = p->vpool;
  for(int i = 0 ; i < VMA_POOL_SIZE ; i++){
    if(vpool[i].status == VMA_FREE){
      vpool[i].status = VMA_USED;
      return vpool+i;
    }
  }
  return 0;
}

uint64
sys_mmap(void)
{
  uint64 addr;
  int length, prot, flags, fd, offset;

  int pte_flags;
  uint64 nsz;
  struct vma *v;
  struct file *f;
  struct proc *p = myproc();

  // -- read arguments --
  if(argaddr(0, &addr) < 0)
    return -1;
  if(argint(1, &length) < 0 || argint(2, &prot) < 0)
    return -1;
  if(argint(3, &flags) < 0 || argint(4, &fd) < 0)
    return -1;
  if(argint(5, &offset) < 0)
    return -1;
  // -- check arguments -- 
  if(length < 0)
    return -1;
  if(offset < 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f = myproc()->ofile[fd]) == 0)
    return -1;
  // You can assume that addr will always be zero, 
  // meaning that the kernel should decide the virtual
  // address at which to map the file. 
  if(addr != 0)
    return -1;
  // (prot, flags) to PTE flags
  // PTE flags should be stored in VMA
  pte_flags = 0;
  if(prot & PROT_NONE) pte_flags |= 0;
  if(prot & PROT_READ){
    if(!f->readable) return -1;
    pte_flags |= PTE_R;
  } 
  if(prot & PROT_WRITE){
    if(!f->writable && flags != MAP_PRIVATE) return -1;
    pte_flags |= PTE_W;
  } 
  if(prot & PROT_EXEC) pte_flags |= PTE_X;
  // flags can only be MAP_SHARED or MAP_PRIVATE
  if(flags == MAP_SHARED){
    pte_flags |= PTE_MAP_SHARED;
  } else if(flags == MAP_PRIVATE){
    pte_flags |= PTE_MAP_PRIVATE;
  } else {
    return -1;
  }
  // Find an unused region to map the file
  // Fill in the page table lazily, in response to page faults.
  addr = PGROUNDUP(p->sz);
  nsz = addr + length;
  // for(nsz = addr ; nsz < nsz+length ; nsz += PGSIZE){
  //   if(mappages(p->pagetable, nsz, PGSIZE, -1, PTE_V | PTE_U) < 0){
  //     uvmdealloc(p->pagetable, nsz, p->sz);
  //     return -1;
  //   }
  // }
  // Keep track of what mmap has mapped for each process.
  //  The VMA should contain a pointer to a struct file for the file
  //  being mapped; mmap should increase the file's reference count
  //  so that the structure doesn't disappear when the file is closed.
  v = valloc(p);
  if(v == 0)
    panic("mmap: no free vma");
  v->addr = addr;
  v->length = length;
  v->pte_flags = pte_flags;
  v->f = filedup(f);
  p->sz = nsz;

  return addr;
}

uint64
sys_munmap(void)
{
  uint64 addr;
  int length, i, n, r;
  struct vma *pv;
  struct proc *p = myproc();
  // -- read arguments --
  if(argaddr(0, &addr) < 0 || argint(1, &length))
    return -1;
  // -- check arguments --
  if(length < 0)
    return -1;
  // find corresponding vma for it
  for(i = 0 ; i < VMA_POOL_SIZE ; i++){
    pv = p->vpool + i;
    if(pv->status == VMA_FREE) continue;
    if(pv->addr > addr || pv->addr + pv->length < addr + length) continue;
    break;
  }
  if(i == VMA_POOL_SIZE)
    return -1;
  n = (PGROUNDUP(addr + length) - PGROUNDDOWN(addr));
  // If an unmapped page has been modified 
  // and the file is mapped MAP_SHARED, 
  // write the page back to the file.
  if(pv->pte_flags & PTE_MAP_SHARED){
     int off = PGROUNDDOWN(addr) - pv->addr;
     int max = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * BSIZE;
     for(i = 0; i < n; i += r){
       int n1 = n - i;
       if(n1 > max)
        n1 = max;

      begin_op();
      ilock(pv->f->ip);

      r = writei(pv->f->ip, 1, PGROUNDDOWN(addr) + i, off, n1);
      off += r;

      iunlock(pv->f->ip);
      end_op();

      if(r != n1){
        break;
      }
     }
     // if(i != n)
     // panic("munmap: write back failed");
  }
  // unmap specific pages
  uvmunmap(p->pagetable, PGROUNDDOWN(addr), n / PGSIZE, 1);
  // If munmap removes all pages of a previous mmap, 
  // it should decrement the reference count of
  // the corresponding struct file.
  if(addr == pv->addr && length == pv->length){
    fileclose(pv->f);
    pv->status = VMA_FREE;
  }

  return 0;
}