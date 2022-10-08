[lab lock](https://pdos.csail.mit.edu/6.S081/2020/labs/lock.html)
## use a hash table with one lock per bucket instead of a single linked list
**Remove the list of all buffers (bcache.head etc.) and instead time-stamp buffers using the time of their last use (i.e., using ticks in kernel/trap.c). With this change brelse doesn't need to acquire the bcache lock, and bget can select the least-recently used block based on the time-stamps.**

We suggest you look up block numbers in the cache with a hash table that has a lock per hash bucket.


do not delete acquire&release of bcache.lock at first
	* implement bucket locks but leave the global bcache.lock acquire/release at the beginning/end of bget to serialize the code. Once you are sure it is correct without race conditions, remove the global locks and deal with concurrency issues. You can also run make CPUS=1 qemu to test with one core.

#### 0. buf.h
remove line 8~9(LRU cache list)
replace them with timestamp field
	* Remove the list of all buffers (bcache.head etc.) and instead time-stamp buffers using the time of their last use (i.e., using ticks in kernel/trap.c). With this change brelse doesn't need to acquire the bcache lock, and bget can select the least-recently used block based on the time-stamps.
#### 1. binit
init bucket locks
for: i = 0 -> NBUF
	hash i
	find the slot
	initsleeplock
#### 2. bget
acquire & release bucket lock
check dev, but no blockno
	* Searching in the hash table for a buffer and allocating an entry for that buffer when the buffer is not found must be atomic.
recycle buffer through timestamp
	* When replacing a block, you might move a struct buf from one bucket to another bucket, because the new block hashes to a different bucket. You might have a tricky case: the new block might hash to the same bucket as the old block. Make sure you avoid deadlock in that case.
	* It is OK to serialize eviction in bget (i.e., the part of bget that selects a buffer to re-use when a lookup misses in the cache).
#### 3. bread & bwrite
no change
#### 4. brelse
remove acquire & release bcache.lock?
	* Your solution might need to hold two locks in some cases; for example, during eviction you may need to hold the bcache lock and a lock per bucket. Make sure you avoid deadlock.
acquire & release bucket lock?
update timestamp
#### 5. bpin & bunpin
replace bcache.lock with bucket lock

[[book-riscv-rev2（with notes）.pdf]]