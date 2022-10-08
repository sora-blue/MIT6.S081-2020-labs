Instructor: Robert Tappan Morris

test & debug a little before testing & debugging, instead of giving a complete a solution at one time

* debug process
	* fork() failed
	* ![[Pasted image 20220919150549.png]]![[Pasted image 20220919150519.png]]
		* why scause 2?
			* PTE_W not set off
	* ![[Pasted image 20220919150608.png]]
		* set it off also in parent process![[Pasted image 20220919150801.png]]
	* scause 15
		* where gene the page fault![[Pasted image 20220919150950.png]]![[Pasted image 20220919150940.png]]
			* must be triggered by a store instruction
			* pid 1 is init process
			* pid 2 is the shell that prints $
			* pid 3 is the forked shell process that runs `parsecmd()`
	* deal with the page fault![[Pasted image 20220919151325.png]]
	* implement cowfault
		* outline a strategy to prevent visiting invalid addr?
			* check below MAXVA
			* check crazy addr![[Pasted image 20220919151718.png]]
				* if lazy allocation or other features to add, we need more secure checks
				* real OS get a data structure to deal with it instead of a PT entry
			* kalloc
			* 2 ways of doing a new mapping
				* unmap -> map
				* set up PTE bits directly
			* do sth to the other process?![[Pasted image 20220919152213.png]]
				* we cannot do anything until it itself triggers a page fault
	* a new bug![[Pasted image 20220919152535.png]]
		* exec will accidentally **free up shell's pages**, namely the parent process of it
		* we may also trigger some other page faults when stepping into data page, etc. instead of instruction page
	* build a ref count for memory
		* ![[Pasted image 20220919153303.png]]
			* actually phy addr starts 0x8000 0000 for kernel & user so i can make it (PHYSTOP - 0x80000000) / PGSIZE)
		* where the refcount arr stores?
			* linker looks it up and figure out global vars and allocate mem for it, *kernel data pages*?
			* linker & complier decide it
		* kalloc()<br >![[Pasted image 20220919153801.png]]
		* kfree()<br >![[Pasted image 20220919154235.png]]
		* incref() that anyone can call![[Pasted image 20220919154501.png]]
		* defs.h adds definition
		* the only place that needs a ref inc
			* uvmcopy
	* new bug<br >![[Pasted image 20220919154625.png]]
		* look into gdb<br >![[Pasted image 20220919154655.png]]
		* see `freerange()`
		* ![[Pasted image 20220919154749.png]]
	* ![[Pasted image 20220919154733.png]]
		* `kfree(pa1)` in cowfault
	* ![[Pasted image 20220919154917.png]]
		* 4 concurrent processes so error msg interleaved
		* sys_read
			* is argfd failing?
			* fd 9 failed but fd 9 should not exist
			* an invalid fd was used
			* all processes touched each other's memory range
			* one process wrote other's cow pages
			* `copyout` doesn't check W permission in PTE![[Pasted image 20220919155658.png]]
				* software walked the addr and did not use MMU
		* fix copyout
			* check valid![[Pasted image 20220919155936.png]]
			* check COW pages
				* a workaround: PTE_U and not PTE_W
				* ![[Pasted image 20220919160048.png]]
			* fix walk panic<br >![[Pasted image 20220919160243.png]]

cat .git/config
* actual cause of these scauses? like scause c for init process?![[Pasted image 20220919160738.png]]
	* maybe<br >![[Pasted image 20220919160908.png]]
