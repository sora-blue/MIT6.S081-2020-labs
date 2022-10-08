ext3 = ext2 + journal(log)
* xv6 log review
	* app -> kernel bcache -> log -> fs
		* app
			* a simple implementation of transaction
			* begin_op()
			* end_op()
	* write-ahead rule
		* pre-declare all ops that wants to be **atomic** in log before updates
	* freeing rule
		* cannot reuse the log until all writes in log have been relected to actual location in fs
	* what's wrong with the scheme?
		* slow
		* synchronous 
		* write twice every time
* ext3
	* a logging layer added to the nearly unmodified ext2 fs
	* diff: keep track of multiple transactions  at the same time on diff stages of exec
	* data structure
		* on mem 
			* cache block
			* transaction
				* set of block numbers
				* handles
		* on disk
			* bitmap block
			* root path tree
			* log
* ext3 log format
	* ![[Pasted image 20220930152010.png]]
	* super block
		* block 
			* offset
			* seq num
		* description block
			* block nums
		* content blocks
		* commit block
		* ...
	* desc & commit blocks
		* start with a magic num to diff from data blocks
	* Q: order of multi transactionns?
		* only 1 transaction on **mem** & > 1 old transaction on **disk**(in log records)
		* log sys updates log
		* free & reuse the space
	* 3 ways to good perf: why ext3 > xv6
		* asynchronous sys calls
			* never wait for disk write
		* batching
			* batch calls into one transaction
		* concurrency 
* async sys calls
	* IO concurrency 
	* help batching
	* fsync(fd)
		* flush pre writes to disk
		* why? : downside of async
			* sys call returns -> it may not done 
			* eg. sys call returns -> crash -> data not here
* batching
	* always **one** "open" xaction
		* batch many writes into one xaction
		* by default 1 big xaction every 5 sec in ext3
	* write absorption 
		* may writes on the same block ->
		* update on cached blocks on mem ->
		* write to disk only once
	* disk scheduling 
		* write sequentially but randomly
		* benefit HDD more
* concurrency
	* two kinds
		* sys calls in parallel 
		* many older transactions
			* at any time
				* 1 open xaction
				* >=1 xactions committing to log
				* >=1 xactions writing to home locations 
				* >=1 xactions being freed
		* Q: write & op on the same block?
			* a xaction contains only sys calls given to it so ...
			* open xaction make copies of the blocks it modified, which are to commit to log
			* sound like a copy-on-write scheme, the unwritten blocks not seen by other proc
* ext3 code
	* sys calls
		* sys_unlink()
		* ...
	* begin & end (for every xaction sys)
		* h = start() // returns a handle
		* get(h, block num) // get block cache
		* modify blocks in cache
		* stop(h) // because many share the xaction, only commit until all called stop
	* steps in commit
		* block new sys calls // a little perf defect
		* wait for outstanding sys calls in this xaction to finish
		* open new xaction
		* write descriptor block w/ block #s 
		* write blocks to log
		* wait for writes to finish
		* write commit block
		* wait for commit write to finish // **commit point**
		* write xaction blocks to home locations
		* all writes complete -> re-use log is available 
	* Q: where commit steps run? 
		* background thread in kerenl
	* Q: what if logs are full? will we wait for it being freed?
		* yep
* recovery
	* log super block keeps the begining of oldest valid (not freed) xaction
	* recovery sw scans by data blocks number, check if a descriptor block behind a commit block by magic num
		* extra checks byond magic num
		* ![[Pasted image 20220930161335.png]]
		* Q: what if data blocks happen to begin with magic number?
			* ext3 will replace it with zero & record it in desc block
		* stop when see a invalid desc block(eg. not start with magic num or not having a valid commit block at the end of the supposed xaction)
		* clear the log to last commit block, ignore possibly partial commit
		* replay log to its home location![[Pasted image 20220930161714.png]]
		* start other parts of os **only after** recovery finishes
* tricky details
	* no new syscalls are allowed before syscalls in the xaction is finished<br >why is it implemented?
		* ![[Pasted image 20220930163118.png]]
		* 1:17:12 an example why block syscalls before the current open xaction finishes all syscall
			* ![[Pasted image 20220930163738.png]]
			* the freed inode in t2 seen by t1, yet t2 was abandoned by recovery program, so it's a disaster, we **lost atomicity**, uncommitted t2 change included in t1
		* write-ahead rule is important in recovery 
* Question
	* number of fs threads?
		* prefer one, may be many
	* if crash when writing t5 to home location not finished but t5 is freed? (01:24:??)
		* t8 eating up t5 as it is freed, covering t5's beginning blocks
	* why split commit block and desc block?
		* if integrated, we have to jump back to write commit block after writing data blocks
		* does not help distinguish commit block for t5(old freed xaction) or for t8(new)
		* ext4 writes out all data blocks and commit block together (ext3 commit block has to wait before data blocks finish)
			* if commit block exists but data blocks have not been written?
			* add a **checksum** field in commit block
	*  about data blocks?
		* multiple modes in ext3
			* journaled data(slow)
			* ordered data(popular)
				* only metadata in logs
				* file content directly to home location so it's much faster
				* more complexity
					* for example, crash when file content not fully written, content of previously deleted file may be exposed
					* we have to care about the order of writing inode and writing file content
					* for example, finish writing new file content before committing the xaction
			* 
	* 
