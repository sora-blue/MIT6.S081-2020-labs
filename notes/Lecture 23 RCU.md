[FAQ](https://pdos.csail.mit.edu/6.S081/2020/lec/rcu-faq.txt)
* introduction
	* to get good multi-core performance when read a lot & write little
	* spinlocks disable parallel execution
* read/write lock
	* interface
		* r_lock(&)
		* r_unlock(&)
		* w_lock(&)
		* w_unlock(&)
	* allow many readers & only one writer
	* implementation (simplified ver)
		* ![[Pasted image 20221015154831.png]]
			* CAS(&l->n, x, x+1) // prevent writers write -1 after x < 0 check
			* x = l->n // grab a copy of l->n to prevent some problems
				* suppose two reads call `r_lock` at the same time
		* defect
			* writer may never succeed
			* `r_lock` could be very expensive
				* core cache invalidation & coherence
				* every increment, value cached in other cores has to be invalidated
				* n readers, every reader loops O(n) times
				* total cost: O(n^2)
			* writing will be much more expensive than reading
			* every write to shared data is a disaster for perf
* scenario: imagine a linked list
	* when do we risk reading while other modifying?
		* content changes
		* insert a new element
		* delete a new element 
* RCU
	* ideas
		* idea 1
			* demo procedure
				* initial state
				* ![[Pasted image 20221015161721.png]]
				* instead of replacing in place
				* ![[Pasted image 20221015161741.png]]
				* follows a single write, replacing them with new versions of data
				* ![[Pasted image 20221015161759.png]]
				* someone is reading E2 so not freeing for now
				* with a **single committing write**, we switch to second version of data, which deletes E2
					* double linked list not so good for RCU but tree is fine
		* idea 2
			* demo
				* from
				* ![[Pasted image 20221015162414.png]]
				* to
				* ![[Pasted image 20221015162435.png]]
				* compliers & micro CPUs **reorder memories**
				* say code is 
				* ![[Pasted image 20221015162515.png]]
				* it may go wrong because of reordering
				* ![[Pasted image 20221015162709.png]]
				* so readers & writes should use a barrier
				* ![[Pasted image 20221015162627.png]]
				* to finish any writes before barrier
				* ![[Pasted image 20221015162730.png]]
				* writer ensures the initialization before committing
				* Q: how could reorder r1->x before r1 = e1->next ?
					* i don't know
				* when to free E2?
					* refer to idea 3
		* idea 3
			* using a r/w lock scheme?
			* using garbage collected language?
			* rules
				* R: can't context switch
					* can't yield in a RCU critical section
					* so you need to keep op short, but not an indirect constraint, since lock critical area is always short
					* the real constraint is that you are not allowed to hold pointers to RCU protected data after context switch, prohibition against yielding CPU
				* W: delay free until every core has context switch at least once 
					* implementation: **`synchronize_rcu()`**
					* ![[Pasted image 20221015163915.png]]
					* synchronize_rcu() forces a context switch
					* due to rule 1, no core could still have the old value after synchronize_rcu()
					* writing for RCU protected data is relatively rare so not a big deal for overall perf
					* or you can use **`call_rcv`** that stashes writing op and returns instantly
	* simple use of RCU
		* ![[Pasted image 20221015164958.png]]
		* `rcu_read_lock` and `rcu_read_unlock` does nothing but set flag, preventing time interrupt from doing context switch
		* rcu doesn't prevent writers from interfering each other
		* `rcu_assign_pointer` issues a memory barrier
		* `sychronize_rcu` makes sure everyone reading `old` ends op before writer freeing it
		* in reader, pointer `e` is **not allowed** to be returned, which triggers a rohibited context switch, or you will violate RCU convention
			* you can copy the data pointed anyway
		* Perf: If you use RCU
			* reads to shared data would be extremely fast
			* writes in the opposite
				* `synchronize_rcu` gives up CPU
		* Q: If multi RAM sys?
			* ![[Pasted image 20221015170157.png]]
			* if two sets of CPU chip & its own chip
			* interconnect between two chips
			* chip exchanges info through its own mechanism to fetch data stored in another RAM
		* problems
			* RCU is not universally applicable, only helps when read outnumbers write
				* sequence locks to update data in place, but limited improvement to perf
				* when to write heavy data, better make data not shared
			* readers can see stale data
			* etc.
* summary
	* RCU eliminates locking & writing in readers
* questions
	* stale data in readers?
		* read just before updates
		* usually not a problem
	* O(n^2) also a problem for spinlock? why not mention?
		* locks have hideous costs
		* they can be extremely low if many cores are trying to get the same lock
	* locking in distributed systems?
		* people try to mimic shared data between independent machines
		* that's other stories