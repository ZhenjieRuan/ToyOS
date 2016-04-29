#ifndef _DISCOS_H_
#define _DISCOS_H_

#define CREATE	0
#define MKDIR		1
#define OPEN		2
#define CLOSE		3
#define READ		4
#define WRITE		5
#define LSEEK		6
#define UNLINK	7
#define READDIR 8

typedef struct inode {
	char[4] type;
	int size;
	char[40] location;
} inode_t;

#endif /* ifndef _DISCOS_H_ */
