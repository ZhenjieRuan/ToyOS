
# DISCOS

#### Jason Ruan, Tommy Unger, Sean Smith

An RAMDISK OS with basic file operations support.

## To run:
	
	make clean
	make
	make demo

## File Operations

	+ int rd_creat(char *pathname) -- create a regular file with absolute pathname from the root of the directory tree, where each directory filename is delimited by a "/" character.  On success, you should return 0, else if the file corresponding to pathname already exists you should return -1, indicating an error. Note that you need to update the parent directory file, to include the new entry. 
	+ int rd_mkdir(char *pathname) -- this behaves like rd_creat() but pathname refers to a directory file. If the file already exists, return -1 else return 0. Note that you need to update the parent directory file, to include the new entry.
	+ int rd_open(char *pathname) -- open an existing file corresponding to pathname (which can be a regular or directory file) or report an error if file does not exist. When opening a file, you should return a file descriptor value that will index into the process'  ramdisk file descriptor table. As stated earlier, this table entry will contain a pointer to a file object. You can assume the file object has the file position set to 0. Return a value of -1 to indicate an error, if the file does not exist.
	+ int rd_close(int fd) -- close the corresponding file descriptor and release the file object matching the value returned from a previous rd_open(). Return 0 on success and -1 on error. An error occurs if fd refers to a non-existent file.
	+ int rd_read(int fd, char *address, int num_bytes)-- read up to num_bytes from a regular file identified by file descriptor, fd, into a process' location at address. You should return the number of bytes actually read, else -1 if there is an error. An error occurs if the value of fd refers either to a non-existent file or a directory file.
	+ int rd_write(int fd, char *address, int num_bytes) -- write up to num_bytes from the specified address in the calling process to a regular file identified by file descriptor, fd. You should return the actual number of bytes written, or -1 if there is an error. An error occurs if the value of fd refers either to a non-existent file or a directory file.
	+ int rd_lseek(int fd, int offset) -- set the file object's file position identified by file descriptor, fd, to offset, returning the new position, or the end of the file position if the offset is beyond the file's current size. This call should return -1 to indicate an error, if applied to directory files, else 0 to indicate success. 
	+ int rd_unlink(char *pathname) -- remove the filename with absolute pathname from the filesystem, freeing its memory in the ramdisk. This function returns 0 if successful or -1 if there is an error.  An error can occur (1) if the pathname does not exist, (2) if you attempt to unlink a non-empty directory file, (3) if you attempt to unlink an open file, or (4) if you attempt to unlink the root directory file. 
	+ int rd_readdir(int fd, char *address) -- read one entry from a directory file identified by fd and store the result in user-memory at the specified value of address. When reading a given entry, you should return a 14 byte filename, followed by a two byte index node number for the file. 1 is returned on success (to indicate a successfully read directory entry) and -1 on error. An error occurs if either of the arguments to rd_readdir() are invalid. Each time a process calls rd_readdir() you should increment the file position in the corresponding file object to refer to the next entry. If either the directory is empty or the last entry has been read by a previous call to rd_readdir() you should return 0, to indicate end-of-file.



