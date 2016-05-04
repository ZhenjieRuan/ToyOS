/*
 *  RAMDISK
 */
#include "ramdisk.h"

MODULE_LICENSE("GPL");

static int ramdisk_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations ramdisk_proc_operations;

static struct proc_dir_entry *proc_entry;

static spinlock_t my_lock = SPIN_LOCK_UNLOCKED;

/* ioctl entry point */
static int ramdisk_ioctl(struct inode *inode, struct file *file,
															 unsigned int cmd, unsigned long arg)
{
	spin_lock(&my_lock);
	int ret = 0;
	int size;
	char* pathname;
	char* data;
	ioctl_args_t* args = (ioctl_args_t *)vmalloc(sizeof(ioctl_args_t));
	if(copy_from_user(args, (ioctl_args_t *)arg, sizeof(ioctl_args_t))) {
		printk("<1> Error copy from user\n");
	}
	if (args->pathname != NULL) {
		size = strnlen_user(args->pathname, 14);
		pathname = (char *)kmalloc(size, GFP_KERNEL);
		copy_from_user(pathname, args->pathname, size);
	}
	/*printk("Got user path %s\n", args->pathname);*/
	switch (cmd) {
		case RD_INIT:
			printk("<1> num_blocks:%d\n", args->num_blks);
			init_fd_table();		
			ret = init_fs(args->num_blks);
			vfree(args);
			spin_unlock(&my_lock);
			return ret;
		case RD_OPEN:
			printk("<1> Opening %s\n", args->pathname);
			// Send fd back to user space
			ret = open(args->pid, args->pathname);
			vfree(args);
			spin_unlock(&my_lock);
			return ret;
		case RD_CLOSE:
			printk("<1> Switch case close\n");
			ret = close(args->pid, args->fd_num);
			vfree(args);
			spin_unlock(&my_lock);
			return ret;
		case RD_READ:
			ret = read(args->fd_num, args->address, args->num_bytes, args->pid);
			vfree(args);
			spin_unlock(&my_lock);
			return ret;
		case RD_WRITE:
			data = vmalloc(args->num_bytes);
			printk("<1> Here!\n");
			copy_from_user(data, args->address, args->num_bytes);
			printk("<1> Got write data from user: %s\n",data);
			ret = write(args->fd_num, data, args->num_bytes, args->pid);
			vfree(data);
			spin_unlock(&my_lock);
			return ret;
 		case RD_CREATE:
			ret = create(pathname);
			kfree(pathname);
			vfree(args);
			spin_unlock(&my_lock);
			return ret;
			break;
		case RD_LSEEK:
			ret = lseek(args->pid, args->fd_num, args->offset);
			vfree(args);
			spin_unlock(&my_lock);
			return ret;
		case RD_MKDIR:
			printk("<1> Mkdir %s\n", pathname);
			ret = mkdir(pathname);
			kfree(pathname);
			vfree(args);
			spin_unlock(&my_lock);
			return ret;
			break;
		case RD_UNLINK:
			ret = unlink(pathname);
			kfree(pathname);
			vfree(args);
			spin_unlock(&my_lock);
			return ret;
			break;
		case RD_READDIR:
			ret = readdir(args->pid, args->fd_num, args->address);
			vfree(args);
			spin_unlock(&my_lock);
			return ret;
		default:
			printk("<1> hitting default case \n");
			vfree(args);
			spin_unlock(&my_lock);
			return -EINVAL;
			break;			
	}

	spin_unlock(&my_lock);
	return 0;
}

static int __init initialization_routine(void)
{
	printk("<1> Loading Module\n");

	ramdisk_proc_operations.ioctl = ramdisk_ioctl;

	/* create proc entry */
	proc_entry = create_proc_entry("ioctl_ramdisk_test", 0444, NULL);
	if (!proc_entry)
	{
		printk("<1> Error creating /proc entry\n");
		return 1;
	}

	proc_entry->proc_fops = &ramdisk_proc_operations;

	/*spin_lock_init(&my_lock);*/

	return 0;
}

static void __exit cleanup_routine(void)
{
	printk("<1> Dumping module\n");
	remove_proc_entry("ioctl_ramdisk_test", NULL);
	cleanup_fs();
	return;
}

module_init(initialization_routine);
module_exit(cleanup_routine);
