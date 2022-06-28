#include "atomic.h"
#include "r3_tree.h"


struct _ttl_list {
    ttl_list_t* next;
    uint32_t key;
//    uint32_t timestamp;
    _Atomic void*    value;// переменная может заполняться из разных тредов
//    ttl_list_t* prev;
};
/*! поиск элемента перемещает его по списку вверх
 */
void* ttl_datalist_get  (ttl_datalist_t * dl, uint32_t key)
{
//    ttl_list_t* prev = NULL;
    ttl_list_t* list = dl->list;

    while (list){
        if (list->key == key) {
#if 0
            if(prev) {
                prev->next = list->next;// исключили элемент списка
                list->next = dl->list;
                dl->list = list;// переместили в начало списка
            }
            //list->timestamp = dl->timestamp;
#endif // 0
            return (void*)list->value;
        }
//        prev = list;
        list = list->next;
    }
    return NULL; // не найден
}
/*! при добавлении нас не волнует наличие в списке повторяющихся идентификаторов
список получается не по порядку.
 */
void  ttl_datalist_push (ttl_datalist_t * dl, uint32_t key, void* value)
{    // вставляем всегда в начало сипска!
    ttl_list_t* list = g_slice_new(ttl_list_t);
    list->key   = key;
    list->value = value;
    volatile void** ptr = (volatile void**)&dl->list;
    do {
        list->next  = atomic_pointer_get(ptr);
    } while(!atomic_pointer_compare_and_exchange(ptr, list->next, list));
}

void*  ttl_datalist_replace (ttl_datalist_t * dl, uint32_t key, void* value)
{    // вставляем всегда в начало сипска!
    ttl_list_t *list = dl->list;
    while (list) {
        if (list->key == key) {
            return (void*)atomic_exchange(&list->value, value);
        }
        list = list->next;
    }
    return NULL;
}
/*! \brief удалить элемент с данным идентификатором
 */
void* ttl_datalist_pop (ttl_datalist_t * dl, uint32_t key)
{
    ttl_list_t *prev = NULL;
    ttl_list_t *list = dl->list;
    while (list){
        //if (_expired(list->timestamp, dl->timestamp))
        if (list->key == key) {
            if (prev==NULL) dl->list = list->next;
            else prev->next = list->next;
            void* value = list->value;//(void*) atomic_load(&list->value);
            g_slice_free(ttl_list_t, list);
            return value;
        }
        prev = list;
        list = list->next;
    }
    return NULL;// не найден
}
/*! \brief удалить все элементы списка
 */
void ttl_datalist_clear(ttl_datalist_t * dl)
{
    ttl_list_t *list = atomic_exchange(&dl->list, NULL);
    while (list){
//        if (notify) notify(list->key, list->value);
        ttl_list_t *list_next = list->next;
        g_slice_free(ttl_list_t, list);
        list = list_next;
    }
}
void* ttl_datalist_get_iter(ttl_datalist_t * dl, uint32_t key, void** iter)
{
    ttl_list_t *list = *iter;
    while (list){
        if (list->key == key) {
            *iter = list;
            return (void*)list->value;
        }
        list = list->next;
    }
    return NULL;
}
void* ttl_datalist_get_next(ttl_datalist_t * dl, uint32_t *key, void** iter)
{
    ttl_list_t *list = *iter;
    if (list==NULL) {
        list = dl->list;
    }
    if (list!=NULL) {
        *key = list->key;
        *iter= list->next;
        return list->value;
    }
    *iter = NULL;
    return NULL;
}
void* ttl_datalist_foreach(ttl_datalist_t * dl, int (*test_cb)(uint32_t key, void* value, void* ), void* user_data)
{
    ttl_list_t *prev = NULL;
    ttl_list_t *list = dl->list;
    while (list){
        if (test_cb(list->key, (void*)list->value, user_data)) {
        //if (list->key == key) {
            ttl_list_t* next = list->next;
            g_slice_free(ttl_list_t, list);
            if (prev==NULL) {
                list = dl->list = next;
            } else {
                list = prev->next = next;
            }
        } else {
            prev = list;
            list = list->next;
        }
    }
    return NULL;// не найден
}
