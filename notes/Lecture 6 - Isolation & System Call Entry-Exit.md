[RISC-V常见指令](https://www.cnblogs.com/truelycloud/p/10807398.html)
### how user mode -> kernel mode
* registers
	* stvec
	* sepc
	* sscratch
	* satp (to switch)
	* mode (to switch)
		* user / supervisor 
		* extra privileges
			* r/w control regs
				* satp 
				* stvec
				* sepc
				* sscratch
				* (s for supervisor)
			* use ptes PTE_U not set
			* **cannot** r/w every addr
	* pc (to switch)
	* 32 general purpose reg (to switch)
* isolation & security & transparent to user code
### when shell calls `write()`
* Progress![[Pasted image 20220906201153.png]]
	* write()
	* **ecall**
	* uservec - trampoline.S
	* usertrap() - trac.c
	* syscall()
	* sys_write()
	* syscall() left part
	* usertrapret() - trac.c : return to user mode
	* userret() - trampoline.S
		* **sret**: return to user space
* GDB demo: user -> kernel -> user
	* ecall: user -> kernel<br >![[Pasted image 20220906201907.png]]
	* /b \*0xde6/
	* /info reg/
		* a0 a1 a2: 3 args shell passed
	* /x/2c $a1/
	* tell user from small addr
	* /print/x $satp/
	* /Ctrl+A C -> info mem/ in qemu to print page table![[Pasted image 20220906202420.png]]
		* a: ever accessed (hw maintained)
		* d: ever written
		* 1st: shell's instructions
		* 2nd: shell's data
		* 3rd: stack guard page
		* 4th: 
		* 5th: trapframe: save user reg
		* 6th: trampoline page?
	* /x/3i 0xde4/
	* /stepi/
		* Now where are we after `ecall`? <br >  **trampoline page** mapped to each proc that is protected<br>  **kernel mode** now!
		* the very first code when handling trap
		* PC changed
		* **what ecall changed**
			* user mode -> kernel mode
			* save PC in spec reg
			* jump to: stvec -> PC, the beginning of trampoline page code
		* /print/x $sepc/
	* /x/3i 0x3ffffff004/
	* /info reg/
		* nothing changed
		* so we have to save user reg first
	* /print/x $stvec/
	* now we need to save 32 user regs
		* ecall could have done all those for us including PTE switch, switch to kernel stack pointer, but...
		* RISC-V designers leave the space for software, like some OS use the same PTE for kernel & user
		* 32 slots are in **trapframe** that is mapped to user space and prepared for saving
	* `ld sp, 8(a0)` loads kernel stack pointer, etc.
	* figure out which CPU core![[Pasted image 20220906205849.png]]
	* load usertrap() addr, load kernel page table
		* why don't we crash after switching page table
		* 'cause trampoline code is mapped same va & pa in kernel
	* /tui enable/
	* **in usertrap()**
		* traps from kernel are differentiated from those from user 
		* change $stvec
		* myproc() looks into an array by core id
		* save **$sepc** that stores user PC address
		* r_scause(): why trap : reason in $scause: 0x8 means syscall
			* print/x  $scause
		* return to next instruction
		* intr_on(): interrupt auto-turned-off by RISC-V hw
		* syscall()
	* **in syscall()**
		* read trapframe->a7 to find out which syscall
		* exec sys_write()
		* look into trapframe for args passed from user to kernel
		* assign syscall return value in p->trapframe->**a0**
	* **in usertrapret()**
		* turn off intr to change stvec
			* turn on in trampoline code
		* we can only switch page table in **trampoline** 'cause <br >only **trampoline** is mapped both in user & kernel space
	* **in userret() in trampoline**
		* syscall return value overrides user saved $a0
		* $sscratch $a0 switch, in case another user trap
		* **sret: switch to user mode**
			* spec -> PC
			* re-enable interrupts
* questions
	* write() and read() are expensive so if syscall could return an addr instead of a file descriptor?
		* Provided in many OS called MMFA(Memory Mapped File Access) that maps page to user program
		* gonna be lab assignment
	* csrrw?
		* swaps a0 with special scratch register
		* how kernel exec when no reg could be used
		* print/x $a0
		* print/x $sscratch
		* now we see that we saved $a0 in $sscratch <br >and load trampframe base addr into $a0
			* the machine boots in kernel and goes into user through sret and set up $sscratch reg
	* why trapframe, not the user stack?
		* not sure if user gets a stack, some programming lang use small mem from heap as stacks
	* spec in tramframe?
		* yes it is
		* and it's really important to save user regs through .S code before exec C code
	* **think if a malicious software gonna use these mechanism to break isolation**
	* **SPIE and UPIE?**

