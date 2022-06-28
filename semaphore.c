#include <cmsis_os.h>
#include "semaphore.h"

/*! \ingroup _system
    \defgroup _semaphore Semaphore Management

    \{
    */
/// семафор приватное определение
struct os_semaphore_cb {
    volatile int count; // счетчик
	// можно дополнять
};
/*! \brief создать (инициализировать) семафор

    \param semaphore_def статическое определение семафора
    \param count максимальное значение счетчика
    \return указатель или идентификатор семафора

 */
osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count)
{
    osSemaphoreId semaphore_id = (osSemaphoreId)semaphore_def;
    semaphore_init(&semaphore_id->count, count);
    return semaphore_id;
}
/*! \brief освободить семафор

    \param semaphore_id указатель или идентификатор семафора
    \return статус завершения операции \ref osStatus

 */
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id)
{
    semaphore_leave (&semaphore_id->count);
    return osOK;
}
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id)
{
    semaphore_init(&semaphore_id->count, 0);
    return osOK;
}
/*! \brief запросить доступ к семафору

    \param semaphore_id идентификатор (ссылка на блок управления) треда
    \param millisec таймаут ожидания получения доступа, миллисекуны
	\return number of available tokens, or -1 in case of incorrect parameters.
*/
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec)
{
    int count = semaphore_enter(&semaphore_id->count);
    if (count) return count-1;

    osEvent event = {.status = osEventSemaphore, .value={.p = (void*)&semaphore_id->count}};
    osEventWait(&event, millisec);
    if (event.status == osEventTimeout) return -1;
    return event.value.v-1;
}
//! \}
