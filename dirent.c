#include <unistd.h>
#include <sys/stdio.h>
#include <dirent.h>
#include <errno.h>
typedef struct _dentry dentry_t;
struct _dentry {
	const char *d_name;
	uint32_t  key;// ключ для сравнения
	dentry_t* next;
};

int readdir_r(DIR *restrict dirp, struct dirent *restrict entry, struct dirent **restrict result){
	dentry_t *dentry;
	if (entry==NULL)
		dentry = dirp->list;
	else
		dentry =((dentry_t*)entry)->next;
	if (dentry==NULL) return ENOENT;
	*result = (struct dirent *)dentry;
	return 0;
}
DIR *opendir(const char *path){
	Device_t* dev = dtree_path(NULL, path, &path);
	if ((dev!=NULL && path==NULL) && dtree_nlink(dev)>=0) return (DIR*)(dev+1);
	return NULL;
}
int closedir(DIR *dirp){
	Device_t *dev = (Device_t*)dirp -1;
	dtree_unref(dev);
	return 0;
}
