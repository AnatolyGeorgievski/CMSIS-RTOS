/*! POSIX_DEVICE_IO: Device Input and Output 

	Copyright (C) 2022 Anatoly Georgievskii <Anatoly.Georgievski@gmail.com>
	
	Реализация функций POSIX_DEVICE_IO для R3v2 RTOS
*/
#include <unistd.h>
#include <atomic.h>
#include <fcntl.h>
#include <r3_slice.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stdio.h>
#include <sys/stat.h>

typedef struct _Device Device_t;
typedef struct _DeviceClass DeviceClass_t;
extern const DeviceClass_t *dev_classes[];

static uint32_t device_flags[1]= {0};// per thread?
Device_t* _devices[32];// per thread? per pid_t?


extern Device_t *dentry_cwd;
extern mode_t _umask;//0777;

static inline void atomic_flag_free (uint32_t *flags, int fd){
	atomic_fetch_and(&flags[fd>>5], ~(1UL<<(fd%32)));
}
static inline int  atomic_flag_alloc(uint32_t *flags){
	int fildes = 0;
	uint32_t map;
	volatile int *ptr = (volatile int *)flags;
	do {
		map = atomic_int_get(ptr);
		if (map==~0U) return -1;
		fildes = __builtin_ctz(~map);
	} while(!atomic_int_compare_and_exchange(ptr, map, map | (1U<<fildes)));
	return fildes;
}
// TODO избавиться от аргумента 
int  device_flag_alloc(Device_t* data){
	int fildes = atomic_flag_alloc(device_flags);
	if (fildes>=0)
		DEV_PTR(fildes) = data;
	return fildes;
}
void device_flag_free(int fildes){
	DEV_PTR(fildes) = NULL;
	atomic_flag_free(device_flags, fildes);
}

#if defined(_POSIX_DEVICE_IO) && (_POSIX_DEVICE_IO>0)
/*! \defgroup POSIX_DEVICE_IO POSIX: Device Input and Output
	\ingroup _posix

	\see 2.5.1 Interaction of File Descriptors and Standard I/O Streams

Терминология
Open File Description -- некоторый объект, содержащий ссылку на объект типа файл и позицию чтения/записи fpos_t.
File Descriptor -- целое число, которое ассоциировано с объектом OFD
Есть несколько специальных значений FD, это числа:
(-1) -- дескриптор не выделен, (-2) (AT_FDCWD) -- значение по умолчанию, указывает на рабочую директорию. 
Дескрипторы 0,1,2 закреплены за консолью - эту традицию мы не поддерживаем, 
поскольку исключаем вмешательство человека.

В нашей	 реализации используется понятие "устройство", Device - это самое общее представление системного объекта.
Структура _Device записывается, как шляпа любого системного объекта. 
В системе регистрируются типы всех системных объектов.

* dev_t -- идентификатор класса системного объекта
* ino_t -- идентификатор объекта, вместе с dev_t образуют уникальный идентификатор объекта.
* uid_t -- мы не планируем размножать пользователей, людей. Но мы планируем размножать Ботов, с различным уровнем доступа
* gid_t -- идентификатор группы безопасности. 
* mode  -- права доступа 9 бит. RWX. 
* nlink -- число ссылок на объект, когда число ссылк снижается мньше нуля, объект удаляется из системы.

Все системные объекты хранятся в системной директории. dtree_*
Объекты могут быть Process Shared, если создаются в директории. Таким образом создаются семафоры sem_open(), 
очереди mq_open(), Shared Mempry Objects shm_open() и FIFO mkfifo(). 
Process Shared означает разрешение адреса для других процессов испольняемых на том же кластере CPU. 
Чтобы любой пользовательский объект стал системным его нужно создать особым образом с использованием
функции dtree_mknodat(). Предварительно необходимо определить идентификатор и класс системного устройства.

Для ассиметричных мультиядрерных процессоров (AMP) мы предполагаем использовать интерфейс типа RPMsg 
и аппаратные семафоры.

	\{ 
FD_CLR( ), FD_ISSET( ), FD_SET( ), FD_ZERO( ), clearerr( ), close( ), fclose( ), fdopen( ), feof( ),
ferror( ), fflush( ), fgetc( ), fgets( ), fileno( ), fopen( ), fprintf( ), fputc( ), fputs( ), fread( ), freopen( ),
fscanf( ), fwrite( ), getc( ), getchar( ), gets( ), open( ), perror( ), poll( ), printf( ), pread( ), pselect( ),
putc( ), putchar( ), puts( ), pwrite( ), read( ), scanf( ), select( ), setbuf( ), setvbuf( ), stderr, stdin,
stdout, ungetc( ), vfprintf( ), vfscanf( ), vprintf( ), vscanf( ), write( ) 
 */
int openat(int fd, const char *path, int oflag, ...)
{
	int fildes = atomic_flag_alloc(device_flags);
	if (fildes<0) return fildes;
	Device_t* dev = DEV_PTR(fd);
	dev = dtree_path(dev, path, &path);
	
	return fildes;
}
int open (const char *path, int oflags, ...){
	int fildes = atomic_flag_alloc(device_flags);
	if (fildes<0) return fildes;
	Device_t* dev = NULL;// = DEV_PTR(fd);
	dev = dtree_path(dev, path, &path);
	if (dev==NULL) return -1;
	if (path!=NULL && (oflags & O_CREAT)) {
		va_list ap;
		va_start(ap, oflags);
		mode_t mode = va_arg(ap, int);
		va_end(ap);
		dev = dtree_mknodat(dev, path, mode, DEV_FIL);
	}
	if (dtree_nlink(dev)<0) return -1;
	if (DEV_ID(dev)==DEV_FIL) {
		struct _File * file = (struct _File *)(dev+1);
		struct _OpenFileDescription *f = g_slice_alloc(sizeof(struct _OpenFileDescription));// open file description 
		f->dev_id = DEV_FIL;
		f->fildes = fildes;
		f->offset = 0;
		if (oflags & O_TRUNC ) f->offset = 0;// O_TRUNC
		if (oflags & O_APPEND) f->offset = file->size;// O_TRUNC
		f->file = file;
		dev = (Device_t*)f;
	}
	DEV_PTR(fildes) = dev;
	return fildes;
}
ssize_t read	(int fildes, void *buf, size_t nbyte){
	Device_t * dev = DEV_PTR(fildes);
	const DeviceClass_t* dev_class = DEV_CLASS(dev);
	return dev_class->read (dev, buf, nbyte);
}
ssize_t write	(int fildes, const void *buf, size_t nbyte){
	Device_t * dev = DEV_PTR(fildes);//_devices[fildes];
	const DeviceClass_t* dev_class = DEV_CLASS(dev);
	return dev_class->write(dev, buf, nbyte);
}
int 	close	(int fildes){
	Device_t* dev = DEV_PTR(fildes);
	const DeviceClass_t* dev_class = DEV_CLASS(dev);
	device_flag_free(fildes);
	int rc = dev_class->close(dev);
	dtree_unref(dev);
	return rc;
}

//! \}
#endif
#if defined(_POSIX_FILE_SYSTEM) && (_POSIX_FILE_SYSTEM>0)
/*! \defgroup POSIX_FILE_SYSTEM POSIX: File System
	\ingroup _posix
	\{
access( ), chdir( ), closedir( ), creat( ), fchdir( ), fpathconf( ), fstat( ), fstatvfs( ), getcwd( ), link( ),
mkdir( ), mkstemp( ), opendir( ), pathconf( ), readdir( ), remove( ), rename( ), rewinddir( ), rmdir( ),
stat( ), statvfs( ), tmpfile( ), tmpnam( ), truncate( ), unlink( ), utime( ) */

/* The value of amode is either the bitwise-inclusive OR of the access permissions to be checked
	(R_OK, W_OK, X_OK) or the existence test (F_OK).

AT_EACCESS The checks for accessibility (including directory permissions checked during
pathname resolution) shall be performed using the effective user ID and
group ID instead of the real user ID and group ID as required in a call to
access( ).
	
 */
int symlinkat(const char *path1, int fd, const char *path2){
	Device_t* dirp = DEV_PTR(fd);// -1 => NULL, -2 => CWD
	dirp = dtree_path(dirp, path2, &path2);
	if (dirp==NULL || path2!=NULL) return -1;
	Device_t* dev = dtree_mknodat(dirp, path2,  0, DEV_LNK);
	*(void**)(dev +1) = (void*)path1;// strndup
	return 0;
}
int mknodat(int fd, const char *path, mode_t mode, dev_t dev_id){
	const char* name;
	Device_t* dirp = DEV_PTR(fd);// -1 => NULL, -2 => CWD
	dirp = dtree_path(dirp, path, &name);
	if (dirp==NULL || name==NULL) return -1;
	Device_t* f = dtree_mknodat(dirp, name,  mode, dev_id);
	return 0;
}
int mknod (const char *path, mode_t mode, dev_t dev_id){
	return mknodat(AT_FDCWD, path, mode, dev_id);
}
int mkfifo(const char *path, mode_t mode){
	return mknodat(AT_FDCWD, path, mode, DEV_FIFO);
}
int access(const char *path, int amode) {
	return faccessat(AT_FDCWD, path, amode, 0);
}
int link  (const char *path1, const char *path){
	return linkat(AT_FDCWD, path1, AT_FDCWD, path, AT_SYMLINK_FOLLOW);
}
int linkat(int fd1, const char *path1, int fd2, const char *path2, int flag)
{
	Device_t* dev = DEV_PTR(fd2);
	dev = dtree_path(dev, path2, &path2);// flag
	if (dtree_nlink(dev)<0) return -1;
	
	Device_t* dirp = DEV_PTR(fd1);
	dirp = dtree_path(dirp, path1, &path1);
	dtree_insert(dirp, path1, dev);
	return 0;
}
int mkdir (const char *path, mode_t mode){
	return mknodat(AT_FDCWD, path, mode, DEV_DIR);
}
int mkfifoat(int fd, const char *path, mode_t mode){
	return mknodat(fd, path, mode, DEV_FIFO);
}
int symlink(const char *path1, const char *path2){
	return symlinkat(path1, AT_FDCWD, path2);
}
inline int creat (const char *path, mode_t mode) {
    return open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
}
int fsync (int fildes){// если операция не завершена, ждать завершения
	return 0;
}
int fdatasync(int fildes){

//	msync(data);
	return 0;
}
int ftruncate(int fildes, off_t length){
	Device_t* dev = DEV_PTR(fildes);
	const DeviceClass_t* dev_class = DEV_CLASS(dev);//[dev->dev_id];
	return dev_class->trunc(dev, length);
}
int chdir(const char *path){
	const char* name;
	Device_t* dirp = dtree_path(NULL, path, &name);
	if (dirp==NULL || name!=NULL) return -1;
	dentry_cwd = dirp;
	return 0;
}
int fchdir(int fildes){
	dentry_cwd = DEV_PTR(fildes);
	return 0;
}
int fstat (int fildes, struct stat *buf){
	Device_t* dev = DEV_PTR(fildes);
	*buf = *(struct stat *)dev;// исправить
	if (DEV_ID(dev)==DEV_FIL) {// дописать
		
	}
}
int stat (const char *restrict path, struct stat *restrict buf){
	return fstatat(AT_FDCWD, path, buf, 0);
}
int unlink(const char *path) {
	return unlinkat(AT_FDCWD, path, 0);// AT_REMOVEDIR
}
int rmdir(const char *path) {
	return unlinkat(AT_FDCWD, path, AT_REMOVEDIR);// AT_REMOVEDIR
}
/*! \} */
#endif
#if defined(_POSIX_FILE_SYSTEM_FD) && (_POSIX_FILE_SYSTEM_FD>0)
/*! \defgroup POSIX_FILE_SYSTEM_FD POSIX: File System File Descriptor Routines
	\ingroup _posix
	\{
faccessat( ), fdopendir( ), fstatat( ), linkat( ), mkdirat( ), openat( ), renameat( ), unlinkat( ),
utimensat( ) */
int 	unlinkat(int fd, const char *path, int flag) {
	Device_t* dev = dtree_path(DEV_PTR(fd), path, &path);
	if (dev==NULL || path!=NULL) return -1;
	dtree_unref(dev);
	return 0;
}
int faccessat(int fd, const char *path, int amode, int flag) {
	
	const char* name;
	Device_t* dev = DEV_PTR(fd);
	dev = dtree_path(dev, path, &name);
	if (dev==NULL || name!=NULL) return -ENOENT;
	if(amode == F_OK) return 0;
	amode &= 007;
	int rel = 6; // \todo установить отношение к группе -0(other) +3(group member) +6(owner)
	return (((dev->mode>>rel) & amode) == amode)? 0: -EACCES;
}
int fstatat(int fd, const char *restrict path, struct stat *restrict buf, int flag){
	Device_t* dev = DEV_PTR(fd);
	const char* name;
	dev = dtree_path(dev, path, &name);
	if (dev==NULL || name!=NULL) return -1;
	*buf = *(struct stat *)dev;
	return 0;
}
int mkdirat(int fd, const char *path, mode_t mode) {
	return mknodat(fd, path, mode, DEV_DIR);
}
#if 0
int linkat(int fd1, const char *path1, int fd2, const char *path2, int flag){
	const char* name = NULL;
	Device_t* dirp = DEV_PTR(fd2);
	dirp = dtree_path(dirp, path2, &name);
	if (dirp==NULL || name==NULL) return -1;
	Device_t* f = dtree_mknodat(dirp, name,  0, DEV_LNK);
	if (f!=NULL) {
		f->size = strlen(path1);
		//f->fildes = fd1; - исправить
		f->phandle = (void*)path1;// strndup
		f->mode = dirp->mode;
	}
	return 0;
}
#endif
/*! \} */
#endif
#if defined(_POSIX_FILE_ATTRIBUTES) && (_POSIX_FILE_ATTRIBUTES>0)
/*! \defgroup POSIX_FILE_ATTRIBUTES POSIX: File Attributes 
	\ingroup _posix
	\{
*/
int chmod(const char *path, mode_t mode){
	Device_t * dev = dtree_path(NULL, path, &path);
	if (dev==NULL || path!=NULL) return -1;
	dev->mode = mode & _umask;
	return 0;
}
int fchmod(int fildes, mode_t mode) {
	Device_t* dev = DEV_PTR(fildes);
	dev->mode = mode & _umask;
	return 0;
}
int chown(const char *path,  uid_t uid, gid_t gid){
	Device_t * dev = dtree_path(NULL, path, &path);
	if (dev==NULL || path!=NULL) return -1;

	if ((uid_t)-1 != uid) dev->uid = uid;
	if ((gid_t)-1 != gid) dev->gid = gid;
	return 0;
}
int fchown(int fildes, uid_t uid, gid_t gid){
	Device_t* dev = DEV_PTR(fildes);

	if ((uid_t)-1 != uid) dev->uid = uid;
	if ((gid_t)-1 != gid) dev->gid = gid;
	return 0;
}
mode_t umask(mode_t cmask){
	return atomic_exchange(&_umask, cmask);
}
/*! \} */
#endif
#if defined(_POSIX_FILE_ATTRIBUTES_FD) && (_POSIX_FILE_ATTRIBUTES_FD>0)
/*! \defgroup POSIX_FILE_ATTRIBUTES_FD POSIX: File Attributes File Descriptor Routines 
	\ingroup _posix
	\{ */
int fchmodat(int fildes, const char *path, mode_t mode, int flag) {
	Device_t* dev = DEV_PTR(fildes);
	const char *name;
	dev = dtree_path(dev, path, &name);
	if (dev==NULL || name!=NULL) return -1;
	dev->mode = mode & _umask;
	return 0;
}
int fchownat(int fildes, const char *path, uid_t uid, gid_t gid){
	Device_t* dev = DEV_PTR(fildes);
	const char *name;
	dev = dtree_path(dev, path, &name);
	if (dev==NULL || name!=NULL) return -1;

	if ((uid_t)-1 != uid) dev->uid = uid;
	if ((gid_t)-1 != gid) dev->gid = gid;
	return 0;
}
//! \}
#endif
#if defined(_POSIX_FD_MGMT) && (_POSIX_FD_MGMT>0)
/*! \defgroup POSIX_FD_MGMT POSIX: File Descriptor Management
	\ingroup _posix
dup( ), dup2( ), fcntl( ), fgetpos( ), fseek( ), fseeko( ), fsetpos( ), ftell( ), ftello( ), 
ftruncate( ), lseek( ), rewind( )
	*/
off_t lseek(int fildes, off_t offset, int whence){
	Device_t* dev = DEV_PTR(fildes);
	const DeviceClass_t* dev_class = DEV_CLASS(dev);
	return dev_class->seek(dev+1, offset, whence);
}
int lstat(const char *restrict path, struct stat *restrict buf){
	return fstatat(AT_FDCWD, path, buf, AT_SYMLINK_NOFOLLOW);
}
int fcntl(int fildes, int cmd, ...)
{
	va_list ap;
	va_start (ap, cmd);
	int res;
	Device_t* dev = DEV_PTR(fildes);
	switch (cmd) {
	default: break;
	}
	return 0;
	va_end(ap);
}
#endif
