* monolithic kernel
	* big abstraction, e.g. FS
		* portability 
		* hide complexity 
		* resource management by kernel
	* one big program
		* easier for implementing syscalls
			* eg. exec, hands in both VM sys and FS
	* all in full hw privilege
* Why not monolithic?
	* big -> complex -> bugs -> security 
	* general purpose -> slow
	* design baked in
		* design limited by the apis provided
		* eg. b-tree and fs, wait for other proc
	* extensibility 
* microkernels
	* idea: 
		* tiny kernel just provides
			* IPC
			* tasks, threads, ...
		* run fs or manage VM in user space, etc.
		* user space procs talk to each other throught kernel IPC
	* example
		* ![[Pasted image 20221006205356.png]]
		* vi: text editor
		* cc: C compiler
		* vm: virtual manage management 
	* nowadays
		* L4 linux kernels
		* embeded sys to special specialized tasks
* Why?
	* aesthetic feelings
	* small 
		* -> secure
		* -> verifiable -> `seL4`, for example
		* -> fast
		* -> more flexible, bake in fewer design decisions
	* user level
		* -> modular
		* -> customize 
		* -> robust
			* eg. device drivers
	* O/S personalities
		* >=1 OS on the same machine
* Challenges
	* minimum syscall API?
		* support everything like `exec` or `fork`
		* rest of O/S
		* IPC speed
		* split servers like fs and vm may leave less space for opt
* L4
	* only 7 syscalls
	* 13, 000 lines of code(loc)
	* basic abstraction
		* tasks
		* addr space
		* threre can be multi threads within tasks
		* ipc between tasks
		* sycalls
			* thread create syscalls
			* send/recv IPC
			* mappings
				* change itself or other thread's addr space
				* for example, send data & code to newly created thread through syscall
			* device access
				* user level device drivers have direct access to device
			* intr -> IPC
				* for example, send one task's page fault to another task
			* yield syscall
			* pager
				* page fault -> kerenl -> IPC msg -> pager -> lazily alloc -> IPC -> kernel -> resume exec
			* Q: diff between a task & a thread?
				* L4 allows multi core exec in single task
				* one task can have multi threads
* IPC slowness
	* asynchronous scheme
	* ![[Pasted image 20221006214302.png]]
		* p1 wants send to IPC
		* append to kerenl sys buffer
		* p1 syscall returns
		* p2 wants to recv IPC (may yield CPU)
		* p2 copy from buffer
		* p2 syscall returns
	* request & response
	* cost
		* context swtch, TLB flush
		* sleep & wakeup
		* scheduler loop
		* buffer
		* etc.
* L4 fast IPC: 20x faster claimed
	* synchronous 
		* ![[Pasted image 20221006214945.png]]
		* send waits for recv & vice versa
		* save a context swtch, P1 send jumps into P2 directly
	* unbuffered
		* ![[Pasted image 20221006215033.png]]
		* copy directly from user space to user space
	* registers - zero copy
		* P1 can put data into designated regs
		* kernel ensures to preserve them when into P2
	* huge msg
		* page mapping
	* RPC
		* `CALL()` syscall - client send+recv
		* `SENDRECV()` - server sends the reply and waits for req msg from next syscall
		* half the number of kernel crossing
	* Q: P1 goes to P2, so P2 to P1 needs P2 to send a response?
		* yep
* compatibility with existing apps
	* easier approach: run monolithic kernel as a user-level process
		* ![[Pasted image 20221006220609.png]]
		* linux kernel server runs a single L4 thread
		* linux kernel server kernel threads implement in itself
			* why not L4 threads?
				* when the paper released, there were no multi core, so linux has its own thread package; or the old ver of linux runs only one thread
		* linux & vi process send & write syscalls while shell can run **concurrently**, which is a big diff
		* linux kernel has no control over which proc to run but L4 does
	* the paper answers the performance of micro kernel
		* Mach: a micro kernel prototype that runs much slower than Unix
		* two syscalls in L4, twice the time
			* ![[Pasted image 20221006221749.png]]
		* now
			* L4 applied in some smart phones
			* not so well to abstract people to migrate servers & historical softwares to L4
		* the idea of running a kernel as user-level software inspires VM Monitor 
* Questions 
	* L4 page tables?
		* L4 always swtch tables under a syscall
		* maybe there's a shared kernel mem for each L4 proc in order to trick L4 to swtch to correct page table
	* micro kernel now?
		* Mach 2.5
		* MacOS
		* Fuchsia
	* only 5% lower?
		* a combination of IPC cost and real software
			* e.g. cost of disk operation >> IPC cost
		* some clever tricks not covered
			* e.g. Tagged TLB to avoid extra context swtch in page 6
			* ![[Pasted image 20221006223624.png]]
			* ![[Pasted image 20221006223632.png]]
