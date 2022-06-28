#include "cmsis_os.h"
#include <stdio.h>
#include "r3_object.h"
#if 0
int r3_file_write(struct _FileObject * file, uint8_t * data, size_t data_len)
{
    if (file->data==NULL){
        file->data = iflash_open(4, osSignalRef(osThreadGetId(), 2));
    } else  {
        osEvent event = osSignalWait(1<<2, 1);//100);
        if (event.status & osEventTimeout)
            return osErrorTimeoutResource;//ERROR(OBJECT, BUSY);
        osSignalClear(osThreadGetId(),1<<2);
        file->start_position += file->data_len;
    }
    if (data_len>0) 
		iflash_send(file->data, data, data_len);
    file->data_len = data_len;
	return osOK;
}
#endif
#if 0
static FILE* _fdopen(uint32_t object_identifier, const char* mode)
{
//	fd &= ((1<<22)-1);
//	uint32_t object_identifier = fd | (_FILE<<22);
    struct _FileObject * file = _object_lookup(devinfo, object_identifier);
	
	FILE * fp = file->fp;
	if (fp==NULL) {// создать
		// 
		fp->_bf._base
		fp->_bf._size = file->file_size;
		size = (file->file_size + (BLOCK_MEDIA_PAGE_SIZE-1))/BLOCK_MEDIA_PAGE_SIZE;
		page = rfs_vector_alloc(&rfs->space, 0, size);
	}
	// на чтение
	fp->_bf._base = media->read(media->data, fp->_bf._base, fp->lbfsize, fp->_lbfsize);
	fp->_r = fp->_bf._size;// доступно на чтение
	fp->_p = fp->_bf._base;
//	fp->object_identifier = object_identifier;
	return fp;
}

/*! \brief файл */
FILE* fopen (const char *restrict name, const char*restrict mode)
{// поиск текстовых строк по структурированному представлению директории
	const char *s = name;
	if (s[0]=='/'){// от корня
		s++;
	}
	do {
		while (s[0]!='\0' && s[0]!='/') s++;
		if (s[0]=='/') {
			for (i=0; i< parent->subordinate_list.count; i++) {
//				node_name = parent->subordinate_list.array[i].name;
				object_identifier = parent->subordinate_list.array[i].object_identifier;
				object = _object_lookup(devinfo, object_identifier);
				if (strncmp(object->object_name, name, s - name)==0)
				{
					break;
				}
			}
			s++;
			// открываем StrucruredView Object
			parent = object;//_object_lookup(devinfo, object_identifier);
		}
	} while (s[0]!='/');
	if (0) {
		
	}
	// . . .
	return _fopen(fd, mode);
}
#endif
