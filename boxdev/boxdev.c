#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#define DRV_NAME           "boxdev"
#define BUFFER_SIZE 255

static dev_t first; 		// Global variable for the first device number 
static struct cdev c_dev; 	// Global variable for the character device structure
static struct class *cl; 	// Global variable for the device class

char w[BUFFER_SIZE];
size_t w_len;

static int my_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO DRV_NAME " : Driver: open()\n");
	return 0;
}
static int my_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO DRV_NAME " : Driver: close()\n");
	return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	size_t used_len = len > w_len ? w_len : len;
	printk(KERN_INFO DRV_NAME  " : Driver: read()\n");
	if (*off == 0) {
		if (copy_to_user(buf, &w, used_len) != 0)  {
        	return -EFAULT;
    	} else {
			(*off)++;
			return w_len;
		}
	} else {
		return 0;
	}
}
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	size_t used_len = len > BUFFER_SIZE ? BUFFER_SIZE : len;
	printk(KERN_INFO DRV_NAME " : Driver: write()\n");
	if (copy_from_user(&w, buf, used_len) != 0)
		return -EFAULT;
	else {
		w_len = used_len;
		return len;
	}
}

static struct file_operations pugs_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};
 
static int __init ofcd_init(void) /* Constructor */
{
	printk(KERN_INFO DRV_NAME " : Registered\n");
	
	if (alloc_chrdev_region(&first, 0, 1, "JON") < 0) { goto err_return;	}
	if ((cl = class_create(THIS_MODULE, "boxdev")) == NULL) { goto err_unregister_chrdev_return; }
	if (device_create(cl, NULL, first, NULL, "boxdev") == NULL) { goto err_class_destroy_return; }
	cdev_init(&c_dev, &pugs_fops);
	if (cdev_add(&c_dev, first, 1) == -1) { goto err_device_destroy_return; }
	
	return 0;
  
	/* Cleanup of registers */        
err_device_destroy_return:        
	device_destroy(cl, first);
err_class_destroy_return:
	class_destroy(cl);
err_unregister_chrdev_return:
	unregister_chrdev_region(first, 1);
err_return:
	return -1;
}
 
static void __exit ofcd_exit(void) /* Destructor */
{
  cdev_del(&c_dev);
  device_destroy(cl, first);
  class_destroy(cl);
  unregister_chrdev_region(first, 1);
  printk(KERN_INFO DRV_NAME " : Unregistered\n");
}
 
module_init(ofcd_init);
module_exit(ofcd_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jon Henneberg");
MODULE_DESCRIPTION("Boxdev driver");
