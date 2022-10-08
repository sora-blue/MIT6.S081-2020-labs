* features implemented by **page fault** in modern OS
	* lazy allocation
	* MMA
	* cow(copy-and-write) fork
	* demand paging
* VM benefits
	* isolation
	* level of indirection
		* kernel controls va -> pa
			* trampoline
			* guard page
		* **page fault** makes these mappings **dynamic** on the fly
* info needed
	* the faulting va
		* sits in `stval` register
	* type of page fault
		* Table 4.2
		* in the `scause` register
		* R/W/I
		* eg. ecall is 8
	* va of instruction that cause page fault
		* `p->tramframe->epc` that corresponds to `spec` register
* allocation: sbrk
	* starts at where p->sz points to
	* starts at the bottom of heap
	* do eager allocation
		* kernel allocates mem for app when sbrk() is called
	* in lazy allocation 
		* sbrk() only does: `p->sz = p->sz + n;`
		* pg fault
		* allocate 1 page , zero it, and map **in pg fault handler**
		* demo
			* ![[Pasted image 20220910153235.png]]
			* ![[Pasted image 20220910153243.png]]
			* scause 0xf
			* stval: 0x4008
				* 8 bytes above fourth page
				* fifth page not allocated
			* sepc: 0x12a4
			* malloc() -> sbrk()
			* look `usertrap()` and we should add code here like(not working to full blow)![[Pasted image 20220910153815.png]]
				* ```
				if (r_scause() == 15){
					check if within p->sz
					allocate a physical page
					if failed, kill the process
				} ```
			* some of sbrk() allocated but never used, so not mapped![[Pasted image 20220910154010.png]]
			* in a basic approach, we can just ignore the panic![[Pasted image 20220910154204.png]]
			* what else is broken after this change?
				* negative number could be given and we **have to shrink **
		* question
			* if we use up all memories up?
				* return an error and kill the process
				* you gonna do it in lazy lab
			* va does not start at 0?
				* not above p->sz -> ok
				* above p->sz -> wrong
			* why kill proc?
				* no choice
				* real OS could be more sophisticated 
				* *i think it may be using swap memory*
* other features with page fault
	* zero fill on demand
		* BSS that has many pages that is zero **in va**
		* **in pa**, all these zero-page mapped to **one page** in pa and read-only
		* a change to these pages<br> -> pgfault<br > -> r/w page allocated<br > -> map it to new allocated page![[Pasted image 20220910155034.png]]
		* advantages
			* save space
			* less work in exec()
		* eg. C global variables
		* questions 
			* pagefault makes it slower?
				* yep. Every pgfault per 4096 zeros, so there can be an optimization
	* cow fork(copy-on-write fork)
		* like when shell execs ork -> exec, copying pages is a waste
		* mappings in child is **read-only** at first
		* child wants to modify the page<br > -> copy page<br > -> map it<br > -> ...![[Pasted image 20220910155809.png]]
		* questions
			* what about parentless process? can i just change permission and not coy?
				* up to you
			* can hw have a instruction copy a range of mem?
				* x86 got, riscv not
			* how kernel recognize if it's a cow page or it should be lazy allocated 
				* almost all pt table have support for this
				* see PTE bits, you can use RSW at your will
			* multi addrs to the same page
				* **we cannot free page on spot when the process exits**
				* so a ref count on every phy page will be in need
			* where to contain the ref count? RSW bits?
			* can we use info stored in kernel, not RSW bits?
				* yep
	* demand paging(1)
		* original eager allocate![[Pasted image 20220910161359.png]]
		* may not all the code or data is used
		* allocate text and data segment in pte but not all in pa
		* exec first instruction<br > -> got a pgfault<br > -> read it into mem<br > -> map mem into pgtbl<br > -> restart instruction
	* demand paging (2)
		* some programs' binary might be bigger than phy memory
		* solution: **evict a page when running out of memory**<br > -> use the-just-free page<br > -> restart instruction
		* algorithm that chooses the page to free: LRU(least recently used)
		* dirty or no-dirty page?
			* dirty page have been written
			* no-dirty page will be preferred to be freed 'cause nothing should be done
		* PTE 7-bit D: dirty bit
		* PTE 6-bit A: access bit
			* used in LRU strategy 
			* a clock algorithm does refresh the bit
			* like every 100ms you clear the A bit
		* questions
			* know dirty pages in cache but dirty pages in memory?
				* it's about files loaded in memory that is modified in mem
			* 
* memory-mapped files
	* `mmap()` and `unmap()`  provides support for this
	* when `unmap()` is called, write back dirty pages
	* also the process is done **lazy**
	* use vma( = virtual memory area) to figure out the boundary
	* questions
		* multi proc -> same file?
			* undefined behavior 
			* a file lock or sync function is needed
		* length or flags?
			* length be file length
			* flags can describe whether the page is private or shared
		* too long length?
			* put part of the file into file
* Summary
	* page tables + traps/page fault -> enormous elegant & powerful features