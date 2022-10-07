/*! \defgroup _system Ядро операционной системы

	загрузка системы и основной цикл выполнения фоновых процессов

	Система состоит из независимых модулей, которые регистрируются в ядре путем
	добавления записей в сегмент MOD \see module.h
	Каждый модуль может создавать контекст в динамической памяти и возвращать указатель на
	контекст в результе функции инициализации модуля. В основном цикле вызывается функция
	"сканирования" для каждого модуля.

	Определен следующий порядок загрузки системы:
-#	Подсистема сбора сообщений
-#	менеджер динамической памяти,
-#	выделение памяти "слайсами", использует менеджер динамической памяти, используется в системе для работы со списками
-#	загрузка конфигурации системы, параметры конфигурации применяются для настройки модулей
-#	инициализация системного таймера, процесс периодического запуска задач реального времени

	первые четыре системы необходимы для запуска и инициализации модулей.
*/
#include "cmsis_os.h"
#include "pio.h"
#include "module.h"
#include <stdio.h>
#include "atomic.h"
#include "semaphore.h"
#include <sys/thread.h>
//#include "r3stdlib.h"
//#include "r3v2protocol.h"
//#include "r3rtos.h"
//#include <string.h>
//static osService_t *services = NULL;

static int main_weak()
{
    return osOK;
}
/* возможно мы не хотим использовать треды, тогда достаточно не включать в компиляющию все что относится к osThread */
#pragma weak osKernelInitialize = main_weak
#pragma weak osKernelRunning = main_weak
#pragma weak osKernelStart = main_weak
//#pragma weak osThreadYield = main_weak
#pragma weak __libc_init_array = main_weak

//static void main_none(){}
//#pragma weak r3_slice_init = main_none
//#pragma weak events_init = main_none

static int  osVersion();
static int 	module_list();

extern ModuleInfo __start_ModList[];
extern ModuleInfo __stop_ModList[];

/*! \ingroup _system
	\brief жизненный цикл системы

*/
typedef void (*Callback)(void);

extern int main(void * data);
osThreadDef(main,osPriorityNormal,0,0x400);
extern void radian_bus(void * user_data);
//osThreadDef(radian_bus,osPriorityNormal,0,0x400);

int __main(void)
{
	if (osKernelInitialize () != osOK) { // check osStatus for other possible valid values
	// exit with an error message
	}
	if (!osKernelRunning ()) { // is the kernel running ?
		if (osKernelStart () != osOK) { // start the kernel
		// kernel could not be started
		}
	}
#if 0
// extern void low_level_init();
//	low_level_init(); // эта функция должна выполняться до копирования и инициализации памяти иначе всё может повиснуть.
  	TRACE_Init();	// система сбора сообщений
	events_init();	// инициализация потока событий
    r3_mem_init();	// распределение динамической памяти
    r3_slice_init();// выделение памяти слайсами
    r3_sys_ident();		// идентификация системы, одна и та же прошивка должна уметь адаптироваться под разный размер памяти или тип процессора
    r3_reset_ident();	// идентификация типа загрузки: по питанию, програмная перезагрузка или сбой
    FLASH_LoadConfig();	// загрузить конфигурацию устройства
  #if 0
    R3_port_irq_init();	// регистрация прерываний от ног, внешние события
  #endif
    r3_sys_init();	// инициализация системного таймера
#endif
// Конструкторы по сути вызываются перед  входом в MainThread
// __libc_init_array(): 
	extern Callback __start_init_array[];
	extern Callback __stop_init_array[];
	Callback* cb = __start_init_array;// конструкторы
	while (cb != __stop_init_array){
		(*cb)();
		cb++;
	}
	(void) osVersion(NULL, NULL);
#if (defined(osFeature_Modules) && (osFeature_Modules!=0))
    ModuleInfo * module = __start_ModList;
    while (module != __stop_ModList)
    {
        if (module->init != NULL) // бывают
        {   //костыль
			module->data = module->init(module->data);
        }
        module++;
    }
#endif
	// инициализаця OpenCL 
	(void) module_list();//NULL, NULL);
#if (defined(osFeature_OpenCL) && (osFeature_OpenCL!=0))
	void* command_queue=cl_command_queue_init(NULL);
#endif
	osThreadCreate(osThread(main), NULL);
#undef osFeature_MainThread
#define osFeature_MainThread 1
	while (1)
	{
#if (defined(osFeature_Modules) && (osFeature_Modules!=0))
		module = __start_ModList;
        while (module != __stop_ModList)
        {
            if (module->scan != NULL)
            {
                module->scan(module->data);
            }
            module++;
        }
#endif // osFeature_Modules
//		debug("#");
#if (defined(osFeature_Services) && (osFeature_Services!=0))// сервисы
extern void osServiceWorkFlow(void);
			osServiceWorkFlow();
#endif
		/* 		if (pio_get_state(GPIOB, 1<<8)!=0) pio_reset_output(GPIOB, 1<<8);
		else pio_set_output(GPIOB, 1<<8);*/
#if (defined(osFeature_OpenCL) && (osFeature_OpenCL!=0))
		(void) cl_command_queue_process (command_queue);
#endif
        if (osFeature_MainThread) {
			//osThreadYield();// отдыхаем до следующего прерывания
			__WFE();// если тред только один, начинаются тормоза при обработке модулей.
		}
	}
	while(1) ;
return 0;
}
/*! \ingroup _r3_cmd
	\{
*/
/*!	\brief распечатать список загруженных модулей
*/
static int module_list()//unsigned char* buffer, int* length)
{
    ModuleInfo *module = __start_ModList;
    puts("Listing loaded modules:\r\n");
    while (module != __stop_ModList)
    {
        printf("%s\r\n", module->name);
        module++;
    }
    return 0;
}

const char Version[]= BOARD_NAME" "osKernelSystemId" "__DATE__" "__TIME__"\r\n";
/*!	\brief вывести версию системы
*/
static int osVersion(unsigned char* buffer, int* length)
{
	puts(Version);
	return 0;
}
/*!	\brief выполнить перезагрузку системы
*/
int r3_sys_soft_reset(unsigned char* buffer, int*size)
{
    NVIC_SystemReset();
    return 0;
}
//! \}
