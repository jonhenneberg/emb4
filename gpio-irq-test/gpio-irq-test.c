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
 
#define DRV_NAME           "gpio-irq-test"

static dev_t first; 			// Global variable for the first device number 
static struct cdev c_dev; 	// Global variable for the character device structure
static struct class *cl; 	// Global variable for the device class


struct gpio_irq_test {
   u16 irq;
   u16 irq_pin, gpio_pin, led_pin;
   u8 irq_fired, irq_enabled, led_value;
};

static struct gpio_irq_test test_data;


static irqreturn_t gpio_test_irq_interrupt_handler(int irq, void* dev_id) {
	struct gpio_irq_test* data = (struct gpio_irq_test*)dev_id;
	printk(KERN_INFO DRV_NAME "IRQ: Interupt recived()\n");
	data->irq_fired = 1;
	data->led_value ^= 1;     
	gpio_set_value(data->led_pin, data->led_value);
	return IRQ_HANDLED;
}

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
	} else {
		gpio_set_value(test_data.gpio_pin, 0);
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
	int err;
	printk(KERN_INFO DRV_NAME " : Registered\n");

	test_data.irq_pin = 46;
	test_data.gpio_pin = 38;
	test_data.led_pin = 54;

	if (alloc_chrdev_region(&first, 0, 1, "JON") < 0) { goto err_return;	}
	if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) { goto unregister_chrdev_return; }
	if (device_create(cl, NULL, first, NULL, "gpiotest") == NULL) { goto class_destroy_return; }
	cdev_init(&c_dev, &pugs_fops);
	if (cdev_add(&c_dev, first, 1) == -1) { goto device_destroy_return; }

	if (gpio_request(test_data.gpio_pin, "out pin")) {
		printk(KERN_ALERT DRV_NAME " : Unable to register output pin %d\n", test_data.gpio_pin);
		goto device_destroy_return;
	}

	if(gpio_request(test_data.led_pin, "led pin")) {
		printk(KERN_ALERT DRV_NAME " : Unable to register led pin %d\n", test_data.led_pin);
		goto free_outpin_return;
	}

	if (gpio_request(test_data.irq_pin, "irq pin")) {
		printk(KERN_ALERT DRV_NAME " : Unable to register irq pin %d\n", test_data.led_pin);
		goto free_ledpin_return;
	}

	gpio_direction_output(test_data.led_pin, 0);
	gpio_direction_output(test_data.gpio_pin, 0);
	gpio_direction_input(test_data.irq_pin);
	

   err = gpio_to_irq(test_data.irq_pin);
   if (err < 0) {
      printk(KERN_ALERT DRV_NAME " : failed to get IRQ for pin %d.\n", test_data.irq_pin);
      goto free_irqpin_return;
   } else {
      test_data.irq = (u16)err;
      err = 0;
   }

	err = request_any_context_irq(test_data.irq, gpio_test_irq_interrupt_handler, IRQF_TRIGGER_RISING | IRQF_DISABLED, DRV_NAME, (void*)&test_data);
   if (err < 0) {
      printk(KERN_ALERT DRV_NAME " : failed to enable IRQ %d for pin %d.\n", test_data.irq, test_data.irq_pin);
      goto free_irqpin_return;
   } else
      test_data.irq_enabled = 1;


	return 0;

	/* Cleanup of registers */	
	free_irqpin_return:
		gpio_free(test_data.irq_pin);
	free_ledpin_return:
		gpio_free(test_data.led_pin);
	free_outpin_return:
		gpio_free(test_data.gpio_pin);
	device_destroy_return:	
   	device_destroy(cl, first);
	class_destroy_return:
   	class_destroy(cl);
	unregister_chrdev_return:
		unregister_chrdev_region(first, 1);
	err_return:
		return -1;
}
 
static void __exit ofcd_exit(void) /* Destructor */
{
	free_irq(test_data.irq, (void*)&test_data);

	gpio_free(test_data.led_pin);
	gpio_free(test_data.gpio_pin);
	gpio_free(test_data.irq_pin);

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
MODULE_DESCRIPTION("GPIO IRQ Test");
