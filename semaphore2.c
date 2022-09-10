#include <cmsis_os2.h>
#include "semaphore.h"

#include "r3_slice.h"
#include <sys/thread.h>
#include <svc.h>
typedef struct _thread osThread_t;
extern volatile osThread_t* current_thread;

/*!	\defgroup _semaphore2 RTOSv2: Semaphore Management
	\ingroup _rtos2
    \{
    */
/// семафор приватное определение
typedef struct os_semaphore_cb osSemaphore_t;
struct os_semaphore_cb {
    volatile int count; // счетчик
	// можно дополнять
};
osSemaphoreId_t 	osSemaphoreNew (uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr)
{
	osSemaphore_t *sem = g_slice_alloc(sizeof(struct os_semaphore_cb));
	semaphore_init(&sem->count, initial_count);
	return sem;
}
/*! \brief освободить семафор

    \param semaphore_id указатель или идентификатор семафора
    \return статус завершения операции \ref osStatus

 */
osStatus_t osSemaphoreRelease (osSemaphoreId_t semaphore_id)
{
	osSemaphore_t* sem = semaphore_id;
    semaphore_leave (&sem->count);
    return osOK;
}
osStatus_t osSemaphoreDelete (osSemaphoreId_t semaphore_id)
{
	osSemaphore_t* sem = semaphore_id;
    semaphore_init(&sem->count, 0);
    return osOK;
}
/*! \brief запросить доступ к семафору

    \param semaphore_id идентификатор (ссылка на блок управления) треда
    \param millisec таймаут ожидания получения доступа, миллисекуны
	\return number of available tokens, or -1 in case of incorrect parameters.
*/
osStatus_t osSemaphoreAcquire (osSemaphoreId_t semaphore_id, uint32_t millisec)
{
	osSemaphore_t* sem = semaphore_id;
    int count = semaphore_enter(&sem->count);
    if (count) return osOK;

    svc3(SVC_EVENT_WAIT, osEventSemaphore, (uintptr_t)&sem->count, millisec*1000);
	int status = current_thread->process.event.status;
    if (status == osEventSemaphore) return osOK;
    if (status == osEventTimeout) return osErrorTimeout;
    return osErrorISR;
}
//! \}
