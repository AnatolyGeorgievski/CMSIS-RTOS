//#include "r3stdlib.h"
#include "r3_slist.h"
#include "atomic.h"
/*! \defgroup _r3_slist Односвязные списки объектов

Представлена реализация списков односвязных совместимая со списками Glib.
В Glib списки двухсвязные.
    \{
*/
/*! \brief добавить элемент в конец списка
    \param list - список
    \param data - данные
    \return начало списка
*/
GSList* g_slist_append(GSList* list, void* data)
{
    GSList * last = g_slice_new(GSList);
    last->data = data, last->next = NULL;
    if (list == NULL) return last;
    GSList * first = list;
    while (list->next != NULL) list = list->next;
    list->next = last;
    return first;
}

/*! \brief освободить место занятое списком

Элементы списка должны освобождаться отдельно
    \param list - список
*/
void   g_slist_free  (GSList *list)
{
    while(list){
        GSList* prev = list;
        list = list->next;
        g_slice_free(GSList, prev);
    }
}
/*! \brief добавить элемент в начало списка

    \param list - начало списка
    \param data - данные для нового элемента списка
    \return начало списка
*/
GSList* g_slist_prepend(GSList* list, void* data)
{
    GSList * last = g_slice_new(GSList);
    last->data = data, last->next = list;
    return last;
}

GSList* g_slist_prepend_atomic   (GSList** head, void* data)
{
    GSList* list = g_slice_new(GSList);
    list->data = data;
	volatile void** ptr = (volatile void**)head;
	do {
		list->next = atomic_pointer_get(ptr);
		atomic_mb();
	} while (!atomic_pointer_compare_and_exchange(ptr, list->next, list));
	return list;
}

/*! удалить один элемент списка */
/*void   g_slist_free_1(GSList *list)
{
    g_slice_free(GSList, list);
}*/
#if 0

#endif

/*! \brief последний элемент списка
    \param list - список
    \return последний элемент списка
*/
GSList* g_slist_last(GSList* list)
{
    if (list)// return list;
    while (list->next) list = list->next;
    return list;
}

/*! \brief N-й элемент списка
    \param list - список
    \return последний элемент списка
*/

GSList* g_slist_nth   (GSList* list, unsigned int n)
{
    while (list && n!=0){
        n--;
        list = list->next;
    }
    return list;
}

/*! \brief подсчет элементов в списке
    \param list - список
    \return количество элементов в списке
*/
int g_slist_length(GSList* list)
{
    int count = 0;
    while (list){
        count++;
        list = list->next;
    }
    return count;
}



//! \}
