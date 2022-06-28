#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include "atomic.h"
/*! \defgroup _semaphore_base Базовое определение семафоров

    Семафор — это объект, с которым можно выполнить три операции:
 - init(n): счётчик := n
 - enter(): ждать пока счётчик станет больше 0; после этого уменьшить счётчик на единицу.
 - leave(): увеличить счётчик на единицу.

    семафоры могут использоваться в ядре и в прерываниях. На базовом определении семафоров строятся \ref _memory_pool "osPool выделение памяти из таблицы", \ref _mutex "Мьютексы" и \ref _semaphore "Семафоры"

    \{
*/

#define SEMAPHORE_INIT(n) (n)
/*! \brief инициализация семафора

 */
static inline void semaphore_init(volatile int * ptr, int count)
{
    *ptr = count;
//    atomic_mb();
	//__DMB();
}
/*! \brief вход в критическую область

    Функция НЕ блокирует исполнение, только убавляет счетчик входов на единицу.

    \return счетчик семафора до вычитания, 0 - семафор занят.
 */
static inline int semaphore_enter(volatile int * ptr)
{
    register int count;
    do {
        count = atomic_int_get(ptr);
        if((count==0)) {
			atomic_free(); 
			// __ldrex и __strex идут в паре, надо выполнить операцию __clrex, но кажется не обязательно
			break;
		}
    } while(!atomic_int_compare_and_exchange(ptr, count, count-1));
    //atomic_mb();
	//__DMB();
    return count;
}
/*! \brief выход из критической области
 */
static inline int semaphore_leave(volatile int * ptr)
{
//	__DMB();
    return atomic_fetch_add(ptr, 1);
    //atomic_mb();// сбрасываем ?
}

//! \}
#endif // SEMAPHORE_H
