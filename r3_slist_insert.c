#include "r3_slist.h"
// TODO не проверял

typedef int (*GCompareFunc)(void*, void*);
/*! \brief добавить элемент в список перед указанным
    \param list - список
    \param sibling - элемент списка перед которым вставляется новый элемент
    \param data - данные
    \return начало списка
*/
GSList* g_slist_insert_before(GSList * list, GSList* sibling, void* data)
{
//    if (!list) return list;
    GSList* first = list;
    if (first == sibling || first==NULL){ // перед первым элементом
        first = g_slice_new(GSList);
        first->data = data, first->next = sibling;
    } else {
        while (list->next)
        {
            if (list->next == sibling){
                break;
            }
            list = list->next;
        }
        // доехали до конца списка, не нашли, вставим в конец
        list->next = g_slice_new(GSList);
        list = list->next;
        list->data = data, list->next = sibling;
    }
    return first;
}
GSList* g_slist_insert_after (GSList * list, GSList* sibling, void* data)
{
    GSList * elem = g_slice_new(GSList);
    elem->data = data, elem->next= NULL;
    if (sibling == NULL) {
        list = elem;
    } else {
        elem->next = sibling->next;
        sibling->next = elem;
    }
    return list;
}

/*! \brief добавить элемент в список с сортировкой
    \param list - список
    \param data - данные
    \param func - функция сравнения, возвращает >0 если a после b
    \return начало списка
*/
GSList* g_slist_insert_sorted (GSList *list, void* data, GCompareFunc func)
{
    GSList* first = list;
    if (first == NULL || func(list->data, data)>0) {// первым элементом списка
        first = g_slice_new(GSList);
        first->data = data, first->next = list;
    } else {
        GSList* prev = first;
        list = list->next;
        while (list){
            if (func(list->data, data)>0) break;
            prev = list;
            list = list->next;
        }
        prev->next = g_slice_new(GSList);
        prev = prev->next;
        prev->data = data, prev->next = list;
    }
    return first;
}
