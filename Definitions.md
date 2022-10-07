## POSIX: Definitions ##
Выдержки из раздела стандарта POSIX.1-2017: [POSIX.1-2017:Definitions]

[POSIX.1-2017:Definitions]: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html "POSIX.1-2017: Definitions"


[3.2] Absolute Pathname

A pathname beginning with a single or more than two <slash> characters

[3.25] Asynchronous Events

Events that occur independently of the execution of the application.

[3.26] Asynchronous Input and Output

A functionality enhancement to allow an application process to queue data input and output
commands with asynchronous notification of completion.

[3.27] Async-Signal-Safe Function

A function that can be called, without restriction, from signal-catching functions. Note that,
although there is no restriction on the calls themselves, for certain functions there are restrictions
on subsequent behavior after the function is called from a signal-catching function. No function
is async-signal-safe unless explicitly described as such.

[3.28] Asynchronously-Generated Signal

A signal that is not attributable to a specific thread. Examples are signals sent via kill( ), signals
sent from the keyboard, and signals delivered to process groups. Being asynchronous is a
property of how the signal was generated and not a property of the signal number. All signals
may be generated asynchronously

[3.30] Asynchronous I/O Operation

An I/O operation that does not of itself cause the thread requesting the I/O to be blocked from
further use of the processor.

This implies that the process and the I/O operation may be running concurrently.

[3.31] Authentication

The process of validating a user or process to verify that the user or process is not a counterfeit.

[3.32] Authorization

The process of verifying that a user or process has permission to use a resource in the manner
requested.

[3.39] Barrier

A synchronization object that allows multiple threads to synchronize at a particular point in
their execution.

[3.40] Basename

For pathnames containing at least one filename: the final, or only, filename in the pathname. For
pathnames consisting only of <slash> characters: either '/' or "//" if the pathname consists of
exactly two <slash> characters, and '/' otherwise.

[3.76] Blocked Process (or Thread)

A process (or thread) that is waiting for some condition (other than the availability of a
processor) to be satisfied before it can continue execution.

[3.77] Blocking

A property of an open file description that causes function calls associated with it to wait for the
requested action to be performed before returning.

[3.79] Block Special File

A file that refers to a device. A block special file is normally distinguished from a character
special file by providing access to the device in a manner such that the hardware characteristics
of the device are not visible.

[3.91] Character Special File

A file that refers to a device (such as a terminal device file) or that has special properties (such as
/dev/null )

[3.107] Condition Variable

A synchronization object which allows a thread to suspend execution, repeatedly, until some
associated predicate becomes true. A thread whose execution is suspended on a condition
variable is said to be blocked on the condition variable.


[3.118] CPU Time (Execution Time)

The time spent executing a process or thread, including the time spent executing system services
on behalf of that process or thread. If the Threads option is supported, then the value of the
CPU-time clock for a process is implementation-defined. With this definition the sum of all the
execution times of all the threads in a process might not equal the process execution time, even
in a single-threaded process, because implementations may differ in how they account for time
during context switches or for other reasons.

[3.119] CPU-Time Clock

A clock that measures the execution time of a particular process or thread.

[3.126] Deferred Batch Service

A service that is performed as a result of events that are asynchronous with respect to requests.
	
[3.127] Device

A computer peripheral or an object that appears to the application as such.

[3.128] Device ID

A non-negative integer used to identify a device.

[3.129] Directory

A file that contains directory entries. No two directory entries in the same directory have the
same name.

[3.130] Directory Entry (or Link)

An object that associates a filename with a file. Several directory entries can associate names
with the same file. \see <dirent.h>

[3.131] Directory Stream

A sequence of all the directory entries in a particular directory. An open directory stream may be
implemented using a file descriptor.

[3.144] Empty Directory

A directory that contains, at most, directory entries for _dot_ and _dot-dot_, and has exactly one link
to it (other than its own _dot_ entry, if one exists), in _dot-dot_. No other links to the directory may
exist. It is unspecified whether an implementation can ever consider the root directory to be
empty.

[3.150] Epoch

The time zero hours, zero minutes, zero seconds, on January 1, 1970 Coordinated Universal Time (UTC).

[3.161] Feature Test Macro

A macro used to determine whether a particular set of features is included from a header. \see <unistd.h>

[3.163] FIFO Special File (or FIFO)

A type of file with the property that data written to such a file is read on a first-in-first-out basis.

#### [3.164] File {#_3_164}

An object that can be written to, or read from, or both. A file has certain attributes, including
access permissions and type. File types include regular file, character special file, block special
file, FIFO special file, symbolic link, socket, and directory. Other types of files may be supported
by the implementation.

#### [3.166] File Descriptor

A per-process unique, non-negative integer used to identify an open file for the purpose of file
access. The value of a newly-created file descriptor is from zero to {OPEN_MAX}−1. A file
descriptor can have a value greater than or equal to {OPEN_MAX} if the value of {OPEN_MAX}
has decreased (see sysconf( )) since the file descriptor was opened. File descriptors may also be
used to implement message catalog descriptors and directory streams; see also Section [3.258](#_3_258).

[3.167] File Group Class

The property of a file indicating access permissions for a process related to the group
identification of a process. A process is in the file group class of a file if the process is not in the
file owner class and if the effective group ID or one of the supplementary group IDs of the
process matches the group ID associated with the file. Other members of the class may be
implementation-defined.

[3.168] File Mode

An object containing the file mode bits and some information about the file type of a file.

[3.169] File Mode Bits

A file’s file permission bits, set-user-ID-on-execution bit (S_ISUID), set-group-ID-on-execution
bit (S_ISGID), and, on directories, the restricted deletion flag bit (S_ISVTX).

[3.170] Filename

A sequence of bytes consisting of 1 to {NAME_MAX} bytes used to name a file. The bytes
composing the name shall not contain the <NUL> or <slash> characters. In the context of a
pathname, each filename shall be followed by a <slash> or a <NUL> character; elsewhere, a
filename followed by a <NUL> character forms a string (but not necessarily a character string).
The filenames _dot_ and _dot-dot_ have special meaning. A filename is sometimes referred to as a
"pathname component". 

[3.172] File Offset

The byte position in the file where the next I/O operation begins. Each open file description
associated with a regular file, block special file, or directory has a file offset. A character special
file that does not refer to a terminal device may have a file offset. There is no file offset specified
for a pipe or FIFO.

[3.192] Hard Link

The relationship between two directory entries that represent the same file; see also Section [3.130]
(on page 53). The result of an execution of the ln utility (without the −s option) or the link( )
function. This term is contrasted against symbolic link

[3.228] Monotonic Clock

A clock measuring real time, whose value cannot be set via clock_settime( ) and which cannot
have negative clock jumps.

[3.229] Mount Point

Either the system root directory or a directory for which the st_dev field of structure stat differs
from that of its parent directory.

[3.233] Multi-Threaded Program

A program whose executable file was produced by compiling with c99 using the flags output by
getconf POSIX_V7_THREADS_CFLAGS, and linking with c99 using the flags output by getconf
POSIX_V7_THREADS_LDFLAGS and the −l pthread option, or by compiling and linking using
a non-standard utility with equivalent flags. Execution of a multi-threaded program initially
creates a single-threaded process; the process can create additional threads using pthread_create( )
or SIGEV_THREAD notifications.

[3.234] Mutex

A synchronization object used to allow multiple threads to serialize their access to shared data.
The name derives from the capability it provides; namely, mutual-exclusion. The thread that has
locked a mutex becomes its owner and remains the owner until that same thread unlocks the
mutex.

[3.244] Nice Value

A number used as advice to the system to alter process scheduling. Numerically smaller values
give a process additional preference when scheduling a process to run. Numerically larger
values reduce the preference and make a process less likely to run. Typically, a process with a
smaller nice value runs to completion more quickly than an equivalent process with a higher
nice value. The symbol {NZERO} specifies the default nice value of the system.

[3.245] Non-Blocking

A property of an open file description that causes function calls involving it to return without
delay when it is detected that the requested action associated with the function call cannot be
completed without unknown delay.

[3.257] Open File

A file that is currently associated with a file descriptor.

[3.258] Open File Description

A record of how a process or group of processes is accessing a file. Each file descriptor refers to
exactly one open file description, but an open file description can be referred to by more than
one file descriptor. The file offset, file status, and file access modes are attributes of an open file
description.

[3.271] Pathname

A string that is used to identify a file. In the context of POSIX.1-2017, a pathname may be limited
to {PATH_MAX} bytes, including the terminating null byte. It has optional beginning <slash>
characters, followed by zero or more filenames separated by <slash> characters. A pathname
can optionally contain one or more trailing <slash> characters. Multiple successive <slash>
characters are considered to be the same as one <slash>, except for the case of exactly two
leading <slash> characters.

[3.273] Path Prefix

The part of a pathname up to, but not including, the last component and any trailing <slash>
characters, unless the pathname consists entirely of <slash> characters, in which case the path
prefix is '/' for a pathname containing either a single <slash> or three or more <slash>
characters, and '//' for the pathname //. The path prefix of a pathname containing no <slash>
characters is empty, but is treated as referring to the current working directory.

[3.278] Pipe

An object identical to a FIFO which has no links in the file hierarchy

[3.279] Polling

A scheduling scheme whereby the local process periodically checks until the pre-specified
events (for example, read, write) have occurred.

[3.282] Portable Filename Character Set

The set of characters from which portable filenames are constructed.
A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
a b c d e f g h i j k l m n o p q r s t u v w x y z
0 1 2 3 4 5 6 7 8 9 . _ -

#### [3.285] Preempted Process (or Thread) {#_3_285}

A running thread whose execution is suspended due to another thread becoming runnable at a
higher priority.

[3.289] Priority

A non-negative integer associated with processes or threads whose value is constrained to a
range defined by the applicable scheduling policy. Numerically higher values represent higher
priorities.

[3.291] Priority Inversion

A condition in which a thread that is not voluntarily suspended (waiting for an event or time
delay) is not running while a lower priority thread is running. Such blocking of the higher
priority thread is often caused by contention for a shared resource.

[3.292] Priority Scheduling

A performance and determinism improvement facility to allow applications to determine the
order in which threads that are ready to run are granted access to processor resources.

[3.293] Priority-Based Scheduling
	Scheduling in which the selection of a running thread is determined by the priorities of the
runnable processes or threads.

[3.310] Read-Only File System
	A file system that has implementation-defined characteristics restricting modifications.

[3.311] Read-Write Lock
	Multiple readers, single writer (read-write) locks allow many threads to have simultaneous
read-only access to data while allowing only one thread to have write access at any given time.
They are typically used to protect data that is read-only more frequently than it is changed.
	Read-write locks can be used to synchronize threads in the current process and other processes if
they are allocated in memory that is writable and shared among the cooperating processes and
have been initialized for this behavior.

[3.313] Real Time
	Time measured as total units elapsed by the system clock without regard to which thread is
executing.

[3.314] Realtime Signal Extension
	A determinism improvement facility to enable asynchronous signal notifications to an
application to be queued without impacting compatibility with the existing signal functions.

[3.324] Relative Pathname
	A pathname not beginning with a <slash> character.
	
[3.325] Relocatable File
	A file holding code or data suitable for linking with other object files to create an executable or a
shared object file.

[3.342] Semaphore
	A minimum synchronization primitive to serve as a basis for more complex synchronization
mechanisms to be defined by the application program.

[3.346] Shared Memory Object
	An object that represents memory that can be mapped concurrently into the address space of
more than one process.

[3.350] Signal
	A mechanism by which a process or thread may be notified of, or affected by, an event occurring
in the system. Examples of such events include hardware exceptions and specific actions by
processes. The term signal is also used to refer to the event itself.

[3.351] Signal Stack
	Memory established for a thread, in which signal handlers catching signals sent to that thread
are executed.

[3.361] Spawn
	A process creation primitive useful for systems that have difficulty with fork( ) and as an efficient
replacement for fork( )/exec.

[3.364] Spin Lock
	A synchronization object used to allow multiple threads to serialize their access to shared data.

[3.365] Sporadic Server
	A scheduling policy for threads and processes that reserves a certain amount of execution
capacity for processing aperiodic events at a given priority level.

[3.370] Stream
	Appearing in lowercase, a stream is a file access object that allows access to an ordered sequence
of characters, as described by the ISO C standard. Such objects can be created by the fdopen( ),
fmemopen( ), fopen( ), open_memstream( ), or popen( ) functions, and are associated with a file
descriptor. 

[3.371] STREAM
Appearing in uppercase, STREAM refers to a full-duplex connection between a process and an
open device or pseudo-device. It optionally includes one or more intermediate processing
modules that are interposed between the process end of the STREAM and the device driver (or
pseudo-device driver) end of the STREAM; see also Section [3.370].

[3.381] Symbolic Link
	A type of file with the property that when the file is encountered during pathname resolution, a
string stored by the file is used to modify the pathname resolution. The stored string has a
length of {SYMLINK_MAX} bytes or fewer.

[3.382] Synchronized Input and Output
	A determinism and robustness improvement mechanism to enhance the data input and output
mechanisms, so that an application can ensure that the data being manipulated is physically
present on secondary mass storage devices.

[3.389] System

An implementation of POSIX.1-2017.
	
[3.394] System Databases

An implementation provides two system databases: the "group database" (see also Section
[3.188]) and the "user database" (see also Section [3.435]).

[3.404] Thread

A single flow of control within a process. Each thread has its own thread ID, scheduling priority
and policy, errno value, floating point environment, thread-specific key/value bindings, and the
required system resources to support a flow of control. Anything whose address may be
determined by a thread, including but not limited to static variables, storage obtained via
malloc( ), directly addressable storage obtained through implementation-defined functions, and
automatic variables, are accessible to all threads in the same process.

[3.407] Thread-Safe

A thread-safe function can be safely invoked concurrently with other calls to the same function,
or with calls to any other thread-safe functions, by multiple threads. Each function defined in
the System Interfaces volume of POSIX.1-2017 is thread-safe unless explicitly stated otherwise.
Examples are any ``pure’’ function, a function which holds a mutex locked while it is accessing
static storage, or objects shared among threads.

[3.408] Thread-Specific Data Key

A process global handle of type pthread_key_t which is used for naming thread-specific data.
Although the same key value may be used by different threads, the values bound to the key by
pthread_setspecific( ) and accessed by pthread_getspecific( ) are maintained on a per-thread basis
and persist for the life of the calling thread.

[3.410] Timeouts

A method of limiting the length of time an interface will block; see also Section [3.76]

[3.411] Timer

A mechanism that can notify a thread when the time as measured by a particular clock has
reached or passed a specified value, or when a specified amount of time has passed.

[3.412] Timer Overrun

A condition that occurs each time a timer, for which there is already an expiration signal queued
to the process, expires.

[3.428] Typed Memory Name Space

A system-wide name space that contains the names of the typed memory objects present in the
system. It is configurable for a given implementation.

[3.429] Typed Memory Object

A combination of a typed memory pool and a typed memory port. The entire contents of the
pool are accessible from the port. The typed memory object is identified through a name that
belongs to the typed memory name space.

[3.430] Typed Memory Pool

An extent of memory with the same operational characteristics. Typed memory pools may be
contained within each other.

[3.447] Working Directory (or Current Working Directory)

A directory, associated with a process, that is used in pathname resolution for pathnames that do
not begin with a <slash> character.

[3.452] Zombie Process

The remains of a live process (see Section [3.210]) after it terminates (see Section [3.303]) 
and before its status information is consumed by its parent process.