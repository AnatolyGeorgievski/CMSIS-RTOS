#ifndef _CPIO_H_
#define _CPIO_H_

#define C_IRUSR 0000400 // Read by owner.
#define C_IWUSR 0000200 // Write by owner.
#define C_IXUSR 0000100 // Execute by owner. 
#define C_IRGRP 0000040 // Read by group. 
#define C_IWGRP 0000020 // Write by group. 
#define C_IXGRP 0000010 // Execute by group. 
#define C_IROTH 0000004 // Read by others. 
#define C_IWOTH 0000002 // Write by others. 
#define C_IXOTH 0000001 // Execute by others. 

#define C_ISUID 0004000 // Set user ID. 
#define C_ISGID 0002000 // Set group ID. 
#define C_ISVTX 0001000 // On directories, restricted deletion flag. 
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