### Page tables (Virtual memory)
* address space
	* briefly: CPU -virtual addr.-> MMU -physical addr.-> MEM
	* CPU stores a phy. addr. where page tables locate
	* page tables are **located in MEM** and MMU just looks into it(cpu should get a cahce in it)
	* updateing page tables(SATP reg., for example, in RISC-V) is a **privileged** instruction
	* RISC-V allowed (theoretically) virtual address space: 2 ^ 64
		* actual: 2^39 bytes, roughly 512 Gib
	* RISC-V allowed (theoretically) physical address space: 2 ^ 56
		* why 56? hardware designer cooks it up
	* play by page(usually 4kb), not addr
		* `PPN` stands for `Physical Page Number`
	* *questions*
		* how distinguish between different processes 
			* `SATP` the register decides which "map" to use
	* Giant tables? real RISC-V page table:
		* L3 -> L2 -> L1 
		* flags
			* V: valid PTE for use
			* RWX: access allowed
			* U: Usser accessible 
		* `SATP` stores the **physical** address
		* top 12 empty bits + L3's PPN(44 bits) = next L2's phy addr.
**virtual address page table(64bits -> 39bits)**
|(high)25 bits|27 bits|12 bits(low)|
|---|---|---|
|EXT|PPN|offset|
**physical address page table(L1, 56bits)**
|(high)44 bits|12 bits(low)|
|---|---|
|PPN|offset|
**real RISC-V virtual address(64bits -> 39bits)**
|(high)25 bits|9 bits|9 bits|9 bits|12 bits(low)|
|---|---|---|---|---|
|EXT|L2|L1|L0|offset|
**L3/L2 page table**
|(high)10 bits|44 bits|10 bits(low)|
|---|---|---|
|Reserved|PPN(phy addr of next pt)|flags|
![[Pasted image 20220829202833.png]]
* paging hw (RISC-V)
	* cache pf page entries in cpu
		* translation look-aside buffer (**TLB**)
		* hidden from OS
		* OS -_need-switch-page-table_-> CPU -> flush TLB
		* in RISCV, `sfence_vma` be the instruction
	* *questions*
		* why xv6 walks the process while hw does it already?
			* to create new page table
			* to r/w user mem in kernel 
		* why hw not just privides sth like a walk function?
![[Pasted image 20220829205322.png]]
![[Pasted image 20220829205401.png]]
* 
	* pt provides level of indirection
		* VA->PA mapping completely under OS's control
			* allow tricks like catch page fault and create pages
	* `PLIC` and `CLINT` on both sides
	* `UART`
	* `VIRTIO disk`
	* questions
		* addr below 0x80000000 -> devices but DRAM
		* every process has a corresponding kernel stack
		* virtual free mem: page tables, etc.
		* does xv6 do optimization like consolidate different mappings to the same phy addr into one?
			* would be your challenging homework in lab
	* xv6
		* VA->PA most identical: 0x2000 to 0x2000
		* guard page
			* guard kernel stack
			* empty
			* hight on virtual mem
			* not vaild
			* map the same addr twice, top and somewhere below PHYSTOP
		* permissions
			* Kernel text R-X
			* Kernel data RW-
![[Pasted image 20220829210426.png]]
* xv6 code + layout
	* `kvminit`
		* pagetable on top![[Pasted image 20220829213138.png]]
		* mapping IO device like UART0
		* vmprint(lab work)
			* identity mapping example: UART0's mapping at 0x1000 0000
			* 0x10000000 -> 0x10000\[27-bit index\] (-> 0x0 L2->L3 9-bit index) -> 0x80(L1 -> L2 9-bit index) -> 0x0 (L1 9-bit index)
			* setting up address space
				* trampoline?
	* kvminithart
		* init satp
			* **dramatic moment!** virtual address will start to be used, every address now has a different meaning![[Pasted image 20220829215128.png]]
			* 'cause kernel did identity mapping, we know that after virtual memory is enabled, the address will be translated as it is 
			* so next we will execute right instruction
		* call sfence_vma
	* *questions*
		* how kernel `walk` PTE?
			* kernel have the privileges to rw PTE
			* if alloc not set, return empty the top PTE
				* else: returns the bottom PTE
		* does `walk` directly visits physical addr?
			* the kernel sets up an identity mapping
		* does the following line in vm.c out of range?![[Pasted image 20220829220405.png]]
			* etext is last address of kernel and there's enough for storing kernel text segment
* kvminit()![[Pasted image 20220829220501.png]]
* user virtual memory![[Pasted image 20220829215859.png]]
* 全场最佳：`walk() is one of my favourite functions`
### My questions
* how 9-bit L1 field + satp makes 44-bit index?