/*! Обработка сервисных запросов

В данную группу попадают только те системные функции, которые могут сопровождаться переключением контекстов задач
например usleep()

2020-11-02 Переписать под POSIX

*/
#include "board.h"
#include "cmsis_os.h"
#include "svc.h"
//#include "r3rtos.h"
#include <stdio.h>
/*! \ingroup _stdlib
    \{
 */
__attribute__((noinline)) int usleep(useconds_t usec);
/*! \brief задержка исполнения на микросекунды
 */
int usleep(useconds_t usec){    svc(SVC_USLEEP);}

/* 
#include <unistd.h>
unsigned sleep(unsigned seconds);
unsigned nanosleep(unsigned seconds);

#include <signal.h>
int kill(pid_t pid, int sig);
#include <time.h>
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);
int clock_nanosleep(clockid_t clock_id, int flags,
       const struct timespec *rqtp, struct timespec *rmtp);
*/
/*! \brief выделение динамической памяти
 */
//__attribute__((noinline))
//void* malloc(unsigned int size){    svc(SVC_MALLOC);}

/*! \brief освободить динамическую память
 */
//__attribute__((noinline))
//void free(void* ptr) {    svc(SVC_FREE);}

__attribute__((noinline)) void kill(void* thread){    svc(SVC_KILL);}

/*! \} */

static void svc_handler_main(unsigned int *frame, int svc_number);
static SvcCallback svcs[]= {
[0 ... SVC_COUNT] = svc_handler_main, // значение по умолчанию
};
/*! \brief регистрация системной функции 
 */
void svc_handler_callback(SvcCallback cb, int svc_number)
{
    if (0<=svc_number && svc_number<SVC_COUNT)
        svcs[svc_number] = cb;
}

//we need to decrease the optimization so the the compiler
//does not ignore func and args
// xSP : R0 R1 R2 R3 R12 R14 PC xPSR
//       |svc_args[0]        |svc_args[6]
//                      00 DF|XX XX XX XX <-- code will be executed after reti
//                      -2 -1|0  1  2  3
// __attribute__(( naked ))
void SVC_Handler()
{
#if 0 //(defined(__CORTEX_M) && __CORTEX_M==0)
	// этот вариант может не работать если компилятор сует LR в стек до вызова MSP
	register unsigned int LR __asm("lr");
	register unsigned int* frame = (LR & 4)?(void*)__get_PSP():(void*)__get_MSP();
	register unsigned int svc_num = ((unsigned char *)(frame[6]))[-2];
	svcs[svc_num](frame, svc_num);
#elif defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_8M_MAIN__)
	register SvcCallback* cbs = svcs;
    __asm volatile(
# if 0 // если используется два стека
		"tst lr, #4\n" // Check EXC_RETURN[2]
        "ite eq\n"      // используется стек процесса psp или стек системы msp
        "mrseq r0, msp\n"
        "mrsne r0, psp\n" 
# elif 1// предполагаем обращение только из Thread mode PSP
		"mrs  r0, psp\n\t"
# else // Thread mode MSP
        "mrs  r0, msp\n"// загрузить указатель стека, в нашей реализации используется только msp
#endif
		"ldr  r1, [r0, #24]\n\t" // загрузили указатель команд со стека
		"ldrb r1, [r1, #-2]\n\t" // загрузили параметр инструкции SVC
		"ldr  pc, [%[cbs], r1, LSL #2]\n\t" // добавили смещение и перешли к выполнению
        ::[cbs] "r" (cbs) :"r0", "r1");
#elif defined(__ARM_ARCH_8M_BASE__) || (__ARM_ARCH_6M__)
	register SvcCallback* cbs = svcs;
    __asm volatile(
		"mrs  r0, psp\n"	   // предполагаем обращение только из Thread Mode PSP
		"ldr  r1, [r0, #24]\n" // загрузили указатель команд со стека
		"sub  r1, r1,#2\n"
		"ldrb r1, [r1]\n"         // загрузили параметр инструкции SVC
		"lsl  r1, r1,#2\n"
		"ldr  r1, [%[cbs], r1]\n" // добавили смещение и перешли к выполнению
		"bx   r1\n"
        ::[cbs] "r" (cbs) :"r0", "r1");
#else
	#error "Need SVC procedure call"
#endif
}

static void svc_handler_main(unsigned int *frame, int svc_number)
{
	switch (svc_number){
	case SVC_YIELD:
		__YIELD();
		break;
	default:
		printf("SVC %02X\n", svc_number);
		break;
	}
}
