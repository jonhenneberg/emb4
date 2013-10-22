#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
 
#define DRV_NAME           "gpio-test"

static dev_t first; 		// Global variable for the first device number 
static struct cdev c_dev; 	// Global variable for the character device structure
static struct class *cl; 	// Global variable for the device class


struct gpio_irq_test {
   u16 irq;
   u16 irq_pin, gpio_pin, led_pin;
};

static struct gpio_irq_test test_data;


static int my_open(struct inode *i, struct file *f) {
  printk(KERN_INFO DRV_NAME " : Driver: open()\n");
  return 0;
}

static int my_close(struct inode *i, struct file *f) {
  printk(KERN_INFO DRV_NAME " : Driver: close()\n");
  return 0;
}
 
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	char c[2];
	if (*off == 0) {
		printk(KERN_INFO DRV_NAME " : Driver: read()\n");
		c[0] = gpio_get_value(test_data.irq_pin) + 0x30;
		c[1] = '\n';

		if (copy_to_user(buf, &c, 2)) {
			return -EFAULT;
		} else	{
			(*off)++;
			return 2;
		}
	} else {
		return 0;
	}
}
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
	char c;
	printk(KERN_INFO DRV_NAME " : Driver: write()\n");	
	if (copy_from_user(&c, buf, 1))	return -EFAULT;

	if (buf[0] == '1') {
		gpio_set_value(test_data.gpio_pin, 1);
		gpio_set_value(test_data.led_pin, 1); 
	} else {
		gpio_set_value(test_data.gpio_pin, 0);
		gpio_set_value(test_data.led_pin, 1); 
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
	printk(KERN_INFO DRV_NAME " : Registered\n");

	test_data.irq_pin = 46;
	test_data.gpio_pin = 38;
	test_data.led_pin = 54;

	if (alloc_chrdev_region(&first, 0, 1, "JON") < 0) { goto err_return;	}
	if ((cl = class_create(THIS_MODULE, "gpio-test")) == NULL) { goto err_unregister_chrdev_return; }
	if (device_create(cl, NULL, first, NULL, "gpio-test") == NULL) { goto err_class_destroy_return; }
	cdev_init(&c_dev, &pugs_fops);
	if (cdev_add(&c_dev, first, 1) == -1) { goto err_device_destroy_return; }

	/* Confiure GPIO Pins*/
	if (gpio_request(test_data.gpio_pin, "out pin")) {
		printk(KERN_ALERT DRV_NAME " : Unable to register output pin %d\n", test_data.gpio_pin);
		goto err_device_destroy_return;
	}

	if(gpio_request(test_data.led_pin, "led pin")) {
		printk(KERN_ALERT DRV_NAME " : Unable to register led pin %d\n", test_data.led_pin);
		goto err_free_outpin_return;
	}

	if (gpio_request(test_data.irq_pin, "irq pin")) {
		printk(KERN_ALERT DRV_NAME " : Unable to register irq pin %d\n", test_data.led_pin);
		goto err_free_ledpin_return;
	}

	gpio_direction_output(test_data.led_pin, 0);
	gpio_direction_output(test_data.gpio_pin, 0);
	gpio_direction_input(test_data.irq_pin);

	return 0;

/* Cleanup of registers */	
err_free_ledpin_return:
	gpio_free(test_data.led_pin);
err_free_outpin_return:
	gpio_free(test_data.gpio_pin);
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
	/* Cleanup GPIO pins*/
	gpio_free(test_data.led_pin);
	gpio_free(test_data.gpio_pin);
	gpio_free(test_data.irq_pin);

	/* Cleanup charecter devices*/
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
MODULE_DESCRIPTION("GPIO Test");
