* locks
	* for correct sharing
	* limit performance
	* single-thread performance reaches limit
	* avoid race conditions
		* demo: kfree![[Pasted image 20220915195231.png]]
			* one page lost after commenting out locks![[Pasted image 20220915195931.png]]
	* lock serializes the execution
* lock abstraction
	* acquire-release
* when to lock
	* > 1 process access a shared data structure
		* too strict?
			* lock-free programming
		* too loose? eg. printf
* could locking be atomic?
* lock perspectives
	* locks help avoid lost updates
	* locks make multi-step op atomic
	* locks help maintain invariant
* lock can cause problems
	* deadlock
		* acquire -> acquire![[Pasted image 20220915201651.png]]
		* solution
			* manually order locks, that is, acquire locks in order
* locks vs modularity
	* other modules' locks must be visible to other modules?
* locks vs performance
	* need to split up data structure
	* solution
		* 1. start with coarse-grained locks
		* 2. measure
		* 3.
		* 4. redesign
* case study: uart
	* uart.c
		* ![[Pasted image 20220915202956.png]]
		*  lock roles
			* protect this data structure
			* tailend is in flight
			* hw register has only one writer
		* ![[Pasted image 20220915203127.png]]
		* ![[Pasted image 20220915203541.png]]
		* lock in interrupts![[Pasted image 20220915203811.png]]
* how to implement a lock
	* broken accquire
	* good design
		* rely on a special instruction
* hw test-and-set support
	* risc-v: amoswap addr, R1, R2
		* process:
			* lock the addr
			* tmp <- \*addr
			* \*addr <- r1
			* r2 <- tmp
			* unlock & return
		* test result is stored in r2
		* write r1 into \*addr
	*  implementation dependent on how mem system actually works
```c++
// -What's the problem with it?
// two processes may read l->locked == 0 at the same time
while(true){
	if(l->locked == 0){
		l->locked = 1;
		return;
	}
}
```
* code: spinlock
	* spinlock.h
		* ![[Pasted image 20220915205154.png]]
			* it turns off interrupts at first
				* think uart![[Pasted image 20220915210023.png]]
				* uartputc got the lock
				* cause a interrupt
				* if only one CPU
				* uartintr grab the same lock![[Pasted image 20220915210118.png]]
			* C standard provides a function that is lock-test-and-set(coarse-grained interface)
			* complied into **amoswap** with risc-v isa![[Pasted image 20220915205300.png]]
		* release
			* ![[Pasted image 20220915205557.png]]
			* ![[Pasted image 20220915205607.png]]
			* **interrupt turns on until lock is released**![[Pasted image 20220915210334.png]]
	* more atomic operations in RISC-V![[Pasted image 20220915205847.png]]
* memory ordering
	* if cpu or compiler reorders the order of them?![[Pasted image 20220915210551.png]]
	* so we need **fence**![[Pasted image 20220915210725.png]]
		* any instructions between fence would not be reordered?
* Wrap up
	* locks
		* good for correctness
		* bad for perf
		* complicate programming
	* do not share data structure if you don't have to 
	* start with coarse-grained
	* use a race detector
* question
	* accquire all related locks at the beginning?
		* Coarse-Grained Locking
		* risky of losing performance
	* why not a single store but a `__sync_lock_release`
		* it depends on hw implementation 
		* cache line, wtc.
	* amoswap.w.rl makes the sync instruction redundant?
		* you can dive deeper into RISC-V's memory consistency model
		* there're more find-grained interface
	* multi-threading on single core?
		* you can see older system uses locks to manage interrupts instead of multi-cores

