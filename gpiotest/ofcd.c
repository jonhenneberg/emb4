#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
 
static dev_t first; // Global variable for the first device number 
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class

char c;

static int my_open(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: open()\n");
  return 0;
}
static int my_close(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: close()\n");
  return 0;
}
 
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	char c[2];
	printk(KERN_INFO "Driver: read()\n");
	c[0] = gpio_get_value(46) + 0x30;
	c[1] = '\n';

	if (copy_to_user(buf, &c, 2))
		return -EFAULT;
	else
	   return 2;

}
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	char c;
	printk(KERN_INFO "Driver: write()\n");	
	if (copy_from_user(&c, buf, 1))	return -EFAULT;

	if (buf[0] == '1') {
		gpio_set_value(38, 1);   
		gpio_set_value(54, 1);  
	} else {
		gpio_set_value(38, 0);
		gpio_set_value(54, 0);  
	}

	return len;
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
  printk(KERN_INFO "Namaskar: ofcd registered\n");
  if (alloc_chrdev_region(&first, 0, 1, "JON") < 0)
  {
    return -1;
  }
    if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
  {
    unregister_chrdev_region(first, 1);
    return -1;
  }
    if (device_create(cl, NULL, first, NULL, "gpiotest") == NULL)
  {
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }
    cdev_init(&c_dev, &pugs_fops);
    if (cdev_add(&c_dev, first, 1) == -1)
  {
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }
//	if (gpio_is_valid(38)) {
//		printk(KERN_INFO "GPIO: invalid GPIO 38\n");
//		return -1;
//	}
//	if (gpio_is_valid(54)) {
//		printk(KERN_INFO "GPIO: invalid GPIO 54\n");
//		return -1;
//	}
//	if (gpio_is_valid(46)) {
//		printk(KERN_INFO "GPIO: invalid GPIO 46\n");
//		return -1;
//	}

	if (gpio_request(38, "led out")) {
		printk(KERN_INFO "GPIO: unable to register GPIO38\n");
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	if(gpio_request(54, "led out_usr")) {
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		gpio_free(38);
		printk(KERN_INFO "GPIO: unable to register GPIO54\n");
		return -1;
	}
	if (gpio_request(46, "btn in")) {
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		gpio_free(38);
		gpio_free(46);
		printk(KERN_INFO "GPIO: unable to register GPIO46\n");
		return -1;
	}

		gpio_direction_output(38, 0);
		gpio_direction_output(54, 0);
		gpio_direction_input(46);


  return 0;
}
 
static void __exit ofcd_exit(void) /* Destructor */
{

	gpio_free(38);
	gpio_free(54);
	gpio_free(46);

  cdev_del(&c_dev);
  device_destroy(cl, first);
  class_destroy(cl);
  unregister_chrdev_region(first, 1);
  printk(KERN_INFO "Alvida: ofcd unregistered\n");
}
 
module_init(ofcd_init);
module_exit(ofcd_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <email_at_sarika-pugs_dot_com>");
MODULE_DESCRIPTION("Our First Character Driver");
