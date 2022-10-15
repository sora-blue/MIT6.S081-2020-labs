* lessons
	* 6.033 paper understanding
	* 6.828 more os paper
	* 8.823 6.111 hardware
	* 6.829 network
	* 6.830 database
	* pdos class
		* **6.824 distributing class**
		* 6.858 computer security
		* 6.826 principles of computer 
	* follow os trends
		* sosp conference
		* osdi conference
		* lwn.net
* net lab
	* structure
		* hw
			* ![[Pasted image 20221015212110.png]]
			* network tends to connect to a slower type of bus
			* e1000 connects to PCIe
			* inside e1000
				* several registers
			* packets are sitting in mem through DMA, in ring data structure
		* sw
			* mbufs **outside** tx & rx rings
			* e1000 driver
				* ![[Pasted image 20221015212544.png]]
				* there's locking scheme
					* some data structure is shared
				* transmit
					* runs in top half
				* recv <- interrupt handler 
					* runs in bottom half
					* no concurrency in recv itself
					* only one recv runs at a time
				* two halves can run somewhat concurrently
				* lock panic?
					* ARP request in xv6
						* interrupt
						* e1000_recv
							* acquire lock
						* net_rx
						* net_rx_arp
						* e1000_transmit
							* bottom half -> top half
							* acquire lock
							* deadlock
					* solution
						* recursive lock(not necessary)
						* no locks in recv
						* two locks
					* actually the two halves suppose to be separated
		* rings
			* ![[Pasted image 20221015213645.png]]
			* tx
				* consumer produce coordination 
			* rx
				* handle bursts
				* not trip each other
			* Q: common tricks?
				* yes, consumer coordination trick
			* Q: full queue?
				* some packets will be dropped
				* and TCP will retransmit, sort of stuff
			* Q: recv intr?
				* re-enabled here
				* ![[Pasted image 20221015220405.png]]
		* descriptor
			* defined by hardware
			* ![[Pasted image 20221015213700.png]]
			* cmd
				* RSV RPS
				* RS
				* EOP
			* STATUS
				* DD
	* code
		* transmit solution
			* ![[Pasted image 20221015214051.png]]
			* \_\_sync\_\_synchronize ensures no reorde, that compiler should not move instructions from before this line to after this line
				* make sure fields are set indeed
			* ![[Pasted image 20221015214126.png]]
		* recv solution
			* ![[Pasted image 20221015214736.png]]
			* why while(1) first?
				* recv as much as we possible within one interrupt
				* amortize the cost of the interrupt
				* if once per intr, packets may be left unpicked
		* mbufs
			* common structure in kernels like linux
			* ![[Pasted image 20221015215259.png]]
		* locking
		* loop
		* cmd
	* challenges
		* hw specification 
		* concurrency
			* hw/sw
			* sw/sw
		* debugging
			* cannot single step NIC
* mmap
	* why we need mmap?
		* we want to write a value
		* ![[Pasted image 20221015215722.png]]
		* offset changes
		* data written to incorrect position
		* need an extra `lseek` syscall 
		* ![[Pasted image 20221015215815.png]]
		* very inconvenient interface
		* here comes mmap
		* ![[Pasted image 20221015215939.png]]