1. when user mappings change, kernel page table should be changed **as well**.
2. permissions in kernel page table PTE should be different? but come on, it's the same va, the same pa, the same page! not the same page, it's just yet another table

in `proc_kvminit()`, i used mappages instead and ignored "remap" panic. But when it comes to that you have to keep different permissions...

**Something went wrong in my last task. I should not ignore remap panic as the same page table should only map it once. Back and debug it.**

> *vim delete to boundaries of brackets*
> dfc 删除从文本中出现字符“c”的位置，一直向前直到下一个该字符所 
      出现的位置（包括该字符）之间的内容  
  dtc 删除当前行直到下一个字符“c”所出现位置之间的内容
  https://blog.csdn.net/caisini_vc/article/details/38351133

```c
// add a mapping to the kernel page table.
// only used when booting.
// does not flush TLB or enable paging.
void
kvmmap(uint64 va, uint64 pa, uint64 sz, int perm)
{
  if(!perm)
    uvmunmap(kernel_pagetable, va, sz, 0);
  else if(mappages(kernel_pagetable, va, sz, pa, perm) != 0)
    panic("kvmmap");
}
```
i modified the code above to do unmap without freeing physical pages. And it solved the "remap" panic with all tests passed as well.  But the args were wrong, we dont need a pa, and it's all messed up.
Tomorrow i will:
1. build a `kvmunmap` for kstack's unmap, fix the mistake
2. consider how to do the mapping when user mappings changed. i have to get `pa` at once. i can modify all declarations of it to put `kernel_pagetable` in. It's ok to `kalloc()` again? A definite no. But mappages are ok, **diff pts with the same va or pa do not interfere with each other**.Take care since line 238 `uvmcreate` in vm.c.
3. start next lesson as soon as possible


trapframe problem?
1. modify `proc_kvminit` to map users's p->trapframe?

in `exec.c`
1. kernel code executes in **kstack**
2. the newly created table do not have a mapping for **kstack**
3. have to replace old table with new table while mapping **kstack**
**忽略了一个remap的panic**
```bash
208            panic("remap");
(gdb) bt
#0  mappages (pagetable=pagetable@entry=0x87e84000, va=va@entry=33554432, size=size@entry=4096, 
    pa=pa@entry=2246291456, perm=perm@entry=14) at kernel/vm.c:208
#1  0x0000000080001676 in uvmalloc (pagetable=0x87e85000, kpagetable=0x87e84000, oldsz=33554432, 
    newsz=33558528) at kernel/vm.c:300
#2  0x0000000080002074 in growproc (n=<optimized out>) at kernel/proc.c:289
#3  0x0000000080002f60 in sys_sbrk () at kernel/sysproc.c:50
#4  0x0000000080002e64 in syscall () at kernel/syscall.c:140
#5  0x0000000080002b4e in usertrap () at kernel/trap.c:67
#6  0x0000000000004d54 in ?? ()
Backtrace stopped: previous frame inner to this frame (corrupt stack?)
```


































1. got to modify `freewalk` to free pagetables without freeing leaf nodes
2. we got `copyin_new` in vmcopyin.c and old `copyin` in vm.c and the diff is what i should achieve 
3. `copyin` user: syscall.c
4. user virtual address -> physical address, how an extra kernel page table comes up to help? use user's kernel page table instead of translating twice time?
5. ok i got to know it. it's split into 2 parts. <br >First, maintain a complete copy of kernel page table in `struct proc` and make sure satp and whatever switch to it, so now process running on a copy of kernel page table. <br >Next, add user memory mappings to it, so it comes to full blow and now we can finally remove the process of walking the process page-table in software.
```c
// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcva in a given page table.
// Return 0 on success, -1 on error.

int
copyin_new(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
  struct proc *p = myproc();

  if (srcva >= p->sz || srcva+len >= p->sz || srcva+len < srcva)
    return -1;
  memmove((void *) dst, (void *)srcva, len);
  stats.ncopyin++;   // XXX lock

  return 0;
}
```
```c
// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcva in a given page table.
// Return 0 on success, -1 on error.
int
copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
  uint64 n, va0, pa0;
  
  while(len > 0){
    va0 = PGROUNDDOWN(srcva);
    pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0)
      return -1;

    n = PGSIZE - (srcva - va0);
    if(n > len)
      n = len;

    memmove(dst, (void *)(pa0 + (srcva - va0)), n);  
    len -= n;
    dst += n;
    srcva = va0 + PGSIZE;
  }
  return 0;

}
```