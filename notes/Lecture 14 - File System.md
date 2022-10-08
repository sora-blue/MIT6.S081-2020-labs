* introduction
	* user friendly names / pathnames
	* share files between users / processes
	* persistence / durability 
* Why interesting?
	* Abstraction is useful
	* Crash safety
	* Disk layout
	* Performance 
		* storage devices are slow
			* buffer cache
			* concurrency 
* API example
	* ![[Pasted image 20220926093252.png]]
	* file system is not the only implementation of storage system, e.g., database system
	* use a pathname to indicate file, not numbers, human-readable
	* use an offset
	* work fine after unlink, fd independent of name
* File system structures
	* inode
		* fileinfo
		* independent of name
		* index
		* link count
		* open fd count
			* file shoud be deleted when both cnt zero
		* fd must maintain an offset
* Fs layers
	* ![[Pasted image 20220926093818.png]]
	* disk: storage devices
		* sector: smallest unit to r/w 
		* block: a file system idea
		* sector and block do not always share the same size
		* ?synchronous and asynchronous options offered?
	* xv6 disk layout
		* block 0: boot sector
		* block 1: super blocks
		* block 2~32: logs
		* block 32~45: inodes
			* inode: 64 byte
		* block 45~46: bitmap 
		* above: metadata 
		* block 46: data 
		* read inode 10?
			*  32(inode blocks start from) + 10(inode#) * 64(inode size) / 1024(block size)	
	* on-disk inode
		* type
		* nlink
		* size
		* block numbers (e.g., 12 in xv6)
		* indirect block number
			* ![[Pasted image 20220926095150.png]]
		* too small size allowed!: double indirect
			* ![[Pasted image 20220926095507.png]]
		* read byte 8000?
			* 8000 / 1024(block size) = 7th block number
			* 8000 % 1024 = 832 (offset)
	* directories
		* directory  is basically the file in unix-like sys
		* an xv6 entry
			* inum#(2 bytes) + filename(14bytes)
		* pathname lookup? /y/x
			* start from root inode number, eg. block 32 in xv6
			* scans block for name "y"
			* read inode 251
			* scans its block for "x"
		* type?
			* inode type tells whether a directory or a file
		* not very efficient compared to real fs
			* you can plug in your fav data structures
	* bcache block cache
		* **one copy** of block in mem
		* sleep locks
		* LRU
		* two levels of locking
			* bcache.lock
			* b->lock
* summary
	* fs = on-disk data structure
		* the fs in xv6 is a very simple case
	* block cache
* code: bio.c
	* ![[Pasted image 20220926100547.png]]
		* create the file
			* write block 33: inode from free to file, mark it in use
			* write block 33: populate inode entry
			* write block 46: first data block, root directory (inode 1), add the file to the hierarchy 
			* write block 32: inode 1, size change, etc.
			* write block 33: update inode of file x
		* write "hi" to file
			* write 45: bitmap block, pick a free block, 595 here
			* write 595: write H
			* write 595: write i
			* write 33: update size, bn0 of inode of file x
			* why 595?
				* user programs, etc.
		* 
	* echo program<br >![[Pasted image 20220926100611.png]]
	* sys_open()
		* create()
			* look up dir
			* ialloc
				* walk all available and find a free inode
		* what if all calls create() at the same time?
			* bget() deals it![[Pasted image 20220926101843.png]]
		* release bcache.lock?
			* in xv6
			* `bcache.lock` guards bcache
			* `b->block` guards the block 
			* after `b->refcnt++`, nothing would be done to the block
			* block number must appear in buffer cache **only once**
		* mullti processes to one block?
			* ![[Pasted image 20220926102320.png]]
			* release b->lock
			* anybody else observes the changes
		* sleepblock?
			* ![[Pasted image 20220926102641.png]]
			* spin block associated with it so why
			* disk operations long time?
			* turn intr off?
			* not allowed sleeping while holding spinlock
			* so sleeplock acts like a long term lock, live through intr
		* why LRU in bget()?
		* 
		
