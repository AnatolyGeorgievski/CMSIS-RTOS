
#include <unistd.h>
#include <errno.h>
#include <sys/limits.h>
#include <sys/stdio.h>
typedef struct _DeviceClass DeviceClass_t;
long _pathconf(Device_t * dev, int name){
	long v;
	if (dev==NULL) { errno = ENOENT; return -1; }
	const DeviceClass_t *dev_class = DEV_CLASS(dev);
	// выбор согласно способностям устройства
	switch (name) {
#if 0
	case _PC_FILESIZEBITS	: v = FILESIZEBITS; 	break;
	case _PC_LINK_MAX	: v = LINK_MAX; 	break;
	case _PC_MAX_CANON	: v = MAX_CANON; 	break;
	case _PC_MAX_INPUT	: v = MAX_INPUT; 	break;
	case _PC_NAME_MAX	: v = NAME_MAX; 	break;
	case _PC_PATH_MAX	: v = PATH_MAX; 	break;
	case _PC_PIPE_BUF	: v = PIPE_BUF; 	break;
// Symbolic links can be created.
	case _PC_2_SYMLINKS	: v = POSIX2_SYMLINKS; 	break;
#endif
#if defined(_POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO>0)
// Minimum number of bytes of storage actually allocated for any portion of a file.
	case _PC_ALLOC_SIZE_MIN		: v = 1UL<<dev_class->adv.xfer_block; 	break;
// Recommended increment for file transfer sizes
	case _PC_REC_INCR_XFER_SIZE	: v = 1UL<<dev_class->adv.xfer_block; break;
// Maximum recommended file transfer size
	case _PC_REC_MAX_XFER_SIZE	: v = dev_class->adv.xfer_max<<dev_class->adv.xfer_block; 	break;
// Minimum recommended file transfer size
	case _PC_REC_MIN_XFER_SIZE	: v = dev_class->adv.xfer_min<<dev_class->adv.xfer_align; 	break;
// Recommended file transfer buffer alignment. BLOCK_MEDIA_UNIT
	case _PC_REC_XFER_ALIGN		: v = 1UL<<dev_class->adv.xfer_align; 	break;
#endif
//	case _PC_SYMLINK_MAX		: v = SYMLINK_MAX; 				break;
	case _PC_CHOWN_RESTRICTED	: v = _POSIX_CHOWN_RESTRICTED; 	break;
	case _PC_NO_TRUNC	: v = _POSIX_NO_TRUNC; 	break;
//	case _PC_VDISABLE	: v = _POSIX_VDISABLE; 	break;
// Asynchronous input or output operations may be performed for the associated file
	case _PC_ASYNC_IO	: v = _POSIX_ASYNC_IO; 	break;
// Prioritized input or output operations may be performed for the associated file
	case _PC_PRIO_IO	: v = _POSIX_PRIO_IO; 	break;
// Synchronized input or output operations may be performed for the associated file
	case _PC_SYNC_IO	: v = _POSIX_SYNC_IO; 	break;
// The resolution in nanoseconds for all file timestamps.
	case _PC_TIMESTAMP_RESOLUTION	: v = _POSIX_TIMESTAMP_RESOLUTION; 	break;
	default: v=-1; errno = EINVAL; break;
	}
	return v;
}
/*! \brief Распознавание параметров файловой системы
	\ingroup POSIX_DEVICE_IO
 */
long fpathconf(int fildes, int name){
	Device_t* dev = DEV_PTR(fildes);
	return _pathconf(dev, name);
}
/*! \brief Распознавание параметров файловой системы
	\ingroup POSIX_DEVICE_IO
 */
long pathconf(const char *path, int name){
	Device_t* dev = dtree_path(NULL, path, &path);
//	if(dev==NULL) { errno = ENOENT; return -1};
	return _pathconf(dev, name);
}
