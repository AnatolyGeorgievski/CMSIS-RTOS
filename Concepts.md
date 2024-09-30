
## Модель GPIO-v2 ##

Мы принимаем некоторую модель статического описания конфигурации портов ввода-вывода, и стараемся сделать ее переносимой между платформами.

Порты разделены на группы на разных платформах порты могут иметь различную разрядность 8, 16, 32, 64 бита. Маска порта машинно-зависимый тип. На контроллерах Cortex-M4 маска 16 бит:
```c
typedef uint16_t GPIO_Mask_t;
```
Мы стремимся к совместимости нашей модели и описания портов GPIO-V2, принятой в Linux. На "голой" платформе под управлением RTOS мы используем свой сопсоб статического описания портов, который можно привести к модели Linux. 

Статический способ описания включает:
1. Идентификатор аппаратуры порта (gpiochip#, GPIOA..GPIOF)
2. Имя порта, как оно отображается в системе, для диагностики.
3. Маска используемых линий порта
4. Флаги: {`INPUT`,`OUTPUT`,`ALTERNATE`,`ANALOG`} можно комбинировать с флагами {`BIAS_PULLUP`, `BIAS_PULLDOWN`} для входных портов, или {`PUSH_PULL`, `OPEN_DRAIN`} для выходных. Кроме того, существует возможность добавлять атрибуты, включая полярность сигналов `ACTIVE_LOW`, `ACTIVE_HIGH` и `EDGE_RISING`, `EDGE_FALLING`. 
5. Линии могут быть настроены на специальную функцию, например, I2С, SPI или UART. Это определяется вариантом ALT# - альтернативная функция порта.
6. Атрибуты: Начальное состояние линий OUTPUT задается маской. 

Все статические определения портов и привязки к платформе выполняются в локальной конфигурации `"board.h"`.

Для статического описания GPIO используется таблица GPIO_Config, которая обрабатывается до запуска main() и до запуска статических конструкторов (в процессе низкоуровневой инициализации). Вся структура GPIO_Config описывается в файле заголовка "board.h"

Интерфейс работы с портом включает операции gpio_port_set, gpio_port_reset, gpio_port_set_reset по маске, которые должны иметь аппаратную поддержку, обеспечивающую одновременное изменение сигналов. Понятие маски отличается при статическом описании используются смещения - физическая нумерация линий. А при работе с настроенным портом используется логическая организация - номер линии заданный при перечислении. При чтении с порта возвращается маска линий в активном состоянии. gpio_port_state - возвращает логическую маску с учетом полярности сигналов и признаков, параметры `EDGE_*` вызывают специальную настройку порта для детектирования фронтов.


При работе с портом используется дескриптор порта:
```c
int fd =  gpio_port_open(gpiochip_id, name, offsets[], flags, attrs);
val = gpio_port_get(fd, mask);
gpio_port_set_reset(fd, mask_set, mask_reset);
gpio_port_close(fd);
```

## Модель работы с драйвером STREAM

Всякий драйвер устрйоства экспортирует интерфейс `STREAM`:
```c
void* (*open) (uint32_t Device_ID, int flag_id);
int   (*recv) (void* hdl, uint8_t *buffer, size_t max_len);
int   (*send) (void* hdl, uint8_t *buffer, size_t len );
int   (*ioctl)(void* hdl, int param_id, ...);
void  (*close)(void* hdl);
```
Прикладная программа может "открыть", получить доступ к устройству и ожидать флага готовности от устройства. flag_id - смещение в маске флагов. Всего устройство может выставить три флага: завершение отсылки, завершение приема и уведомление об обшибке. 
```c
osSignalWait(sigset_t signals, timeout_us);
```

пример такого устройства - QSPI flash
Есть некоторое различие и в адресации объектов. Файл на устройсвте (в дереве объектов) - объект типа `regular file` или `contguous file`. Который имеет соотвествующий тип `REG` (8) или `CONT` (9). Файлы непрерывные `contguous` плохо описаны и мы их не рассматриваем как отдельные сущности. 
Файл типа `REG` (8) открывается относительно `AT_FDCWD` - рабочей директории процесса. 
Файл типа `REG` наследует идентификатор устройства `Device_ID` которому сопоставлен драйвер устройства с интерфейсом STREAM. Интерфейс также содрежит `ADVISORY_INFO`, см. <sys/stdio.h>. `ADVISORY_INFO` включает информацию по размеру блока, выравниванию и минимальным/макисимальны трансферам. 

Таким образом, `File` будет содержать два идентификатора, свой идентификатор, который состоит из типа ресурса `rt` и уникального номера ресурса `ins`, и идентификатор драйвера `dev_id` с интерфейсом `STREAM`.

Драйвера устройств регистрируются в ядре операционной системы. И каждому присваивается уникальный порядковый номер `dev_id`, по которому можно получить ссылку на интерфейс устрйоства. Для разрешения следует использовать макрос `DEV_CLASS(dev_id)` или `FILE_CLASS(file)`.

```c
ssize_t _read	(FILE* fp, void *buf, size_t nbyte){
	const DeviceClass_t* dev_class = FILE_CLASS(fp);
	return dev_class->recv (fp, buf, nbyte);
}
```
Структура FILE должна содержать три идентификатора, см `Device_t` в <sys/stdio.h>:
```c
	ino_t  ino	 :22;	// уникальный идентификатор (номер) ресурса
	dev_t  dev_id:10; 	// идентификатор драйвера устройства
	mode_t mode	 :16;	// 4 старшие бита содержат класс ресурса
```
* В POSIX вместо указателя `FILE*` используется дескриптор файла `int fildes`. Использование дескрипторов оправдано, если операции с блочным устройством делегируются ядру ОС. 

```c
int   qspi_recv(void* hdl, uint8_t *buffer, size_t max_len){
	off_t offs = device->base + file->offset; // смещение на блочном устройстве

	osEvent event = osSignalWait(sigmask, timeout);
	// ожидать завершения трансфера или использовать aio -асинхронный ввод/вывод
}
```
В общем случае мы заполняем трансфер асинхронного ввода-вывода, см. <aio.h>
```c
	size_t 	aio_nbytes;	// Length of transfer.
	off_t 	aio_offset;	// File offset.
	volatile void *aio_buf;	// Location of buffer.
	struct sigevent aio_sigevent;	// Signal number and value.
```
трансфер будет включать `aio_sigevent` - номер флага события и тип запроса. Драйвер работает синхронно, подтверждая прием/отсылку одного пакета, или асинхронно с очередью из трансферов. После завершения трансфера выставляется флаг события или запускается тред (функция). 

*некоторые концепции POSIX выделил*

### 4.3 Directory Protection

If a directory is writable and the mode bit S_ISVTX is set on the directory, a process may remove
or rename files within that directory only if one or more of the following is true:

* The effective user ID of the process is the same as that of the owner ID of the file.
* The effective user ID of the process is the same as that of the owner ID of the directory.
* The process has appropriate privileges.

Optionally, the file is writable by the process. Whether or not files that are writable by the
process can be removed or renamed is implementation-defined.

### 4.6 File Hierarchy

Files in the system are organized in a hierarchical structure in which all of the non-terminal
nodes are directories and all of the terminal nodes are any other type of file. Since multiple
directory entries may refer to the same file, the hierarchy is properly described as a "directed
graph".


### 4.11 Measurement of Execution Time

The mechanism used to measure execution time shall be implementation-defined. The
implementation shall also define to whom the CPU time that is consumed by interrupt handlers
and system services on behalf of the operating system will be charged. See Section [3.118]

### 4.13 Pathname Resolution

Pathname resolution is performed for a process to resolve a pathname to a particular directory
entry for a file in the file hierarchy. There may be multiple pathnames that resolve to the same
directory entry, and multiple directory entries for the same file. When a process resolves a
pathname of an existing directory entry, the entire pathname shall be resolved as described
below. When a process resolves a pathname of a directory entry that is to be created immediately
after the pathname is resolved, pathname resolution terminates when all components of the path
prefix of the last component have been resolved. It is then the responsibility of the process to
create the final component.

### 4.16 Seconds Since the Epoch

A value that approximates the number of seconds that have elapsed since the Epoch. A
Coordinated Universal Time name (specified in terms of seconds (tm_sec), minutes (tm_min),
hours (tm_hour), days since January 1 of the year (tm_yday), and calendar year minus 1900
(tm_year)) is related to a time represented as seconds since the Epoch, according to the
expression below.
```c
tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
   (tm_year−70)*31536000  + ((tm_year−69)/4)*86400 −
  ((tm_year−1)/100)*86400 + ((tm_year+299)/400)*86400
```

### 4.17 Semaphore

A minimum synchronization primitive to serve as a basis for more complex synchronization
mechanisms to be defined by the application program.

For the semaphores associated with the Semaphores option, a semaphore is represented as a
shareable resource that has a non-negative integer value. When the value is zero, there is a
(possibly empty) set of threads awaiting the availability of the semaphore.

### Process Shared Memory and Synchronization

The existence of memory mapping functions in this volume of POSIX.1-2017 leads to the
possibility that an application may allocate the synchronization objects from this section in
memory that is accessed by multiple processes (and therefore, by threads of multiple processes).

If an implementation supports the _POSIX_THREAD_PROCESS_SHARED option, then the
process-shared attribute can be used to indicate that mutexes or condition variables may be
accessed by threads of multiple processes.

```c
fd = open(mutex_path, O_RDWR | O_CREAT | O_EXCL, 0666);
(void) ftruncate(fd, sizeof(pthread_mutex_t));
(void) pthread_mutexattr_init(&attr);
(void) pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
mtx = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t),
	PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
close (fd);
(void) pthread_mutex_init(mtx, &attr);
```
Пример из стандарта. Я не думаю что пример не вполне корректный. close( ) освобождает дескриптор, 
и завершает трансфер данных на носитель. 
Вероятно считается, что трансфер не завершится пока не закорется карта памяти вызвовом munmap().
Другая проблема - не факт, что в другом процессе откроется та же копия объекта (файла).

Более корректный вариант использования - Shared Memory Object, он заведомо создается в общей (разделяемой) памяти
без копирования на носитель.
```c
fd = shm_open(mutex_path, O_RDWR|O_CREAT, 0660);
(void) ftruncate(fd, sizeof(pthread_mutex_t));
(void) pthread_mutexattr_init(&attr);
(void) pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
mtx = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t),
	PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
(void) pthread_mutex_init(mtx, &attr);
. . .
munmap(mtx, sizeof(pthread_mutex_t));
close(fd);
snm_unlink(mutex_path);
```
Пример содержит инициализацию, объект в памяти создается при отсуствии. В нашей реализации установка атрибута 
PTHREAD_PROCESS_SHARED не влияет на работоспособность. Функции установки атрибутов - пустышки, 
сохраняются ради переносимости кода. 

Вариант использования Shared объекта в другом процессе не требует инициализации.
```c
fd = open(mutex_path, O_RDWR, 0660);
mtx = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t),
	PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
. . .
munmap(mtx, sizeof(pthread_mutex_t));
close(fd);
```
Для совместимости со стандартом следует увеличивать число ссылок на объект при обращении к mmap()
и уменьшать при обращении к munmap().

### Создание дерева устройств из архива и балансировка дерева устройств

Устройства рождаютяся из архива (из описания дерева). При последовательном добавлении 
в бинарное дерево элементов с ключами вида {dev, ino} при последовательной нумерации 
устройств создается линейная структура типа список, поскольку при сравнении ключей 
добавляемый объект всегда попадает в ветку "больше". 
Мы хотим избежать необходимости балансировки дерева в процессе работы. 
Чтобы обеспечить равномерное добавление в бинарное дерево новых узлов при 
последовательной нумерации ключей (директорий, в частности), 
можно использовать отраженный порядок следования бит в ключе. Операция отражения бит поддерживается 
инструкцией __RBIT, Arm. Мы предполагаем специальный метод кодирования для внешнего представления 
идентификаторов {dev, ino}, который позволяет хранить идентификаторы в любом удобном формате.
Балансировка дерева должна выполняться при сохранении или дописывании архива.
При использовании отраженных ключей дописывание дерева при последовательной нумераци ведет к равномерной балансировке. 

### Совместимость с GLib

Есть несколько библиотек по которым можно обеспечить переносимость ПО. 
Стандартные объекты типа односвязные GSList <r3_slist.h> и двусвязные списки GList, дата-листы, N-арные деревья, 
кварки GQuark мы реализуем в дистрибутиве ОС с интерфейсом glib.h. Мы поддерживаем выделение данных слайсами 
GSlice <r3_slice.h>. 
Для утилит командной строки мы поддерживаем GОptions <r3_args.h>.

### Наследовние классов или инкапсуляция системных объектов

Поле dev_id в структуре системного объекта (Device_t) отвечает за идентификацию класса устройства.
Для получения вложенного объекта из ссылки (Device_t\* dev) следует использовать запись
```c
obj = (AnyObject_t *)(dev +1);
```
Для доступа к полям Device_t по адресу вложенного объекта
```c
dev = (Device_t*)obj -1;
```
Подобный способ используется при инкапсуляции, приписывании шапки вложенному пакету.
Такое обращение используется при вызове методов класса драйвера системного устройства.

Каждый системный объект имеет поле mode, 12 бит.

MemBlock_t -- блоки динамической памяти памяти malloc()/free()
Mapped_t -- объекты, с которыми работает функция mmap()/munmap()
OpenFileDescription
Shared Memory Object - это MemBlock_t, выделенный в Shared памяти.
Typed Memory Object - содержит фалги, адрес сегмента и длину сегмента. 

Функция fmemopen открывает любой блок MemBlock_t памяти
Для этого создается OFD, со ссылкой на данные MemBlock_t.

### Дескрипторы устройств

Дескрипторы - идентификатор устройства, 
ОписаниеОткрытогоФайла - содержит ссылку на объект типа Файл и позицию чтения/записи. 
Для других системных объектов, которые не требуют поддержки сессии, по дескриптору 
возвращается указатеь на системный объект. 
FILE реализован как идентификатор -- ссылается на элемент пула OFD.

Дескрипторы выделяются динамически из пула при открытии/закрытии сессии работы с устройством.
Дескрипторы не связаны с флагами событий.
Флаги событий в диапазоне {`SIGRTMIN`,`SIGRTMAX`} выделяются динамически. 

stream - связан с понятием FILE, имеет представление в виде OFD
STREAM - свойство системного объекта, очередь входящих запросов, запросы в форме транзакций. 
Socket - системный сокет, типа STREAM, имеет входящую очередь запросов с приоритетами. 
MQueue - очередь сообщений c приоритетами
FIFO - файл со специальным свойством - он зациклен. 
sem_t - Семафор. Содержит единственное поле -- счетчик семафора. Семафор может быть выделен 
в специальной памяти предназначенной для межпроцессорного взаимодействия Process-Shared. 



### Виртуальная сеть как основа планирования заданий

основа - привязка к виртуальному интерфейсу, который характерируетяся пропускной способностью. 





