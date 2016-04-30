/*
 *  DISCOS
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include "discos.h"

MODULE_LICENSE("GPL");

/* 2MB of free memory */
block_t* disc;


static int ramdisk_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations ramdisk_proc_operations;

static struct proc_dir_entry *proc_entry;

/* ioctl entry point */
static int ramdisk_ioctl(struct inode *inode, struct file *file,
															 unsigned int cmd, unsigned long arg)
{
	unsigned int size;
	char *pathname;
	switch (cmd) {
		case RD_CREATE:
			size = strnlen_user((char *)arg, 100);
			pathname = (char *)kmalloc(size, GFP_KERNEL);
			copy_from_user(pathname, (char *)arg, size);
			printk("Got user path %s\n", pathname);
			break;
		default:
			return -EINVAL;
			break;			
	}

	return 0;
}

static int __init initialization_routine(void)
{
	printk("<1> Loading Module\n");

	/* Getting memory for disc*/
	disc = (block_t *)vmalloc(sizeof(block_t)*8192);

	ramdisk_proc_operations.ioctl = ramdisk_ioctl;

	/* create proc entry */
	proc_entry = create_proc_entry("ioctl_dicos_test", 0444, NULL);
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
	remove_proc_entry("ioctl_discos_test", NULL);
	vfree(disc);
	return;
}

module_init(initialization_routine);
module_exit(cleanup_routine);
