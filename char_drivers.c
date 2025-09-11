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
loff_t pcd_llseek(struct file *filp, loff_t offset, int whence)
{
    // Implement the seek functionality here
    return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	// Implement the read functionality here
	return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	// Implement the write functionality here
	return len;
}

int pcd_release(struct inode *inode, struct file *filp)
{
	// Implement the release functionality here
	return 0;
}

int pcd_open(struct inode *inode, struct file *filp)
{
	// Implement the open functionality here
	return 0;
}

struct file_operations fops = {
	.llseek = pcd_llseek,
	.read = pcd_read,
	.write = pcd_write,
	.open = pcd_open,
};
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
	cdev_add(&cdev, dev_number, 1); // parameters are cdev pointer, device number and count of device numbers

	/* create class in  sys/class which will be read by udevd deamon and dynamically device file is created in /dev */
	class_create(THIS_MODULE, "my_class");

	/* create device file in /dev */
	device_create(my_class, NULL, dev_number, NULL, "my_device");

	print_info("Device driver inserted successfully\n");

    return 0;
}

static void __exit driver_exit(void){
	/* remove device file from /dev */
	device_destroy(my_class, dev_number);
	/* remove class from sys/class */
	class_destroy(my_class);
	/* remove cdev from kernel */
	cdev_del(&cdev);
	/* unregister device number */
	unregister_chrdev_region(dev_number, 1);
	printk(KERN_INFO "Device driver removed successfully\n");
    return;
}

module_init(driver_init);
module_exit(driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KARTHIK G K");
MODULE_DESCRIPTION("A SIMPLE DEVICE DRIVER");
