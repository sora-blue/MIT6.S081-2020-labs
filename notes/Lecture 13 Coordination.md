* xv6 swtch between kerenl threads
	* ![[Pasted image 20220919161416.png]]
		* acquire lock here is important otherwise![[Pasted image 20220919161555.png]]
* xv6 forbids holding any other locks excluding `p->lock`
	* otherwise a deadlock
	* demo![[Pasted image 20220919161939.png]]
		* suppose we have a machine with single core
		* p1 acq and gives up cpu
		* p2 acq it but spins
		* deadlock, sys will freeze
		* a timer interrupt could occur
		* if timer interrupt in kernel code, it will call `yield()`
		* but `acquire` in xv6 turns interrupts off
		* xv6 checks if any other lock held in `swtch`
* coordination - sleep/wakeup
	* fundamental tool for writing threaded programs
	* situation
		* wait for some events
			* disk read
			* wait()
		* make diff threads interact
			* pipe
	* approaches
		* busy wait: a loop that checks if the requirement fulfilled
		* sleep & wakeup
* code: sleep & wakeup interface
	* uart driver rewritten![[Pasted image 20220919162927.png]]
		* classic driver style
		* not very efficient yet
		* a sleep channel and a lock passed
	* an interrupt per character ![[Pasted image 20220919163105.png]]
		* if finished, wakeup in uartwrite
* lost wakeups?
	* demo
		* broken_sleep(some channel value)![[Pasted image 20220919163643.png]]
		* wakeup(some channel value)![[Pasted image 20220919163728.png]]
		* usage without locking![[Pasted image 20220919163906.png]]
			* race access to hw!
			* ![[Pasted image 20220919164020.png]]
			* ![[Pasted image 20220919164041.png]]
			* we cannot hold the lock after sleep!
			* ![[Pasted image 20220919164138.png]]
			* result:<br >![[Pasted image 20220919164317.png]]
				* emit a few chars -> spin until new input from kb
			* why?
				* ![[Pasted image 20220919164419.png]]
				* other core acquires the lock as soon as the release
				* but nothing sleeps
				* so wakeup actually wakes up nothing
				* input fixes the output 'cause only one interrupt in uart driver here accidentally ![[Pasted image 20220919164655.png]]
				* `tx_done` is a simple communication way
			* we have to close the window
			* sleep protects the condition, atomically release & regain the lock
* code: implementation of sleep & wakeup
	* ![[Pasted image 20220919165235.png]]
	* ![[Pasted image 20220919165258.png]]
		* **acquire the lock of process** and block the wakeup() 
		* sleep re-acquires it after returns
		* acquire parent's lock?
			* prevent children & parent exits at the same time (concurrent exit)
			* children cannot free themselves 'cause, for example, kernel stack
		* why not set ZOMBIE before wakeup parent?
			* we lock up `p->lock` so it's fine
	* other examples 
		* piperead()
* semaphore: other implementation 
	* simpler, less general
	* interface?
		* just `p()` and `v()`
* shut down threads
	* challenges
		* kernel tasks not finished
		* not free up the resources using
* code
	* exit()
		* close all open files
		* release cwd reference
		* re-parent children process to init
		* set ZOMBIE status 'cause resources not fully released
		* call sched()
	* wait()
		* a critical part of exit()
		* parent process yields zomebie children![[Pasted image 20220919170935.png]]
			* parent call freeproc()
			* free up all resources of children process
	* kill()
		* ![[Pasted image 20220919172118.png]]
		* it cannot just kill the process 'cause it may be updating the file system
		* does actually nothing but set up flags
		* process checks itself if it is killed, for example, when timer interrupt goes off![[Pasted image 20220919171624.png]]
		* kill sleeping process on spot
			* for example![[Pasted image 20220919171944.png]]
		* sleep in file system does not check killed 'cause we want to finish fs operations
		* kill needs no permission?
			* xv6 is a toy OS
* questions
	* will init exit?
		* never, if there's a fatal error
	* shutdown OS?
		* fs should be left in good shape
		* and nothing but stop executing instructions