#include "r3_slist.h"

/*! \brief удалить элемент из списка

    \param list - поиск производится вниз по списку
    \param data - исключается первое появление таких данных
    \return начало списка
*/
GSList* g_slist_remove(GSList* list, const void* data)
{
    if (!list) return list; // нет элементов в списке
    GSList * first = list;
    if (list->data == data) { // в первом элементе
        first = list->next;
        g_slice_free(GSList, list);
    } else {
        GSList* prev = list;
        while ((list = list->next) != NULL)
        {
            if (list->data == data){
                prev->next = list->next;
                g_slice_free(GSList, list);
                break;
            }
            prev = list;
        }
    }
    return first;
}
