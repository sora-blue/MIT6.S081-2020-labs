![[Pasted image 20220912201813.png]]
Thinking that i could finish today' lab within 1 hour until
```
xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$ lazytests
n=65536, new_sz=81920 
lazytests starting
running test lazy alloc
n=1073741824, new_sz=1073754112 
test lazy alloc: OK
running test lazy unmap
n=1073741824, new_sz=1073754112 
n=-1073741824, new_sz=12288 
panic: freewalk: leaf
```
The test program sbrks an area and **writes value to it**,forks a program, sbrks a very large negative number, then stores a value in freed area to see whether a page fault would raise as normal
the problem occurs that the program returns but uvmunmap does not free some memory in leaf, which confuses me for next 2 hours
```
page table 0x0000000087234000
..4: pte 0x0000000021c93cdf pa 0x000000008724f000
pte: 0x0000000021c93cdf
pa: 0x000000008724f000
panic: freewalk: leaf
```
```
page table 0x0000000087207000
..0: pte 0x0000000021c8ac01 pa 0x000000008722b000
.. ..0: pte 0x0000000021c8d001 pa 0x0000000087234000
.. .. ..4: pte 0x0000000021c93cdf pa 0x000000008724f000
```