#include "cmsis_os.h"
#include "atomic.h"
#include "queue.h"

#ifndef NULL
#define NULL ((void*)0)
#endif // NULL

/*! \ingroup _system
    \defgroup _queue Неблокирующая асинхронная очередь

Очередь предназначена для одновременной работы одного получателя и множества отправителей.
Очередь не блокирует доступ и не вызывает ожидания тредов.

Идея: имеется два односвязных
списка: один используется для постановки в очередь, второй список используется для чтения
объектов из очерди. Если список чтения пуст, производится подмена и копирование списка
записи в список чтения. Операции доступа к указателям списков выполнены на атомарных операциях
чтения и замены указателей \ref _atomic "atomic_pointer_get", \ref _atomic atomic_pointer_compare_and_exchange.

    \{
 */


/*! \brief взять элемент из очереди
    \param q очередь объектов
    \return первый элемент очереди
 */
List_t* queue_pop (Queue * q)
{
    if (q->head != NULL) {
        List_t * el = q->head;
        if (el) q->head = el->next;
        return el;
    }
    volatile void** ptr = (volatile void**)&q->tail;
    List_t *el =  (List_t*)atomic_pointer_exchange(ptr, NULL);
    if (el==NULL) return NULL;
    List_t *head = NULL;
    while (el->next) { // перевернуть вверх дном
        List_t * next = el->next;
        el->next = head;
        head = el;
        el = next;
    }
    q->head = head;
    return el;
}

/*! \brief добавить в очередь элемент.

 */
void queue_push(Queue * q, List_t* el)
{
    volatile void** ptr = (volatile void**)&q->tail;
    List_t* tail;
    do {
        tail = atomic_pointer_get(ptr);
        el->next = tail;
        atomic_mb(); // может и не требуется, используется чтобы гарантировать что в память записан элемент next
    } while (!atomic_pointer_compare_and_exchange(ptr, tail, el));
}
/*! \} */
