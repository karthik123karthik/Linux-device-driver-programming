#include <linux/cdev.h>
#include <linux/uaccess.h> // for copy_to_user and copy_from_user
#include <linux/module.h>

#define NUM_OF_DEVICES 3

#define DEV1_SIZE 1024
#define DEV2_SIZE 2048
#define DEV3_SIZE 4096


char device1_buffer[DEV1_SIZE];
char device2_buffer[DEV2_SIZE];
char device3_buffer[DEV3_SIZE];

struct dev_private_date {
    char* buffer;
    size_t size;
    char* serial_number;
    int perm;
    cdev_t cdev;
};

struct drv_private_data{
    size_t total_devices;
    dev_t device_number;
    struct class *class_pcd;
    struct device *device_pcd;
    struct dev_private_date device_data[NUM_OF_DEVICES];
};

struct drv_private_data drv_data = {
    .total_devices = NUM_OF_DEVICES,
    .device_data   = {
        [0] = {
            .buffer = device1_buffer,
            .size = DEV1_SIZE,
            .serial_number = "PCD123456",
            .perm = 0x1
        },
        [1] = {
            .buffer = device2_buffer,
            .size = DEV2_SIZE,
            .serial_number = "PCD123457",
            .perm = 0x10
        },
        [2] = {
            .buffer = device3_buffer,
            .size = DEV3_SIZE,
            .serial_number = "PCD123458",
            .perm = 0x11
        }
    }
};

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read  = pcd_read,
    .write = pcd_write,
    .llseek = pcd_llseek,
    .open  = pcd_open,
    .release = pcd_release
};

static int __init driver_init(void){
    int i;
    int ret;
    printk(KERN_INFO "Driver Init\n");
    /*Assign a device number*/
    ret = alloc_chrdev_region(&drv_data.device_number, 0, drv_data.total_devices, "my_device");
    if (ret<0){
        printk(KERN_INFO "Device Number allocation failed\n");
        goto out;
    }

    /*create device class under sys/class */
    drv_data.class_pcd = class_create(THIS_MODULE, "my_class");
    if (IS_ERR(drv_data.class_pcd)) {
        printk(KERN_INFO "Class creation failed\n");
        ret = PTR_ERR(drv_data.class_pcd);
        goto unreg_chrdev;
    }

    for(i=0; i<drv_data.total_devices; i++){
        printk(KERN_INFO "Device %d: Major: %d, Minor: %d\n", i, MAJOR(drv_data.device_number), MINOR(drv_data.device_number)+i);        /* Initialize cdev structure with fops */
        /* Initialize cdev structure with fops */
        cdev_init(&drv_data.device_data[i].cdev, &fops);
        drv_data.device_data[i].cdev.owner = THIS_MODULE;
        /* Register a device (cdev structure) with VFS */
        ret = cdev_add(&drv_data.device_data[i].cdev, drv_data.device_number + i, 1);
        if (ret<0){
            printk(KERN_INFO "cdev_add failed for device %d\n", i);
            goto cdev_del;
        }
        printk(KERN_INFO "Device %d added to the system\n", i);
        /* Create device file for the device */
        drv_data.device_pcd = device_create(drv_data.class_pcd, NULL, drv_data.device_number + i, NULL, "my_device%d", i);

        if (IS_ERR(drv_data.device_pcd)) {
            printk(KERN_INFO "Device creation failed for device %d\n", i);
            ret = PTR_ERR(drv_data.device_pcd);
            goto dev_del;
        }
    }

    cdev_del:
    dev_del:
        while (i--) {
            device_destroy(drv_data.class_pcd, drv_data.device_number + i);
            cdev_del(&drv_data.device_data[i].cdev);
        }
        class_destroy(drv_data.class_pcd);
    unreg_chrdev:
        unregister_chrdev_region(drv_data.device_number, drv_data.total_devices);
    out :
        return ret;
} 

static void __exit driver_exit(void){
    int i;
    printk(KERN_INFO "Driver Exit\n");

    for(i=0; i<drv_data.total_devices; i++){
        device_destroy(drv_data.class_pcd, drv_data.device_number + i);
        cdev_del(&drv_data.device_data[i].cdev);
    }
    class_destroy(drv_data.class_pcd);
    unregister_chrdev_region(drv_data.device_number, drv_data.total_devices);
    return;
}


module_init(driver_init);
module_exit(driver_exit);


MODULE_DESCRIPTION("A pseudo character driver which handles multiple devices");
MODULE_AUTHOR("KARTHIK G K");
MODULE_LICENSE("GPL");



