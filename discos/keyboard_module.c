/*
 *  Simple Keyboard Driver
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/wait.h>

MODULE_LICENSE("GPL");

/* Attribute structure, copied to user space. */
struct keyboard_char_t {
  char character;
};
/* Cookie for requesting and removing the interrupt */
struct keyboard_module_cookie_t {
  char * contents;
};
/* A structure that holds the most recently retrieved character, along with some state information. */
struct keyboard_buffer_t {
  char character;
  int ready;
  int shift;
};

static struct keyboard_buffer_t keyboard_buffer;

static struct keyboard_char_t key_input;

DECLARE_WAIT_QUEUE_HEAD(keyboard_wq);

/** The module shares the interrupt, requiring a struct for dev_id * */
static struct keyboard_module_cookie_t keyboard_module_cookie = {"imacookie"};

#define IOCTL_KEYBOARD _IOR(0, 6, struct keyboard_char_t)

static int pseudo_device_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

static inline unsigned char inb( unsigned short usPort ) {
  unsigned char uch; 
  asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
  return uch;
}

static inline void outb( unsigned char uch, unsigned short usPort ) {
  asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
} 

/** Our ISR for IRQ1. Places the character in a datastructure for waiting processes to access.  */
irqreturn_t keyboard_handler(int irq, void *dev_id) {
  char c;
  static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
  
  if( (c = inb( 0x60 )) & 0x80 ) { /* A key has been released. */
    c &= 0x7f; //get the key released
    if( c == 0x2a || c == 0x36 ) { //Clear shift if we release the shift key.
      keyboard_buffer.shift = 0;
    }
    return IRQ_HANDLED;
  }
  
  if(c == 0x2a || c == 0x36) { //Detect shift keys
    keyboard_buffer.shift = 1;
    return IRQ_HANDLED;
  }
  
  keyboard_buffer.character = scancode[(int)c];
  
  if(c == 0x0e ) { //Backspace
    keyboard_buffer.character = 0x08;
  } else if(c == 0x1c) { //Enter (Shift screws it up)
    keyboard_buffer.character = '\n';
  } else if(keyboard_buffer.shift) {
    if(keyboard_buffer.character >= 0x61 || keyboard_buffer.character <= 0x7D ) { //Check for letters to capitalize.
      keyboard_buffer.character -= 0x20;
    }
  }
  
  keyboard_buffer.ready = 1;
  wake_up_interruptible(&keyboard_wq);

  return IRQ_HANDLED;
}

static int __init initialization_routine(void) {
  printk("<1> Loading module\n");
  //Get IRQ1
  request_irq(1,keyboard_handler ,IRQF_SHARED,"keyboard_module", (void *) &keyboard_module_cookie);
  pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;

  /* Start create proc entry */
  proc_entry = create_proc_entry("ioctl_keyboard_test", 0444, NULL);
  if(!proc_entry)
    {
      printk("<1> Error creating /proc entry.\n");
      return 1;
    }

  proc_entry->proc_fops = &pseudo_dev_proc_operations;

  return 0;
}

static void __exit cleanup_routine(void) {
  printk("<1> Dumping module\n");
  remove_proc_entry("ioctl_keyboard_test", NULL);
  //Release IRQ1
  free_irq(1,(void *)&keyboard_module_cookie); 
  return;
}

/***
 * ioctl() entry point...
 */
static int pseudo_device_ioctl(struct inode *inode, struct file *file,
                               unsigned int cmd, unsigned long arg)
{
  unsigned long copied;
  switch (cmd){

  case IOCTL_KEYBOARD:
    wait_event_interruptible(keyboard_wq, (keyboard_buffer.ready == 1));  /** Wait for a character */
    key_input.character = keyboard_buffer.character;
    keyboard_buffer.ready = 0;
    copied = copy_to_user((struct keyboard_char_t *)arg,&key_input,
                          sizeof(struct keyboard_char_t));
    printk("<1> ioctl: call to the keyboard driver ( %c ), but we couldn't copy %u bytes!\n",
	   key_input.character,copied);
    break;
  
  default:
    return -EINVAL;
    break;
  }
  
  return 0;
}

module_init(initialization_routine);
module_exit(cleanup_routine);
