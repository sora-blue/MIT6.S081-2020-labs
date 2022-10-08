page level granularity

* os kernel use vm in creative ways
* paper argues user apps can use VM to garbage collector
	* garbage collector
	* data compression
	* shared virtual memory
* What primitive OS should provide for these algorithms?
	* trap
		* similar to alarm handler in lab:15
		* 
	* prot1: decrease accessibility of one page
	* protN: decrease accessibility of N pages
		* more efficient as TLB flush is expensive 
	* unprot: increase accessibility 
	* dirty
	* map2
* unix today provides these syscalls:
	* mmap
		* map a file
			* addr, length, r/w, private or shared, fd, offset
	* mprotect
		* addr, len, protection mode
	* unmap
	* sigaction
		* signal handler
		* simple to `sigalarm()` implemented in xv6 lab
	* mappings to primitives in paper
		* ![[Pasted image 20221001205704.png]]
* VM implementation 
	* page table + virtual memory area (VMAs)
		* VMA
			* contiguous range of addrs
			* same permission
			* backed by same obj
* how applications use these primitives
	* user-level traps
		* process
			* PTE marked invalid
			* CPU jumps into kernel
			* kernel saves state
			* asks VM sys what to do
				* similar to lazy-allocation lab
				* look at VMAs
			* upcall into user space
				* user may call mprotect(), etc.
			* handler returns to kernel
			* kernel resumes interrupted process
		* Q: possible security vulnerabilities?
			* handler runs the same context as its application
			* handler can't affect other applications
		* example: huge memorization table
			* save results of expensive ops with args in a range
			* challenge: table > phy mem
				* sol: use vm primitives
					* alloc huge range
					* tb[i] -> pg fault -> f(i) -> tb[i]
					* free some unused pages in need by `proc1 / procN`
			* code: [sqrts.c](https://pdos.csail.mit.edu/6.S081/2020/lec/sqrts.c)
				* use `handle_sigsegv()` to dynamically alloc mem and calc results
	* garbage collector
		* example: A copying garbage collector
			* ~~copy GC, move data away to make up a bigger continguous space~~
			* forwarding:
				* move an object from "from space" to "to space"
				* and leave a forwarding pointer in the "from space"
				* ![[Pasted image 20221001212630.png]]
			* discard & free "to space"
		* baker's algorithm 
			* features
				* real-time
				* incremental
			*  main idea
				* ![[Pasted image 20221001213309.png]]
				* not to stop everything
				* just move the root object
				* whenever calls `new()`, forward a few more objects
				* whenever dereference a pointer
					* check if it's in "form space"
					* forwarding
					* ensure every pointer in "to space" points to objs in "to space"
			* drawbacks
				* it's too bad to dereference every time
				* parallelise gc becomes difficult, possible race
					* eg. forward objs twice
			* with user primitives
				* benefits
					* reduce the cost of check
					* get concurrency for free
				* approach
					* ![[Pasted image 20221001215000.png]]
					* there r scanned & unscanned areas in "to space"
					* copy root objs
					* map NONE
					* get a page fault
					* fault handler
						* scan one page of objects
						* forward them
							* eg. get 2 objs to unscanned space in "to space"
						* unprotect the scanned space in "to space" 
				* features
					* incremental
					* no pointer check
						* vm hw does it for us
				* Q: how to maintain the invariance of staying in the same page when forwarding objs that root has reference to?
					* flip the space, nothing done
					* copy a page of objs to "to space" 
					* n objs unscanned
					* GC go through pointers of the page
					* GC copies objs to unscanned space
				* concurrency
					* GC go through unscanned pages
					* scan one page at a time
					* app cannot access the page with `unmap`
					* automatic parallelism that app and GC can run concurrently
				* tricky issue with(w.) concurrency
					* how GC access unscanned protected area?
						* ![[Pasted image 20221001215426.png]]
					* solution
						* use `map2` 
						* ![[Pasted image 20221001215525.png]]
						* Q: diff pt?
							* the same pt but 2 addrs
				* code: not very serious and untested demo [baker.c](https://pdos.csail.mit.edu/6.S081/2020/lec/baker.c)
					* ![[Pasted image 20221001215835.png]]
						* readptr: check if in "from space" or "to space"
						* vm makes it cheaper
					* ![[Pasted image 20221001215913.png]]
						* gene garbage
						* use `readptr` to simulate a compiler with GC will do
					* ![[Pasted image 20221001220130.png]]
						* without VM solution like usage of `mutex`
							* check free space ok
							* if not, do the flip
					* ![[Pasted image 20221001220249.png]]
						* switch `from` and `to` pointer
						* forward root
					* ![[Pasted image 20221001220326.png]]
					* ![[Pasted image 20221001220422.png]]
						* without VM, readptr `forward`s every time
					* ![[Pasted image 20221001220528.png]]
						* shm_open makes a in-mem file
						* truncate it
						* **`map2`** by mmap twice
					* readptr with VM<br >![[Pasted image 20221001220710.png]]
					* page fault
					* ![[Pasted image 20221001220733.png]]
						* visit unscanned -> scan function runs
						* `scan` runs within addr range of GC
						* `mprotect` make addr available to user 
					* back to filp()
						* ![[Pasted image 20221001221025.png]]
				* should we use VM here?
					* most cases VM feature could have been implemented by extra instructions, if without compiler involved
					* apps like checkpoint or SVM needs VM
					* unix supports these primitive
* what has changed since '91?
	* most unix do support these primitives
	* continuous dev, some big changes
		* 5 levels PTE
		* ASID
			* Address Space IDentifiers 
			* to deal with TLB flush cost
		* KPTI
			* kernel pagetable isolation
			* to deal with meltdown attack
* Q&A time
	* contiguous range of addrs in VMA?
		* for virtual addr 
		* one VMA for a mmap call
		* see mmap lab
	* when stop & resatrt with baker GC?
		* GC can run all the time, can stop when no work
		* trace obj -> unscanned area not growing -> no work for GC