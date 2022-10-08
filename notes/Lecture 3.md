### isolation
* to enable OS's stability
* If no OS?
	* Strawman design
		* CPU distribution problem
			* multiplexing
			* eg. endless loop
		* Memory isolation problem
			* boundaries between text seg. of programs in MEM
* Unix interface
	* abstract hardware resources
	* elements
		* process: time-multiplexing
		* exec: virtual memory
		* files: abstract disk blocks
	* problems
		* cache affinity
* defensive OS
	* aims
		* app cannot crash OS
		* app cannot break out of its isolation
	* HareWare Support
		* kernel mode
		* page table / virtual memory
* User/Kernel mode
	* privileged instructions
		* set up page table
		* disable clock
		* interrupts
	* unprivileged instructions 
	* 3 levels of privilege
	* problems
		* switch from user mode to kernel mode?
			* page table for each process 
			* ecall instruction in RISC-V
				* a syscall number as argument 
				* particular entry point in kernel
				* eg. user fork() <br >-> ecall SYS_fork <br >-> kernel side `syscall()`() 	<br >-> check a table, privilege, etc.
		* how prevent endless loop in user program 
			* kernel sets a timer for each user program 
* kernel = trusted computing base (TCB)
	* kernel must have no bug
	* kernel must treat process as malicious ones
	* two ways of design
		* monolithic kernel design: OS implementation **all in kernel mode**
			* tight integration -> performance 
			* though more risky of getting bugs in it
		* macro kernel design 
			* opposite of monolithic design 
			* keep smaller code in kernel mode -> probably fewer bugs
			* eg.
				* exec use a IPC system
				* sh -exec-> kernel -> File System
				* FileSystem -result-> kerenl -> sh
			* kernel as intermediate -> performance?
				* switch cost between u/k mode
				* less integration, etc. sharing a page cache becomes less possible 
		* apply in
			* mono 
				* classic unix design
				* data center
			* micro
				* embeded systems like `minx` or `sel4`
	* problem
		* how kernel compiled
			* Makefile -> proc.c -> GCC -> proc.S -> assembler -> proc.o
			* ld(loader) links .o together
			* qemu(C program thats simulate a risc-v hw env.)
				* starts from 0x80000000, etc.
qemu simulates it: 
![[Pasted image 20220829190855.png]]
* how to debug xv6 in qemu
	* make CPUS=1 qemu-gdb
	* riscv64-linux-gnu-gdb / gdb-multiarch
	* gdb commands
		* b \_entry
		* si
		* layout split
		* n
	* `usercode()`  -> proc.c `initcode`
		* exec `init` program: init.c
gdb split mode