#include <cmsis_os.h>
#include "atomic.h"
#include "semaphore.h"
//#include "queue.h"
//#include "thread.h"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/// приватное определение мьютекса
struct os_mutex_cb {
    volatile int count; //!< счетчик семафора, начальное значение счетчика \b 1
    //osThreadId owner;
};
/*!	\defgroup _mutex RTOS: Mutex management
	\ingroup _rtos
    Мьютексы -- это семафоры с счетчиком 1, бинарные семафоры.
    \{
    */
/*! \brief создать мьютекс

    \param mutex_def статическое определение мьютекса \ref osMutexDef
    \return идентификатор или указатель на мьютекс
 */
osMutexId osMutexCreate (const osMutexDef_t *mutex_def)
{
	osMutexId mutex_id = mutex_def->dummy;
	semaphore_init(&mutex_id->count, 1);
	return mutex_id;
}
/*! \brief запросить мьютекс

    \param [in] mutex_id идентификатор (ссылка на блок управления) треда
    \param [in] millisec таймаут ожидания получения доступа, миллисекуны
	\return Status and Error Codes
	- \b osOK: the mutex has been obtain.
    - \b osErrorTimeoutResource: the mutex could not be obtained in the given time.
    - \b osErrorResource: the mutex could not be obtained when no timeout was specified.
    - \b osErrorParameter: the parameter mutex_id is incorrect.
    - \b osErrorISR: osMutexWait cannot be called from interrupt service routines.
*/
osStatus osMutexWait (osMutexId mutex_id, uint32_t millisec)
{
    int count = semaphore_enter(&mutex_id->count);
    if (count) return osOK;

    osEvent event = {.status = osEventSemaphore, .value={.p = (void*)&mutex_id->count}};
    osEventWait(&event, millisec);
    if (event.status == osEventTimeout) return osErrorTimeoutResource;
    return osOK;
}

/*! \brief освободить мьютекс

    \param [in] mutex_id идентификатор мьютекса полученный через функцию \ref osMutexCreate
    \return Status and Error Codes
    - \b osOK: the mutex has been correctly released.
    - \b osErrorResource: the mutex was not obtained before.
    - \b osErrorParameter: the parameter mutex_id is incorrect.
    - \b osErrorISR: osMutexRelease cannot be called from interrupt service routines.

    \todo при удалении объекта обеспечить целостность очереди, чтобы если ref>0 не обрывалась очередь
*/
osStatus osMutexRelease (osMutexId mutex_id)
{
    semaphore_leave(&mutex_id->count);
    return osOK;
}
/*!	\brief удалить мьютекс

    \param [in]	mutex_id	идентификатор или указатель на мьютекс полученный от \ref osMutexCreate.
    \return Коды завершения операции
    - \b osOK: the mutex object has been deleted.
    - \b osErrorISR: osMutexDelete cannot be called from interrupt service routines.
    - \b osErrorResource: all tokens have already been released.
    - \b osErrorParameter: the parameter mutex_id is incorrect.

 */
osStatus osMutexDelete (osMutexId mutex_id)
{
    semaphore_init(&mutex_id->count,0);
	return osOK;
}

//! \}
