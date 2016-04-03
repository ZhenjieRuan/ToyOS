/**
 * ioctl test module -- Rich West
 * modified by -- Zhenjie Ruan
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>


MODULE_LICENSE("GPL");

/* attribute structures */
struct ioctl_test_t {
	char *string;
};

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)

static int pseudo_device_ioctl(struct inode *inode, struct file *file,
							                 unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

static int __init initialization_routine(void) {
	printk("<1> Loading module\n");

	pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;

	/* Start create proc entry */
	proc_entry = create_proc_entry("ioctl_test", 0444, NULL);
	if (!proc_entry) {
		printk("<1> Error creating /proc entry.\n");
		return 1;
	}

	proc_entry->proc_fops = &pseudo_dev_proc_operations;

	return 0;
}

/* printk version that prints to active tty */
void my_printk(char *string) {
	struct tty_struct *my_tty;

	my_tty = current->signal->tty;

	if (my_tty != NULL) {
		(*my_tty->driver->ops->write)(my_tty, string, strlen(string));
		(*my_tty->driver->ops->write)(my_tty, "\015\012", 2);
	}
}

static void __exit cleanup_routine(void) {
	printk("<1> Dumping module\n");
	remove_proc_entry("ioctl_test", NULL);

	return;
}

/***
 * ioctl() entry point...
 */
static int pseudo_device_ioctl(struct inode *inode, struct file *file,
															 unsigned int cmd, unsigned long arg)
{
	struct ioctl_test_t ioc;
	switch (cmd) {
		case IOCTL_TEST:
			copy_from_user(&ioc, (struct ioctl_test_t *)arg,
										 sizeof(struct ioctl_test_t));
			printk("<1> ioctl: call to IOCTL_TEST %s!\n",
							ioc.string);
			my_printk("Got msg in kernel\n");
			my_printk(ioc.string);
			break;

		default:
			return -EINVAL;
			break;
	}

	return 0;
}

module_init(initialization_routine);
module_exit(cleanup_routine);
