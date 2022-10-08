* introduction
	* thread
		* one serial execution
		* not the most sufficient 
		* most user friendly
	* thread state
		* PC
		* regs
		* stack
	* thread system
		* automate the interleaving
	* two topics
		* how to interleave
			* multi-core processor
			* switch among threads
		* shared memory?
			* single addr space
			* xv6 kernel threads do share memory
			* xv6 user processes - single thread - no shared mem
			* linux does allow 
	* none-thread technique
		* state machinery
		* event driving
* thread challenges
	* switching
		* scheduling
	* what to save/restore
	* compute bound threads: when to set a thread aside from running
* timer interrupts
	* kernel handler **yields** : thread switch
	* it is pre-emptive schedule(opposite to voluntary)
* state 
	* RUNNING
		* have a CPU
	* RUNNABLE
		* need to save CPU state
		* pc, regs, etc.
	* SLEEP
* in xv6, scheduler switches actually kernel threads![[Pasted image 20220917101534.png]]![[Pasted image 20220917102101.png]]
	* tf = trapframe that saves **user** registers
	* ctx contains **kernel thread** registers
	* kernel thread returns to user process
	* in xv6, there's direct user2user switch
	* kernel thread -> this cpu's scheduler thread -> kernel thread
	* each core's scheduler thread is set up at boot time
	* context switch
		* common: process 1 -> process 2
		* xv6: kernel thread -> scheduler thread
		* always produced by a call to `swtch()`
	* thread only runs on one cpu
	* in xv6, a process is either executing on user space or kernel space
		* each process has two threads: user thread & kernel thread
* code: how xv6 switches process on 1 cpu by timer interrupt
	* gdb
		* where
		* finish
		* print p->trapframe
		* print p->trapframe->epc
		* step
		* stepi 28: execute 28 instructions once
	* process
		* timer interrupt
		* usertrap()
		* myproc() 
			* use `tp` register to get cpu id
		* devintr()
		* which_dev == 2
		* yield()
			* acquire the lock 
				* because change ofstate: RUNNING -> RUNNABLE
				* we don't want other see it's RUNNABLE and run it while switch is not finished
			* change process state
			* call `sched()`
				* a bunch of checks because of bugs occurred in previous version of xv6
				* call `swtch()`
					* ra saves return address![[Pasted image 20220917104830.png]]
					* swtch doesn't save pc?
						* 'cause we're executing in swtch!
					* swtch doesn't save caller regs
					* swtch saves current kernel stack
					* $sp after switch: boot stack![[Pasted image 20220917105554.png]]![[Pasted image 20220917105628.png]]
			* `scheduler()`
				* yield() has acquired the lock
				* we released the lock here![[Pasted image 20220917105747.png]]
				* switch to another process![[Pasted image 20220917110241.png]]
				* ra returns to `sched` (the line before the thread was switched to another process kernel thread, the stack is fine, the cpu regs are the only volatile ones)![[Pasted image 20220917110319.png]]
				* 
			* p->lock(-> sounds R)
				* yield
				* switch context(also turn off interrupts)
* questions
	* where is context in xv6?
		* process context: p->context
		* scheduler context: in `struct cpu`
	* process calls some system calls to sleep on its own?
		* yes, for example, `read`
	* switch on x86? eg., floating unit reg?
		* the code is deeply microprocessor dependant
	* malfunction? 
		* user process cannot turn off time interrupts, thus pre-emptive scheduling
		* xv6 is carefully designed not to turn off time interrupts forever in a loop
		* follow-up: hw malfunction?
			* buy a new computer :)
			* sw sometimes compensate hw errors(eg. network packets) but we assume it is working
	* why .S code, not .C?
		* you cannot directly see these registers in ordinary C
	* if thread ends before a timer interrupt
		* handled by `exec()` and other system calls & interrupts
		* timer interrupt is an example here
	* how OS manage user threads? diff user threads run on diff cores?
		* linux supports it with a complicated way
		* each thread in linux is almost a complete process but share addr space to some extent
	* ?
		* `allocproc` sets up the faked first swtch![[Pasted image 20220917112151.png]]
		* for the first switch, `usertrapret()` is also faked, as you return from kernel mode to user space, and there's actually no user trap ever happened before it
		* fork() copies counter, and the first doesn't call fork(), so![[Pasted image 20220917112513.png]]
	* weird code in forkret?![[Pasted image 20220917112755.png]]
		* the initialization of file system has to be deferred when the first process is running 'cause the file system init code must be executed within a process's context
##### preparations
risc-v stores cpu id in `mhartid`, and xv6 stores it in `tp` reg of each cpu.
![[Pasted image 20220917094007.png]]
![[Pasted image 20220917094018.png]]
[Why -mcmodel medany?](https://blog.csdn.net/zoomdy/article/details/100699108)

##### my questions

1. when `release(&p->lock);` gets executed in `yield(void)`?
![[Pasted image 20220917093454.png]]
2. when a process is under yield, every other core scheduling thread is spinning 'cause `acquire(&p->lock);`?