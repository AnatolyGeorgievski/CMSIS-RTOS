/*! Transactional memory */
/*! \see [ACLE](https://github.com/ARM-software/acle) 
	Arm C Language Extensions, Transactional Memory Extension (TME) intrinsics,Version: 2022Q2 
	
	Hardware Transactional Memory

Hardware Transactional Memory (HTM) 
	allows hardware extensions to ensure that memory accesses and code execution 
	are atomic around specific code areas that are defined by the code developer.

Here are the new instructions that are added to the PE for TME:

    TSTART <Xd>  ; Transaction start instruction
    TCOMMIT   ; Transaction code section end
    TCANCEL <#imm6> ; Transaction cancel with reason code
    TTEST   : Transaction status and nesting level test

	*/
#if defined(__ARM FEATURE TME)
#include <arm_acle.h>
#define _TMFAILURE_REASON 	0x00007fffu
#define _TMFAILURE_RTRY 	0x00008000u
#define _TMFAILURE_CNCL 	0x00010000u
#define _TMFAILURE_MEM 		0x00020000u
#define _TMFAILURE_IMP 		0x00040000u
#define _TMFAILURE_ERR 		0x00080000u
#define _TMFAILURE_SIZE 	0x00100000u
#define _TMFAILURE_NEST 	0x00200000u
#define _TMFAILURE_DBG 		0x00400000u
#define _TMFAILURE_INT 		0x00800000u
#define _TMFAILURE_TRIVIAL 	0x01000000u
uint64_t __tstart (void);

#endif
enum _TMStatus {
	TM_ABORT, TM_ACTIVE, TM_COMMIT
};
typedef struct _Trans trans_t;
typedef uintptr_t blockid;
struct _Trans {
	enum _TMStatus code;
};
struct _Locator {
	trans_t  trans; // pointer to transaction descriptor 
	void* old_data; // (previously) committed data 
	void* new_data; // tentative or newly committed data
};
/*! \brief Allocates a new logical block. 
	\param Size in bytes of block to be allocated. 
	\return Logical block identifier. Remarks: Other transactional memory calls identify the new block by using the block id.*/
blockid tm_alloc(int size){
}
/*! \brief Deallocate a logical block that is  no longer required. 
	\param Block id of logical block to be deallocated. 
	\return None. 
	\note In the examples used herein, we have not addressed the deallocation of logical blocks after use. This issue can in many cases be addressed by using Value Recycling techniques we have separately pro posed, in which case we would use the tm_delete() operation to deallocate logical blocks.  */
void tm_delete(blockid b) {}
/*! \brief Used to initialize a newly-allocated block. 
	\param Block id of logical block to be initialized. 
	\return Pointer to data block where initial value can be stored. 
	\note The returned pointer can be used only until the first call to tm open for this block. Initialization is not part of a transaction; even if initialization is done within a transaction that later aborts, the initialization remains.*/
void*tm_init(blockid b) 
/*! \brief Determine size of logical block. 
	\param Block id. 
	\return Size in bytes of specified logical block. 
	\note A logical block's size is the size specified when it was allocated by tm alloc(). */
int tm_sizeof(blockid b) {

}
/*! \brief Explicitly abort current transaction. 
	\param None. 
	\return None. 
	\note An alternative is to treat tm start() as an abort if it is invoked during a transaction. However, some implementations might perform better if transactions that are not intended to be completed are explicitly aborted.*/
void tm_abort(trans_t *trans) {
	trans->code = TM_ABORT;
}
/*! \brief Attempt to commit current transaction. 
	\param None. 
	\return True if transaction commits successfully, false otherwise. */
bool tm_commit(trans_t *trans) {
	/* validating the entries in the read-only table */
	return CAS(&trans->code, TM_ACTIVE, TM_COMMIT);
}
/*! \brief Determine whether current transaction can still commit Successfully. 
	\param None. 
	\return False if current transaction of invoking thread is destined to abort, true if it can still commit. 
	\note Useful for determining whether data read so far in transaction was all consistent (if not, transaction will not commit successfully). Can also be used to avoid expensive computation if the transaction cannot com mit. */
bool tm_validate(trans_t *trans) {
/* For each pair (o, v) in the calling threads read-only
table, verify that (v) is still the most recently committed version of (o). */
/* Check that the status field of the Transaction object remains ACTIVE. */
	return (&trans->code == TM_ACTIVE);
}
/*! \brief Start a transaction. 
	\param None. 
	\return None. 
	\note In the simple API presented here for the purposes of introducing the  idea  of dynamic transactional memory, we assume that transactions are not nested. For this simple case, no parameters are needed for tm_start (). It is straight forward to extend the API, and the implementation approaches described later, to Support nested transactions.*/
trans_t* tm_start(trans_t *trans){
//	trans_t *trans = g_slice_new(trans_t);
	trans->code = TM_ACTIVE;
	trans->id = atomic_fetch_add(trans_unique_id, 1);
	return trans;
}
/*! \brief Open a logical block for access within a transaction.
	\param Block id of logical block to be accessed. 
	\return Pointer to private copy of logical blocks COntentS. 
	\note Invoked within a transaction (i.e., after the invoking thread has called tim start() and before it has Subsequently called tim abort( )  or tim commit()). Returns a pointer to a tentative data block which can be used for reading values from the block and for storing values that will take effect if and when the enclosing transaction Successfully commits. Multiple calls to tim open() for the same block in the same transaction will return the same pointer. Calls to tm open() for different blocks in the same transaction may yield incon sistent values, but any transaction that observes Such inconsistencies is guaranteed not to commit Success fully. Thus, code that risks behaving incorrectly if it observes inconsistent data should call tm validate() (described later) to ensure data is valid before executing the risky code. Pointers returned by tm open() should be used only within the lifetime of the enclosing trans action and should not be stored in data blocks.
*/
void*tm_open (trans *t, blockid b){
	
}
/*! \brief Open a logical block within a transaction for read only access. 
	\param Block id of logical block to be read. 
	\return Pointer to private copy of logical blocks COntentS. 
	\note Data written to the private copy will not be stored to the logical block upon Successful commit of the trans action.*/
void*tm_read(trans *t, blockid b){
	
}  
/*! \brief Release a block previously opened with tim read () from the transaction. 
	\param Block id of logical block to be released from transaction. 
	\return None.*/
void tm_release(trans *t, blockid b){
	
}  

typedef 
struct _List {
	blockid next;
	uint8_t data[0];
};

bool tm_list_delete(blockid head, )
{
	do {
		tx = tm_start();
		blockid prev_blk = head;//tm_read(tx, head);
		List_t *node;
		while((node = tm_read(tx, prev_blk)) && tm_validate(tx)){
			if (node->key == key) {
				break;
			}
			rel = prev_blk;
			prev_blk = &node->next;
			tm_release(tx, rel);// операция read->reuse
		}
		if (node->key == key)
		{
			prev = tm_open(tx, prev_blk);
			prev->next = node->next;
			tm_delete(node);
		}
	} while (!tm_abort(tx));
}

const tm_dummy_transaction = {
	.code=TM_COMMIT,
	.old_data = NULL;
	.new_data = NULL;
};
head = &tm_dummy;
static void* tm_read_commited(tx, locator_t* v)
{
	trans_t *trans = v->trans;// трнзакция предыдущая
	do {
		code = atomic_get(&trans->code);
		if (code != TM_ACTIVE) {
			break;
		}
	}while (!atomic_compare_exchange(&trans->code, code, TM_ABORT));
	// transaction is now either committed or aborted
	return (code==TM_COMMIT? v->new_data: v->old_data);
}
tm_open(tx, blk)
{
	tx->old = ref;
	
	l->tx = tx;
	clone->
	l->trans = tx;
	do {
		v = _dereference(blk);
		l->old_data = tm_read_commited(blk);
		tm_clone(l->new_data, l->old_data, DATA_SIZE);
	} while (!CAS(blk, v, l));
}
tm_read(tx, )
{
	do {
		ref = atomic_get(link);
		if ((ref & WRITE_LOCK) == 0) break;
	}while(!CAS(link, ref, ref & MASK)); // оборвать чужие транзакции
	return ref & MASK;
}
tm_start(trans_t * tr){
	//запросить идентификатор версии
	do {
		tx = atomic_get(global_tx);
		tr->trans = tx;
	} while (!CAS(global_tx, tx, tr));
	
}
#define rcu_dereference(ptr)
#define rcu_assign_pointer(ptr, v)
#define rcu_read_lock()
#define rcu_read_unlock()

tm_commit(trans_t * tr)
{
	if(atomic_fetch_sub(tr->readers,1)==0){
		// удалить мусор
		
	} else {
		osEventWait(osWaitSemaphore, &tr->lock,...);
	}
}
tm_commit(trans_t * tr){
	if (*tr->write==NULL) {
		return 1;
	}
	v = __LDREXW(tr->write);//
	for(ref as prev=>value) {
		if (prev->value != value) {
			atomic_clear();
			break;
		}
	}
	consistent = __STREXW(tr->new, tr->write);
	if (consistent) {
		
	}
}

// пометить на удаление элемент списка
rcu_write(tx, oid) {
rcu_delete(tx, oid) {
	tx->garbage[i++] = oid;
}

tm_commit(){
	
	if (prev_trans->code == COMMIT){
		CAS(trans->ptr, trans->old_data, trans->new_data);
	}
	
}