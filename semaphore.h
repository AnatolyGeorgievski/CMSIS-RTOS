#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include "atomic.h"
#include <time.h>
/*! \defgroup _semaphore_base Базовое определение семафоров

    Семафор — это объект, с которым можно выполнить три операции:
 - init(n): счётчик := n
 - enter(): ждать пока счётчик станет больше 0; после этого уменьшить счётчик на единицу.
 - leave(): увеличить счётчик на единицу.

    семафоры могут использоваться в ядре и в прерываниях. На базовом определении семафоров строятся \ref _memory_pool "osPool выделение памяти из таблицы", \ref _mutex "Мьютексы" и \ref _semaphore "Семафоры"

	\see The Open Group Base Specifications Issue 7, 2018 edition
	\see IEEE Std 1003.1-2017 (Revision of IEEE Std 1003.1-2008)
https://pubs.opengroup.org/onlinepubs/9699919799/
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
/*! \} */
/*! \ingroup _posix 
	\defgroup POSIX_SEMAPHORES POSIX: Semaphores 
	\{ */
#define SEM_FAILED NULL

typedef struct _sem sem_t;
int    sem_close(sem_t *);
int    sem_destroy(sem_t *);
int    sem_getvalue(sem_t *restrict, int *restrict);
int    sem_init(sem_t *, int, unsigned);
sem_t *sem_open(const char *, int, ...);
int    sem_post(sem_t *);
int    sem_timedwait(sem_t *restrict, const struct timespec *restrict);
int    sem_trywait(sem_t *);
int    sem_unlink(const char *);
int    sem_wait(sem_t *);

/*! \} */
#endif // SEMAPHORE_H
