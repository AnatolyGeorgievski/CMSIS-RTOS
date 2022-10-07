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
```
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

```
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
```
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
```
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
```
obj = (AnyObject_t *)(dev +1);
```
Для доступа к полям Device_t по адресу вложенного объекта
```
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
Флаги событий в диапазоне {SIGRTMIN,SIGRTMAX} выделяются динамически. 

stream - связан с понятием FILE, имеет представление в виде OFD
STREAM - свойство системного объекта, очередь входящих запросов, запросы в форме транзакций. 
Socket - системный сокет, типа STREAM, имеет входящую очередь запросов с приоритетами. 
MQueue - очередь сообщений c приоритетами
FIFO - файл со специальным свойством - он зациклен. 
sem_t - Семафор. Содержит единственное поле -- счетчик семафора. Семафор может быть выделен 
в специальной памяти предназначенной для межпроцессорного взаимодействия Process-Shared. 



### Виртуальная сеть как основа планирования заданий

основа - привязка к виртуальному интерфейсу, который характерируетяся пропускной способностью. 





