#ifndef _CPIO_H_
#define _CPIO_H_
/*
C_IRUSR Read by owner. 0000400
C_IWUSR Write by owner. 0000200
C_IXUSR Execute by owner. 0000100
C_IRGRP Read by group. 0000040
C_IWGRP Write by group. 0000020
C_IXGRP Execute by group. 0000010
C_IROTH Read by others. 0000004
C_IWOTH Write by others. 0000002
C_IXOTH Execute by others. 0000001
*/
#define C_ISUID 0004000 //Set user ID. 
#define C_ISGID 0002000 //Set group ID. 
#define C_ISVTX 0001000 //On directories, restricted deletion flag. 
/*C_ISDIR Directory. 0040000
C_ISFIFO FIFO. 0010000
C_ISREG Regular file. 0100000
C_ISBLK Block special. 0060000
C_ISCHR Character special. 0020000
C_ISCTG Reserved. 0110000
C_ISLNK Symbolic link. 0120000
C_ISSOCK Socket. 0140000
*/
#define MAGIC "070707"
#endif//_CPIO_H_