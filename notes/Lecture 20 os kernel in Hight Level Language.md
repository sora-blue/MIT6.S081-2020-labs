* in Biscuit
	* kernel threads are go routines
	* Each user thread has companion kernel thread
	* on x86 isa
* biscuit design puzzles
	* runtime on bare-mental
		* a special ver of go routine, agnostic, implemented carefully to not rely on too many os features
		* some necessary os features are emulated
	* goroutines run different apps
		* go scheduler cannot switch pagetable
		* entry & exit of kernel switches pgtbl
		* which means simliar approach to copyin&copyout in xv6
	* device interrupts in runtime critical 
		* cannot turn off intr when holding lock
		* barely did nothing, no lock, no mem allocation
		* but send a flag and wakeup a functional go routine to deal with the intr
	* heap exhaustion (hardest, unexpected surprise)
		* when heap is full
			* in other langs
				* malloc() can return 0
			* in go runtime
				* there is no error returned
		* strawmans
			* panic: undesired
			* wait for mem in allocator
				* may deadlock
				* eg. one waiting for mem while one tries to free mem
			* cannot check/handle allocation failure like C code
		* linux faces them either
			* **"too small to fail" rule**
		* biscuit solution: reserve mem
			* no checks, no error handling code, no deadlocks
			* kernel clears cache and yadayada to meet reservation
			* how to compute max mem for each syscall?
				* HLL is easy to analyze
				* use go's static analysis package
				* mem escaping?
					* eg. h() alloc mem, hands it to g()
				* annotations for difficult cases
					* eg. set limits for this and that
* implementation 
	* ![[Pasted image 20221008155120.png]]
	* go lang doesn't affect implementation of these features
	* Should we use HLL to build os kernel?
		* 1. does biscuit use HLL features?
			* ![[Pasted image 20221008151138.png]]
		* channel feature was not preferred also in go runtime
	* 2. does hll simplify biscuit code?
		* ![[Pasted image 20221008155623.png]]
		* GC
			* e.g. VMA would be freed by GC
		* simpler concurrency, simpler mem sharing
			* e.g. ![[Pasted image 20221008155737.png]]
				* buf mem freed by GC
				* locks in C reference count could be bottleneck
		* simpler read-lock-free concurrency
			* lock free
			* ![[Pasted image 20221008160046.png]]
			* get() reader - lock free
			* pop() writer - with lock
			* in C, if no lock and free head after $atomic_store, $Head could be used after free
				* ![[Pasted image 20221008160311.png]]
				* Linux used an approach call RCU(Read Copy Update)
					* defer mem free until it's safe
					* programs're restricted to obey RCU rules
			* with GC, there's none issue
	* would hll prevent exploits?
		* 2017 Linux CVEs, if in go
		* ![[Pasted image 20221008160525.png]]
			* 40 CVEs could be avoided
* HLL performance cost
	* ![[Pasted image 20221008160956.png]]
	* nearly the same order of magnitude, not 0.1x or 0.01x
	* what is the breakdown of it?
		* ![[Pasted image 20221008161241.png]]
			* safety checks: arr bound check, no pointer
		* ![[Pasted image 20221008161329.png]]
			* prologue taks the most, which is related to mem alloc
			* GC cycles linear to live objs ~~(and free memory left)~~
				* ![[Pasted image 20221008161543.png]]
				* ![[Pasted image 20221008161606.png]]
				* total mem = live mem + free mem
* GC pauses in biscuit 
	* ![[Pasted image 20221008161918.png]]
* code path comparision: Go Vs C
	* ![[Pasted image 20221008162339.png]]
	* ![[Pasted image 20221008162415.png]]
		* instructions execd per second
* Conclusion
	* ![[Pasted image 20221008162514.png]]
* Questions
	* go channels instead of mutex locks?
		* locks & condition vars used
		* fs implementation with go channels got bad performance
	* worst case in static mem ana?
		* conservative scheme
		* tool computes the worst case
		* eg. exec for loop, cannot figure out
			* annotate max loops
		* eg. recursion
			* tweak Biscuit to prevent recursion function pass
	* static analysis usage?
		* compiler optimization 
	* assembly code to bre replaced by go?
		* we tried to write evrything in go
		* ~1, 500 assembly to boot go runtime
	* massive concurrency in Go routines?
		* go routine allocs stack dynamically
		* go routine support is mostly done in user space
		* go runtimes has some `mthread`, on top of which go routine runs
		* pthread implementation is a little heavyweight
	* all machine code?
		* Go runtime is complied program that has always to be running
[what is shim](https://zhuanlan.zhihu.com/p/436456032)
shim layer as bootloader?