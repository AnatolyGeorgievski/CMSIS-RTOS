# Операционная система R3v2 CMSIS-RTOS

Ядро операционной системы R3v2. Предназначена для работы на контроллерах ARM Cortex-M. 
Операционная система поддерживает мультизадачность, интерфейс CMSIS-RTOS, -RTOSv2 и POSIX. Поддерживает совместимость со стандартом С11: thread, mutex, condition variables, thread specific storage... Интерфейс C11 thread.h является нативным. Системные интерфейсы POSIX в разработке.

## Характеристики

- Многозадачность и примитивы синхронизации выполнены таким образом, чтобы обспечить неблокирующее выполнение всех процессов. Lock-Free, Wait-free.
- Переключение (планирование) задач происходит в прерывании PendSV. Прерывание PendSV выполняется на хвосте любого прерывания. 
- Поддержка таймерных задач с компенсацией округления временных интервалов.
- Поддержка фоновых процессов (служб) типа сбор мусора при динамическом распределении памяти или фоновая обработка потоков сообщений. 
- Нативная поддержка очереди процессов наподобие OpenCL. Поддержка очереди позволяет реализовать модели асинхронного исполнения по событиям. При этом задачи (кернелы) не занимают стек. 
- Неблокирующие асинхронные очереди, динамическое распределение памяти malloc не блокирующее,  выделение памяти слайсами не блокирующее, без ожидания, без задержек. 
- поддержка протокола системного уровня для удаленного управления и сбора данных. 
- поддержка системного реестра, объектная модель данных совместимая с BACnet. 
- поддержка бинарных форматов BACnet
- Отладка ядра и прикладных задач выполняется через подсистему Trace. На контроллерах Cortex-M поддерживается вывод отладочной информации через ITM SWO или UART.

## Введение
Интерфейс CMSIS-RTOS поддерживается, потому что стандарт. Надо отметить, что далеко не все примитивы синхронизации задач присутствующие в CMSIS задействованы в реальных задачах. На практике достаточным оказывается использоваие всего трех элементов: сигналы или флаги процессов, семафоры, асинхронные очереди. Практически, для реализации мультизадачности необходимо, чтобы ядро поддерживало ожидание семафора и ожидание сигналов. Все варианты ожидания ресурсов или очередей можно выстроить на семафорах, понимая под семафором - счетчик свободных рессурсов. Таким образом минимальный вариант планировщика обеспечивает ожидание флагов и семафоров. Флаги процесса являются неотъемлемой частью контекста процесса. 

Процесс - это объект у которого есть: флаги, таймер ожидания, функция обработки потока. В R3 поодерживается нексолько концепций управления процессами. Процессы бывают "служебные", такие как сбор мусора или вывод системных сообщений. Такие процессы мы называем "служба", они не используют отдельный стек, выполяются в составе системного треда. Еще один вариант процессов - кернелы - элементы блочной диаграммы. В R3 поддерживается планировщик для фоновых задач с возможностью синхронизации запуска кернелов (функциональных блоков) по готовности данных и готовности исполнения, поддерживается конвейер задач подобно OpenCL. 

Ценный ресурс для контроллеров - оперативная память. По этой причине мы стараемся минимизировать использование памяти под стек. Стеки могут быть разделяемые между множеством типовых задач. Мы поддерживаем концепцию ThreadPool на системном уровне. Это означает возможность запуска неограниченного числа задач на фиксированном количестве тайм слотов (стеков).

Таймерные задачи. ОС обладает уникальным свойством. Мы поддерживаем интерфес запуска периодидических таймерных процессов с возможностью компенсации фазовой ошибки времени запуска. Таймерных процессов может быть несколько и каждый из них не препятствует и не замедляет работу другого. Эта функция в частности используется для реализации управления двигателями или для цифровой обработки сигналов. 

В остальном мы придерживаемся совместимости со стандартом C11, который предлагает свой вариант тредов. Треды C11 оказываются базовым интерфейсом на котором строится интерфейс тредов CMSIS-RTOS. Мы широко используем атомарные операции для реализации механизмов синхронизации процессов, таких как мьютексы и семафоры, в user space, без передачи управления операционной системе. Это позволяет реализовать принцип Lock-Free, Wait-free, синхронизацию задач без задержек и без блокировок. 

Мы нацелены на переносимость программного кода, для этого в составе ОС реализуется функционал POSIX и POSIX threads. Поддержка POSIX структурирована по функциональным группам: Timers, Basic Threads, Semaphores, SpinLocks, Realtime Signals, Device IO.. . [см. Подробно](UNISTD.md)

## Интерфейсы С1x
Поддержка системных интерфейсов Си
* threads.h... функции управления процессами, threads
* mutex и condition variables.
* tss.h... thread specific storage
* stdio.h
* stdlib.h

## Интерфейс CMSIS-RTOS 
Функционал операционной системы определяется набором определений указанных в файле заголовка "[cmsis_os.h](cmsis_os.h)". В системе R3 поддержан весь функционал CMSIS-RTOS.

## Интерфейс CMSIS-RTOS v2
Функционал поддерживается выборочно, поскольку явной потребности перехода не наблюдается. Основой является интерфейсы C1x с раширением POSIX. 
* osMemoryPool
* osThreadFlags
* osMessageQueue

## Интерфейсы POSIX
Реализована обширная поддрежка стандарта POSIX.1-2017. В частности, поддерживаются POSIX Basic Threads(PTHREAD), Timers, Semaphores(SEM), SpinLocks, Reader Writer Locks, Realtime Signals, Message Passing (MSG), Shared Memory Objects (SHM), Device I/O, Memory Mapped Files, ... 
[См. подробное описание...](UNISTD.md)

## Системные службы
Всегда в системе есть выделенный процесс системный/фоновый. В фоновом процессе последовательно запускаются "Службы". Службы не занимают стек. Службы используют те же самые механизмы синхрнизации и те же самые механизмы ожидания событий, но только ожидание происходит не внутри функции. Службы по сути реализуют кооперативную многозадачность, в рамках одного выделенного системного процесса.

## Кооперативная и вытесняющая многозадачность
Если задача выполняет дольше заданного кванта времени, происходит переключение задач. Квант времени системного таймера настраивается. При использовании не блокирующих вызовов, а блокирующие вызовы в системе не поддерживаются, ситуация вытестения, когда та или иная задача выполняется достаточно продолжительное время, чтобы произошло принудительное переключение задач, не реализуется. Таким образом, реализуется механиз кооперативной многозадачности. Мы поддерживаем уведомления - это механизм переключения задач при возникновении события в драйвере периферийного устройства, в прерывании, который продвигает процесс c более высоким приоритетом в очереди переключения задач. Т.е. в результате уведомления сразу, без задержки, на хвосте аппаратного прерывания может производится переключение контекстов задач. Управление передается задаче, за которой закреплено взаимодействие с драйвером. 

Такое переключение выполняется с учетом приоритета задачи и обеспечивает минимальную задержку на обработку. 

## Таймерные задачи
В операционной системе поддерживается механизм запуска таймерных задач по системному таймеру, с точностью до микросекунды. Каждая такая задача запускается со своей фазой по времени. Точность определяется квантом времени системного таймера, стандартный квант может настраиваться и уменьшаться до 10мкс. Таймерные задачи выполняются периодически с компенсаицей ошибки окргуления времени, что обеспечивает возможность одновременной работы нескольких таймерных задач с минимальной фазовой ошибкой (Джиттером). Эта функция используется для управления моторами и для цифровой обработки сигналов. 

## Система сбора сообщений Trace
Сбор сообщений нужен для отладки. Вывод сообщений возможен сразу в несколько устройств, например через DAP SWO или UART. Отладка ядра, как правило, происходит с использованием функций puts, printf для тредов и функций snprintf + debug для прерываний ядра. Вывод сообщений с использованием printf, не блокирующий происходит в фоновом режиме. Вывод сообщений debug - блокирующий. 

## Менеджер памяти
Система поддерживает несколько механизмов распределения памяти. Выделение памяти из кучи: malloc/free. Выделение памяти из массива -- memory pool. Выделение памяти слайсами. Механиз выделения памяти слайсами совместим с Glib, увеличивает количество слайсов . Отличительной чертой реализации является отсутсвие задержек, отсутвие циклов обработки, и отсуствие блокировок при выделении и освобождении динамической памяти. Реализация выполнена на атомарных операциях. 

## Вместо файловой системы
Мы старательно избегаем использовать файловую систему для работы с данными. Данные должны рождаться в памяти и прикладная программа ничего не должна знать про способ хранения данных. С точки зрения прикладной программы данные всегда расположены в памяти контроллера. Использование файловой системы ведет к копированию данных. Заменой файловой системе явяляется системный реестр, часть которого сохраняется при выключении питания. Концепция реестра совместима с адресацией объектов BACnet. Поддерживается механизм конфигурации устройства основанный на перемещаемых сегментах памяти. Переменная в программе может быть помечена, как часть статической конфигурации. Сегмент статической конфигурации формируется на этапе загрузки и линковки кода. Загрузка конфигурации происходит при включении питания, поддерживается журнал операции сохранения конфигурации при аварийном выключении. 

## Файловая система (RFS)
. . .

## Системное устройство (Драйвер)
open, close, read, write, send, recv --использование данного интерфейса предполагает эксклюзивный доступ к устройству(ресурсу). Драйвер использует понятие флаг, ассоциированный с устройством и процессом-владелецем ресурса (owner). Все вызовы не блокирующие. Ожидание одного и более событий реализуется на сигналах/флагах процесса. 

## Портирование
Изначально ориентировано на использование микроконтроллеров с архитектурой ARM Cortex M3/M4/M7, M23/M33/M35P/M55/M85. Поддерживаются профили armv7-m, armv8-BASE и armv8-MAIN. Портировать можно на любую архитектуру, где присутсвтует аппаратная поддержка неблокирующих атомарных операций (операции эксклюзивного доступа). 

## Развитие операционной системы
* Перенос функционала на много-ядерные платформы. 

Copyright (c) 2008-2022. Anatoly Georgivskii
