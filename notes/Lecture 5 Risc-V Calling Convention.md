### Today's Lecture: a road map
* lab syscall
* c -> asm / processors
* risc-v & x86
* registers
* stack + calling convention
* struct layout in memory

```asm
.global // make sure functions in .S can be used elsewhere
.text //text segment
sum_to:
	mv t0, a0
	li a0, 0
loop:
	add a0, a0, t0
	addi t0, t0, -1
	bnez t0, loop
	ret
```
* gdb
	* tui enable
	* layout asm
	* layout reg
	* layout src
	* layout split = src + asm
	* focus reg
	* info b / info reg
	* apropos (-v) tui: show comands including tui
	* i frame: stack frame
	* i args: arguments
	* i locals
	* bt
	* p *argv@3: dummymain print 3 args in stack 
	* x/s argv
	* x/d argv
	* *watch points*
		* watch i: watch var value
	* *conditional breakpoint*
		* b sum_to if i == 5
* tmux on Athena: split terminal window
	* Ctrl+B C (Emacs com that get an extra window)
	* Ctrl+B % / Ctrl+B "
	* Ctrl+b O jump across
### Risc-V Registers
![[Pasted image 20220905151321.png]]
* use ABI names
* **compressed instructions of RISC-V** that is 16-bit
* we use x8-15 **in compressed instruction mode**
* a0-a7 argument registers
* by `saver` we mean
	* caller: not preserved across fn call
	* callee: preserved
	* eg. return address is caller saved
* various data types fit into all these 64-bit reg by convention
* a0-a1: return value reg pair to put 128-bit return value
* question
	* why callee saved reg useful?
### Stack
![[Pasted image 20220905162357.png]]
* stack is to keep functions organized
* grow from hight addr to low addr
* reg
	* sp -> bottom of stack
	* fp -> top of cur. frame
* functions in asm
	* basic structure
		* function prologue 
		* body
		* epilogue
	* types
		* leaf function
			* the function that does not call other funcs
			* simple enough
			* eg. `sumTo`![[Pasted image 20220905162939.png]]
		* normal function
			* prologue: first 2 instructions that mani stack
				* save return addr
			* epilogue: end 2 instructions that mani stack
			* eg. `sum_then_double`
				* ![[Pasted image 20220905162949.png]]
				* if we delete pro and epi, the ra reg will be overridden by sum_to and `sum_then_double` would not return 