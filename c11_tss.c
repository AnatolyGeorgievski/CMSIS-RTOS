/* Thread-specific storage functions */

#include <threads.h>
#include "thread.h"
#include "r3_slice.h"
struct os_tss {
	void* value;
	tss_dtor_t dtor;
	struct os_tss* next;
};

int   tss_create(tss_t *key, tss_dtor_t dtor)
{
	tss_t tss = g_slice_new(struct os_tss);
	tss->value= NULL;
	tss->dtor = dtor;
	thrd_t thr= thrd_current();
	tss->next = thr->tss;
	thr->tss = tss;
	*key = tss;
	return thrd_success;
}
void  tss_delete(tss_t tss)
{
	void* val = tss->value;
	tss->value = NULL;
	if (tss->dtor!= NULL && val!=NULL) tss->dtor(val);
}
void *tss_get(tss_t tss)
{
	return (tss->value);
}
int   tss_set(tss_t tss, void *val)
{
	tss->value = val;
	return thrd_success;
}
void  tss_destroy(tss_t tss)
{
	while(tss!=NULL) {
		tss_t tss_next = tss->next;
		tss_delete(tss);
		g_slice_free(tss_t, tss);
		tss = tss_next;
	}
}