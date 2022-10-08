### lab-pgtbl 
(yeah yeah hardest lab that cost me two nights and an afternoon)
* first ![[Pasted image 20220910114414.png]]
	* **0-0-0 user data & text**
	* 0-0-1 guard page
	* 0-0-2 user stack (execute bit could have been forbidden)
	* 255-511-510 trapframe
	* 255-511-511 trampoline
	* va can be contiguous while pa can be not contiguous
	* question
		* why 255, not 511? 
			* because sign bit problem
			* if 39th bit is set, sign-bit will be extended to higher bits
			* so xv6 only use 38 out of 39 bits
		* why user data & text in the same page?
			* just to make exec in xv6 simpler
			* not the case in real OS
			* *Yep!*
* second
	* techniques 
		* take baby step: test every thing after a small change is made
		* keep existing code
	* kernelbase sits at entry 2 (calculate it from 0x80000000)
	* teacher's solution: use a function that copys kernel pagetable and add some mappings![[Pasted image 20220910123555.png]]
	* in scheduler, u actually have to switch it to and switch back, otherwise we are relying on freed pagetable![[Pasted image 20220910122450.png]]
	* question
		* `proc kpt -> proc kpt` instead of  `proc kpt -> kpt -> proc kpt`, possible?
			* yep, but a little tricky, test the approach by usertests
* third
	* teacher's solution![[Pasted image 20220910124901.png]]
		* it is not necessary to call mappages
		* you can just copy all these entries from user pagetable
		* risc-v hw does not permit kernel to use U bit set pages  by default(sstatus), so here we **only allow reading** user pages and set U bit 0
* left questions
	* strange order in trapframe?
		* compact encoding of instructions.
	* allow user to accross CLINT or PLIC?
		* you map these device addrs above PHYSTOP
	* There's no phy mem associated with Guard page around kstack
	* you are able to overshoot kstack to next kstack but likely a page fault
	* one pt strategy allows side-channel attack like meltdown

### my questions
* Why CLINT can be ignored but PLIC cannot?