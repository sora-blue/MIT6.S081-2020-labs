* intro
	* problem
		* crash can lead the on-disk fs to be incorrect state
		* eg. power failure stops during multi-step op
	* solution
		* restore from logging
* risks
	* fs operation are multistep disk op
	* crash may leave fs invariance violated
	* after reboot
		* crash again
		* no crash but r/w data incorrectly 
	* example in xv6:
		* echo "hi" > x![[Pasted image 20220926105726.png]]
		* say power failure on write 76
			* no crash but lost an available inode
		* say power failure on write 32
			* inode may be allocated again
	* another example 
		* ![[Pasted image 20220926110308.png]]
		* say power failure on write 45
			* lose a block
		* say power failure on write 33
			* block may be allocated
* Solution - logging
	* advantage 
		* atomic fs calls
		* fast recovery
		* high performance 
	* approach
		* log writes
			* write the op to log, not exec it on spot
		* when op is done, commit op
		* install
			* move logs to where it shold be
		* clean log
	* questions
		* commit op crashes?
			* it's atomic, so it never writes partially
			* all write is done in logging
* manifest in xv6
	* pretty simple
	* ![[Pasted image 20220926112111.png]]
	* code
		* sys_open
			* begin_op()
				* ![[Pasted image 20220926115634.png]]
				* 
			* on-memory ops
			* end_op(): commit
		* fs.c
			* ialloc
				* log_write
					* look if in arr
					* stick the blockno into entry
					* just appends it in in-mem data structure
					* bpin
		* end_op()
			* commit()![[Pasted image 20220926112951.png]]
				* write_log
				* write_head![[Pasted image 20220926112725.png]]
					* crash before bwrite
						* all changes are in mem, nothing would be done
					* crash after bwrite
						* log headers will be recovered
					* ![[Pasted image 20220926112930.png]]
				* install_trans![[Pasted image 20220926113024.png]]
				* recover
					* ![[Pasted image 20220926113236.png]]
					* ![[Pasted image 20220926113053.png]]	
				* why not just writes into dev instead of buf?
					* for simplicity 
				* what about process?
					* we only ensure the consistency of fs
		* put print into sw else to include recovery op
			* ![[Pasted image 20220926113713.png]]
			* ![[Pasted image 20220926113900.png]]
				* bwrite 3,4,5: write into log
				* bwrite 2: write log header
				* bwrite 33, 46, 32: install the changes
				* bwrite 2: clear the log
			* is it efficient?
				* no
				* writing stuff twice 
				* look into how to solve this when reading ext3 paper
* challenges
	* eviction
		* say evict block 45 -> 45 to home location
		* bad write-ahead rule, did not go into log
		* **do not evict blocks that are in log**
		* what does bpin does here?
			* ![[Pasted image 20220926114355.png]]
			* prevent the block from being evicted
	* fs op must fit in log
		* say max log size is 30 here in xv6
		* code: filewrite()
			* ![[Pasted image 20220926114911.png]]
		* solution(slt) in xv6: split the write op
	* concurrent fs calls
		* all concurrent ops must fit in the log
		* slt: limit concurrent fs calls
			* if over the limit, sleep until **group** commit
			* you must commit the op **in the order they happened**
			* it's safer to commit it all together
* summary
	* logging for multi-step ops BUT performance?
* questions
	* cache size >= log size ?
		* it is log funcs that call bget bwrite ...
		* bget panic when no more buffer 'cause we don't want undo changes
	* why group commits
		* **ordering sys calls is important** 
		