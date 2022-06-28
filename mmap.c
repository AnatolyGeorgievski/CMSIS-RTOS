#include <unistd.h>
//long sysconf(int name); // значение системной переменной по идентификатору _SC_PAGESIZE

#include <sys/mman.h>
/*! \brief преобразует идентификатор объекта в указатель в памяти
    \param len длина сегмента данных
    \param fildes идентификатор объекта
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
    return addr;
}
/*!

The munmap() function shall remove any mappings for those entire pages containing
any part of the address space of the process starting at addr and continuing for len bytes.
Further references to these pages shall result in the generation of a SIGSEGV signal to the process.
If there are no mappings in the specified address range, then munmap() has no effect.

The implementation may require that addr be a multiple of the page size as returned by sysconf().
 */
int munmap(void *addr, size_t len);
