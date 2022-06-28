#include "r3_slist.h"

/*! \brief копирование списка без копирования данных
    \param list исходный список
    \return копия списка
 */
GSList* g_slist_copy (const GSList *list)
{
    if (list==NULL) return NULL;
    GSList* top= g_slice_new(GSList);;
    top->data = list->data;
    GSList * prev = top;
    list = list->next;
    while (list) {
        GSList * item = g_slice_new(GSList);
        item->data = list->data;
        prev->next = item;
        prev = item;
        list = list->next;
    }
    prev->next = NULL;
    return top;
}
