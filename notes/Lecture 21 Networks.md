* networks 
* Ethernet packet format
	* dst mac + src mac + payload
	* MAC addr is 48 bit
	* first 24 bits MAC addr for manufacturers
* ARP packet format: Address Resolution Protocol
	*  a bunch of junks + src hw&ip addr + target hw&ip addr
	* ![[Pasted image 20221013203405.png]]
		* ffff ffff ffff + 5254 0a00 0202 + 0806(ARP type): Ethernet header
		* 0a00 020f target ip addr
* ip header format
	* ![[Pasted image 20221013204031.png]]
	* ![[Pasted image 20221013204115.png]]
	* ![[Pasted image 20221013204407.png]]
		* 0x3eae: checksum
		* 0x11: udp protocol 
* udp packet format
	* ![[Pasted image 20221013204514.png]]
	* ![[Pasted image 20221013204814.png]]
	* port 53: DNS Server port
	* maximum ethernet packet size: 1500(paper's Age) -> 9000 ~ 10000 at most today
		* error handling code limits the size also
		* router packet buffer size limits
* typical network stack 
	* ![[Pasted image 20221013205517.png]]
	* mbuf scheme in xv6 for packets buffering demo
	* NIC interrupt routine
		* ![[Pasted image 20221013210154.png]]
		* often queue packets in sys RAM
		* Q: 2 NICs?
			* typically for routers
			* or for security concern, to separate network apart`
* E1000 NIC
	* DMA packets to DMA rings in sys RAM
	* Q: Modern NIC?
		* also use DMA rings
		* more clever, doing something TCP processing in NIC, etc.
		* modern ones can set many queues than just one
	* Q: NIC communicates with VM or memory to visit is cached in CPU?
* Paper
	* screend
		* ![[Pasted image 20221013212304.png]]
			* why stops increasing?
			* paper: 
				* too many interrupts, making CPU *livelock*
				* top priority given to nic input interrupts
				* or DMA using up RAM resources so CPU cannot access RAM
				* net stack is always interrupted and never allowed to run, always discarding packets
		* one solution: polling
			* ![[Pasted image 20221013212740.png]]
			* interrupt does no packet copying but turn off interrupts and call net thread
			* net thread
				* ![[Pasted image 20221013212703.png]]
			* Q: loop when multi NICs?
			* Q: add buffers?
				* may not help, as there's always dropped packets as output packet rate != input packet rate after a point
				* in paper's age NIC does not DMA but store packets in its mem
				* new packets will only be appended to the queue
			* Q: innernal software buffers are full?
				* there'll be livelock as well
				* livelock comes from wasted work, spending time on unprocessed packets
				* net thread can also stop for app to read packets