/*
 *  RAMDISK
 */
#include "ramdisk.h"

MODULE_LICENSE("GPL");

static int ramdisk_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations ramdisk_proc_operations;

static struct proc_dir_entry *proc_entry;

/* ioctl entry point */
static int ramdisk_ioctl(struct inode *inode, struct file *file,
															 unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int size;
	char* pathname;
	ioctl_args_t* args = (ioctl_args_t *)kmalloc(sizeof(ioctl_args_t), GFP_KERNEL);
	if(copy_from_user(args, (ioctl_args_t *)arg, sizeof(ioctl_args_t))) {
		printk("<1> Error copy from user\n");
	}
	size = strnlen_user(args->pathname, 14);
	pathname = (char *)kmalloc(size, GFP_KERNEL);
	copy_from_user(pathname, args->pathname, size);
	printk("Got user path %s\n", args->pathname);
	switch (cmd) {
		case RD_INIT:
			printk("<1> num_blocks:%d\n", args->num_blks);
			init_fd_table();		
			return init_fs(args->num_blks);
		case RD_OPEN:
			printk("<1> Opening %s\n", args->pathname);
			// Send fd back to user space
			return open(args->pid, args->pathname);
		case RD_CLOSE:
			printk("<1> Switch case close\n");
			return close(args->pid, args->fd_num);
		case RD_CREATE:
			ret = create(pathname);
			kfree(pathname);
			return ret;
			break;
		case RD_LSEEK:
			return lseek(args->pid, args->fd_num, args->offset);
		case RD_MKDIR:
			ret = mkdir(pathname);
			kfree(pathname);
			return ret;
			break;
		case RD_UNLINK:
			ret = unlink(pathname);
			kfree(pathname);
			return ret;
			break;
		default:
			printk("<1> hitting default case \n");
			return -EINVAL;
			break;			
	}

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
