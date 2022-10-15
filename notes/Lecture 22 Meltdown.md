* basic idea
	* ![[Pasted image 20221014204320.png]]
	* `r2 = *r1` does not work directly
	* rely on 
		* kernel va all mapped to user PTE
		* CPU speculative execution
		* CPU caching
	* Q: guess kernel addr?
		* KASLR does some randomization
		* but given enough time, attacker can hack it
* speculative execution
	* ![[Pasted image 20221014205329.png]]
	* I/O waiting wastes time
	* so CPU takes branch prediction
	* if r0 is an invalid pointer?
		* whether valid or not is deferred hundreds of cycles
	* retirement
		* when we know an instruction is going to be thrown or committed
		* page fault may come hundreds of cycles after loading mem
	* Q: when `r2 = *r0*`, flags are checked later?
		* yep, only when loaded will permissions be checked
		* and exec state will be cancelled
	* Q: Can't cpu check permissions before loading?
		* Intel internal implementation matters
		* They could have done it
		* Meltdown seems not work on AMD CPUs
		* maybe AMD will not even do the speculation when the addr is invalid
* caches
	* ![[Pasted image 20221014215706.png]]
	* architectural & micro-architectural
		* architectural:       what the manual says it should look like
		* micro-architectural: actual implementation
	* a simple scheme
		* L1: va | data | perms
		* if miss L1 cache -> TLB(Translation Look-aside Buffer)
		* L2: pa | data
		* if miss L2 cache -> RAM
	* when changing pgtbl, L1 cache will have to be flushed as it's va
	* in paper's date, it seemed that most os did not flush it
	* user can tell if data is cached by load time
	* intel always keep detailed micro architectural design secret
* flush + reload
	* suppose we're interested in addr X
		* clflush x // ensure location not cached in any other cache
			* even without it, you can flush everything in cache
		* f()
		* a = rdstc
			* rdstc tells cycle grained time that passed
		* junk = \*x // load data in X
		* b = rdstc
		* b - a
			* calculate cycles passed to tell if f() used X
* meltdown including flush + reload
	* ![[Pasted image 20221014213223.png]]
	* make sure our buffer not cached
	* when `r2 = *r1` is retired matters
		* too early
			* ![[Pasted image 20221014213424.png]]
		* expensive instruction to put the retirement off as much as possible 
	* **line 13 would cause sth to be loaded into the cache**
	* through user page fault handler or sth...
		* use flush+reload to figure out whether the low bit of kernel data is  0 or 1
	* Q: why 1 bit a time?
		* buf should be 2^(wanted page) * PAGE_SIZE
		* too much mem taken
		* the paper also argues that 
			* 1 bit a time -> 1 flush+reload
			* 1 byte a time -> 256 flush+reload
			* so 1 bit a time could actually be faster
	* Q: js or WebAssembly implementation?
	* listing(4) shows that meltdown frequently fails, and it's not fully explained yet
* fixes
	* KASLR (`kpti` in linux)
		* just lift kernel mappings out of user pgtbl
		* 5% extra cost for pgtbl switching
	* PCID approach
		* avoid pgtbl switch costs
	* hardware fix
	* Q: KASLR fix reverted after hw fix?
		* you can check hw fixes in linux kernel by `lscpu`
		* so things may work this way
* Questions
	* L3 cache?
		* L3 cache may be shared L2 cache
	* TLB consults cache for PTE
		* TLB reload is just mem access
		* may consult L2
	* spectre?
		* train branch predictor
		* branch predictor kind of shared
		* flush+reload
	* fix before publishing paper?
		* there's a total mechanism, or paper won't be published
		* hackers might have found it out since 20 years ago :)