// Device Driver is a piece of code that configure and manages a device
// In Kernel space we can see different types of device drivers
// CHAR drivers, Block device drivers and network device drivers
// Kernel will interact with these drivers to perform I/O operations

// CHAR drivers : access data from the device sequentially that is byte by byte
// no advanced buffering startegy is required as data is going byte by byte
// eg : keyboard

//Block drivers : access data from the device in fixed-size blocks
// They are responsible for managing the read and write requests for these blocks
// Block drivers typically use a buffer cache to improve performance eg ssd

// Device numbers : kernel assigns one number to identify the device number containg 2 parts
// major number : minor number (actual device driver identifier and device instance)
// nencessary for the kernel to identify and manage the device driver
#include <linux/module.h>
#include <linux/fs.h>

dev_t dev_number; // device number variable
struct cdev cdev;
/*
struct cdev {
	struct kobject kobj;
	struct module *owner;
	const struct file_operations *ops;
	struct list_head list;
	dev_t dev;
	unsigned int count;
} __randomize_layout;
*/

struct file_operations fops;
/*
struct file_operations {
	struct module *owner;
	fop_flags_t fop_flags;
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
	ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
	int (*iopoll)(struct kiocb *kiocb, struct io_comp_batch *, .....
*/

static int __init driver_init(void){
    /*Assign a device number*/
    alloc_chrdev_region(&dev_number, 0, 1, "my_device");
    printk(KERN_INFO "Device number assigned: Major: %d, Minor: %d\n", MAJOR(dev_number), MINOR(dev_number));

    /* Initialize cdev structure with fops */
    cdev_init(&cdev, &fops);
	cdev.owner = THIS_MODULE; // THIS_MODULE is a macro that points to the current module

    /* Add cdev to kernel VFS*/
	cdev_add(&cdev, dev, 1);

    return 0;
}

static void __exit driver_exit(void){
    return;
}

module_init(driver_init);
module_exit(driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KARTHIK G K");
MODULE_DESCRIPTION("A SIMPLE DEVICE DRIVER");
