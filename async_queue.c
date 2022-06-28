#include <stddef.h>
#include "atomic.h"
#include "cmsis_os.h"
#include "r3_slice.h"

typedef struct _List List_t;
struct _List {
    void* data;
    List_t* next;
};
/*
struct os_AsyncQ_cb {
    volatile List_t *head;
    volatile List_t *tail;
};*/
void osAsyncQueueInit(osAsyncQueue_t* queue)
{
    queue->head=NULL;
    queue->tail=NULL;
}

static List_t * list_reverse(List_t* list)
{
    List_t* top = NULL;
    while (list) {
        List_t* list_next = list->next;
        list->next = top;
        top = list;
        list = list_next;
    }
    return top;
}
void* osAsyncQueueGet(osAsyncQueue_t* queue)
{
    void* data = NULL;
    if (queue->head==NULL) {
//        volatile void** ptr = (volatile void**);
        queue->head = atomic_pointer_exchange(&queue->tail, NULL);
        if (queue->head) {
            queue->head = list_reverse((List_t*)queue->head);
        }
    }
    if (queue->head) {
        volatile List_t* list = queue->head;
        queue->head = list->next;
        data = list->data;
        g_slice_free1(sizeof(List_t), (void*)list);
    }
    return data;
}
void osAsyncQueuePut(osAsyncQueue_t* queue, void* data)
{
    List_t* tr = g_slice_alloc(sizeof(List_t));
    tr->data = data;
    volatile void** ptr = (volatile void**)&queue->tail;
    do {
        tr->next = atomic_pointer_get(ptr);
        atomic_mb();
    } while(!atomic_pointer_compare_and_exchange(ptr, tr->next, tr));
}
