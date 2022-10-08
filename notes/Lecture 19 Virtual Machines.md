### VM scheme
* basic traditional scheme
	* ![[Pasted image 20221007203719.png]]
	* host: VM Monitor(VMM)
	* guest: guest supervisor for kernel & guest user
	* VMM emulates supervisor & user mode to leave kernels unmodified 
	* application
		* cloud service providers like AWS
		* kerenl development like using qemu, easier to debug
		* extra tricks provided by VMM, for example, making checkpoint or migrate guest sys image to other computers
	* trend
		* guest kernel gets to drift down one layer to VMM
		* not very faithful full emulation, guest kernel can talk to VMM to get direct hw access for speeding up
	* Guest should not know it's running in VM, prevalently
	* Guest machine should be confined to secure strict isolation, not to break out of their virtual machine
	* VMM separates VMs apart 
* traditional implementation 
	* pure software approach: way too slow
	* How about run instructions directly on hw?
		* privileged instructions?
		* escape from VM?
	* strategy: Guest kernel in user mode
		*  if VMM runs in kernel mode **without hw support like EPT**
			* Guest OS in user mode
			* VMM arranges it for Guest OS
			* VMM maintains a full table of VM state
			* TRAP-AND-EMULATE
				* Guest O/S process trap <br >-> VMM emualted privileged instruction <br >-> modify **virtual state** <br >-> return by `sret`
			* Q: why use virtual regs?
				* VMM needs real regs
				* e.g. trap -> real **scause** reg
			* VM needs to know if it's guest supervisor or user mode and hartid
			* Q: when Guest OS falls to VMM?
				* ordinary instructions -> directly on hw
				* privileged ones -> VMM
			* when guest os proc wants to call system call
				* `ecall`
				* VMM needs to simulate a user trap
				* write real `spec` to virtual `spec`, etc.
				* back to guest os kernel
			* VMM cooks up a virtual pagetable 
				* Guest Pagetable: Guest va -> Guest pa
				* VMM Map: Guest pa -> Host pa
				* VMM "shadow" pgtbl in real `satp`: Guest va -> Host pa
				* Q: if guest os wants a new pgtbl for new process?
					* guest os just does as usual
					* guest os kernel assign virtual `satp`
					* trap into VMM
					* update its map or cause a real fault
					* update real `satp` along with corresponding virtual states
					* back to guest os kernel
				* in riscv, only if you exec `sfence.vma` does hw see pgtbl entry modification immediately 
			* devices 
				* 1. emulation
					* eg. qemu emulates UART console device for xv6
					* low performance 
				* 2. virtual devices
					* cook up a device interface
					* eg. xv6 VIRTIO_DISK
						* qemu looks to array of ops to be performed
						* and performs them on fs.img in real fs
				* 3. pass-through NIC
					* eg. modern network interface card supports it
				* Q: diff btn 1 and 2?
					* a designed interface that omit many traps
					* to implement a driver btw the virtual device and the real hw is in need
### Hardware support for VM
* VT-x
	* hw provides another set of regs for guest os
		* hw make sure that they won't escape
		* (guest mode, aka, non-root)
		* VMM uses them through VMCS with special instructions
			* eg. VMLAUNCH VM RESUME VMCALL(exits from non-root mode to root mode) etc.
			* every timer interrupt, VM back to VMM, so time-sharing & other features available 
	* hw provides %ct3(intel equivalent of %satp) for guest kernel to load pgtbl without trapping VMM
		* to prevent escaping, there's another reg `EPT` (extended pgtbl)
		* VMM sets EPT for each guest kernel
		* guest va -> `%ct3` -> guest pa -> `EPT` -> host pa
		* every core has an own independent set of VT-x things
### [Dune's approach](https://pdos.csail.mit.edu/6.S081/2020/readings/belay-dune.pdf)
* basic scheme
	* ![[Pasted image 20221007215958.png]](sandbox application)
	* proc in dune mode has its own set of regs like %cr3
		* can have its own pgtbl
		* can have a supervisor mode and a user mode
		* applications
			* run untrusted user code in a sandbox
				* user syscall falls into proc non-root supervisior mode
			* speed up garbage collection
				* ![[Pasted image 20221007220344.png]]
				* dune helps GC see program modification of specific objects
				* dirty bit could be seen when GC trace through pgtbl
				* use load & store instructions rather a syscall to speed up
				* Q: GC + sandbox?
					* we don't get quick access to dirty bits in PTE
					* namely, we cannot combine two features in Dune
					* Dune is an experimental plugin for research
					* Dune uses CPU hw to support a process abstraction
				* Q: fork escape?
					* non-root user mode `fork()` would fall into non-root supervisor mode, so it's not actual `fork()`
					* Dune uses EPT to constraint non-root supervisor mode, setting up an addr space for Dune process
				* Q: user pgtbl?
					* proc addr space has to fit in smaller addr bits in Dune proc due to using another set of regs and due to phy addr bits <= va bits (guess by teacher)
				* Q: latency to pgtbl using VT-x apparatus device?
					* each level of PTE has to go through EPT
					* so the worst case could be really slow
				* Q: shadow copy in pgtbl?
					* VMM has to create a new pgtbl
					* there's quite opportunities for VMM to cache and keep shadow pgtbls for specific VM, etc.
					* maintaining of shado pgtbl is lots of work
					* EPT means you dont have to cook up your own shallow pgtbl
				* Q: when GC scan dirty bits?
					* GC may freeze the process when scan dirty bits
	* dune controls EPT for these processes

> Lecture 19 1:30:25 - consturcts -> constraints
> teacher happy about an extra question