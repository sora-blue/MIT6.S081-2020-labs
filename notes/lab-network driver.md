* You will need to ensure that each mbuf is eventually freed, but only after the E1000 has finished transmitting the packet (the E1000 sets the E1000_TXD_STAT_DD bit in the descriptor to indicate this).

While writing your code, you'll find yourself referring to the E1000 [Software Developer's Manual](https://pdos.csail.mit.edu/6.S081/2020/readings/8254x_GBe_SDM.pdf). Of particular help may be the following sections:

-   Section 2 is essential and gives an overview of the entire device.
-   Section 3.2 gives an overview of packet receiving.
-   Section 3.3 gives an overview of packet transmission, alongside section 3.4.
-   Section 13 gives an overview of the registers used by the E1000.
-   Section 14 may help you understand the init code that we've provided.

* If there is a race
between hardware setting a cause and software clearing an interrupt, the bit remains set. No
race condition exists on writing the Set register. A ‘set’ provides for software posting of an
interrupt.

 nettests
nettests running on port 25099
testing ping: OK
testing single-process pings: OK
testing multi-process pings: scause 0x000000000000000d
sepc=0x0000000080000138 stval=0x0000000021fa705f
panic: kerneltrap