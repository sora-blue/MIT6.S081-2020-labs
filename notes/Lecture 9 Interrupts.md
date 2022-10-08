* same mechanism as syscall & traps
* interrupts
	* asynchronous 
	* concurrency
	* program devices
* registers
	* UART0
	* PLIC: Program Level Interrupt Controller
		* kernel has controls over the priority of interrupts
* drive mnages device
	* uart.c
		* TOP
			* R/W interrupts 
			* queue that is dealt parallel with CPU
		* bottom
			* interrupt handler
* programming device
	* memory mapped I/O
		* ld/st -> r/w control reg of the device
	* eg. manual of uart on one device![[Pasted image 20220914154302.png]]
	* example: $ ls
		* $: device puts $ into vart
		* vart gen interrupts when the char has been sent
		* keyboard -> uart -> bits2byte -> generate an interrupt
* RISC-V support for interrupts
	* SIE: supervisor interrupt enable
		* one bit for **E**xternal devices, **S**oftware interrupt(), **T**imer interupt
	* SSTATUS: bit enable disable
		* atomic mask 
	* SIP: interrupt pending
	* SCAUSE
	* STVEC: program switch
*  code: how interrupts is **enabled** in xv6 on risc-v env
	* start
		* M mode
		* disable paging
		* delegate all interrupts and exceptions to s mode
		* timer interrupt handlers in M mode
	* external device: main,c
		* consoleinit()
			* lock
			* uartinit
				* sets up uart chip
				* baud rate: the speed lines run
		* ...
		* plicinit()
			* PLIC is 32-bit reg
			* PLIC basically routes interrupts 
			* UART0_IRQ: enables interrupt requests from the uart
			* VIRTIO0_IRQ: enables interrupts from IO disks
		* plicinithart()
			* for every core specifically
			* say every core is interested in interrupts from plic
			* not accepting interrupt as **sstatus** not set
	* proc.c
		* scheduler
			* intr_on(): enable interrupt by setting SSTATUS
* code: how interrupt is **handled**
	* init.c<br >![[Pasted image 20220914161541.png]]
		* make a device that represents console using `mknod`, making it `stdin`
		* sets up file descriptors `stdout` and `stderr` by dups
	* sh.c
		* print "$ " to fd 2
	* printf.c (user)
		* ![[Pasted image 20220914161832.png]]
	* sysfile.c
		* sys_write
	* file.c
		* filewrite
		* if FD_DEVICE, call particular handler for that deivce(`console` here)
	* console.c
		* consolewrite
			* either_copyin
			* **uartputc**
	* uart.c
		* keeps a circular buffer![[Pasted image 20220914162127.png]]
		* uartputc![[Pasted image 20220914162607.png]]
			* check if buffer is full
			* if full, shell wold be put to sleep while uart is busy
			* else, put to buffer
		* uartstart
			* check device if it is busy
			* wake up device to do some work
	* devintr()![[Pasted image 20220914163419.png]]
		* looks `scause` register to see if a `supervisor external interrupt`
		* if so, call `plic_claim`
		* plic.c: plic_claim
			* tell PLIC that this particular cpu will handle this interrupt 
			* return interrupt number, say 10
		* 10  == UART0_IRQ, call `uartintr`![[Pasted image 20220914163736.png]]![[Pasted image 20220914163715.png]]
		* 
* What does risc-v hw do?
	* set interrupt enable bit in SSTATUS register
	* keyboard raises the interrupt line
	* goes through PLIC
	* PLIC routes the interrupt to particular core that **has SIE bit set**
		* SIE: supervisor interrupt enable
	* if SIE bit set
		* clear SIE bit: stop further interrupts
		* sepc <- PC: save return addr
		* save current mode
		* mode <- supervisor
		* pc <- `stvec`(usertrap | uservec | kernelvec)
			* hart will resume kernel at the instruction located `stvec`
		* say it ends up in `usertrap`
		* ...
* interrupts and concurrency
	* 1. device + cpu run in parallel
		* producer-consumer parallelism 
		* `consoleread` uses buffer similar to uart
			* kerboard types 'l' -> uart -> plic -> a specific core -> devintr() -> uartintr() -> consoleintr()
	* 2. interrupt stops & restores the current program'<br >interrupt enable/disable
		* kernel interrupt?
			* even kernel can be no sequential
			* kernel may need disable intr to keep atomic
	* 3. top of driver & bottom of driver may run in parallel 
		* example
			* shell call write syscall again
			* go to top level of driver
			* in other cpu, maybe a bottom part is running
		* run in parallel with **locks**
* interrupt evolution
	* interrupt used to be fast
	* interrupt is slow now compared to cpu
		* -> device is more complicated
		* eg. 1.5 million packets per second -> too frequent interrupt![[Pasted image 20220914170745.png]]
		* solution with fast device: polling
			* CPU keeps checking LHR
			* CPU spins util device has data
			* effect
				* to slow device: waste cpu cycles
				* to fast device: saves entry/exit cost
			* effect -> leads to:
				* dynamically switch between polling/interrupts
* question
	* which core would receive interrupt?
		* every core runs `scheduler()` so every sets interrupt enable bit
		* if that time core 1 sets its bit, core 1 would get it?
	* keyboard -> uart -> cpu -> console, but uart when printing a char in console?
		* QEMU acts like uart -> console
	* uart buffer shared across all cores?
		* the data structure is in **memory**
		* so every core shares it
	* how sleep is implemented?
		* `sleep` passes args: address of a channel id `&uart_tx_r`
		* `wakeup` passes `&uart_tx_r`
	* how shared buffer of uart between multi-cores is implemented?
		* ![[Pasted image 20220914171408.png]]
	* why lock inside interrupts?
		* bottom half could run in parallel with the top half
		* all cores could wait for one core? deadlock?
			* no risk of deadock here![[Pasted image 20220914171755.png]]
			* release the lock while sleeping
			* `sleep` returned -> re-acquire
	* all cpus get interrupted?
		* depends on how you program PLIC
		* xv6 uses the approach that only one core gets it
	* print not atomic?
		* locks are only around `putc`
		* `putc` from multi cores could interleave
	* how timer interrupt is handled?
		* use machine mode
		* kernelvec.S![[Pasted image 20220914172618.png]]
		* mret: machine mode -> supervisor more
		* kernel trap
	* too much memory allocated?![[Pasted image 20220914173015.png]]![[Pasted image 20220914173055.png]]
		* no answer yet