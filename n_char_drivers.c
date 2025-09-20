#include <linux/cdev.h>
#include <linux/uaccess.h> // for copy_to_user and copy_from_user
#include <linux/module.h>

#define NUM_OF_DEVICES 3

#define DEV1_SIZE 1024
#define DEV2_SIZE 2048
#define DEV3_SIZE 4096

#define RDONLY 0x10
#define WRONLY 0x01
#define RDWR   0x11



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
            .perm = RDONLY
        },
        [1] = {
            .buffer = device2_buffer,
            .size = DEV2_SIZE,
            .serial_number = "PCD123457",
            .perm = WRONLY
        },
        [2] = {
            .buffer = device3_buffer,
            .size = DEV3_SIZE,
            .serial_number = "PCD123458",
            .perm = RDWR
        }
    }
};

int check_permission(int dev_perm, int file_perm){
    if(dev_perm == RDWR)return 0;
    if((dev_perm == RDONLY) && (file_perm & FMODE_READ) && !(file_perm & FMODE_WRITE)) return 0;
        return 0;
    if((dev_perm == WRONLY) && (file_perm & FMODE_WRITE) && !(file_perm & FMODE_READ)) return 0;
        return 0;

    return -EPERM;
}

int pcd_open(struct inode *inode, struct file *file){
    int ret;
    int minor_n;
    struct dev_private_date *dev_data;

    minor_n = MINOR(inode->i_rdev);
    printk(KERN_INFO "Device Opened: %d\n", minor_n);

    /*Get the device's private data structure */
    dev_data = container_of(inode->i_cdev, struct dev_private_date, cdev);
    /*Save the device's private data structure in the file's private data field */
    file->private_data = dev_data;

    ret = check_permission(dev_data->perm, file->f_mode);

    if(ret!=0){
        printk(KERN_INFO "FILE Open failed\n");
    }else
        printk(KERN_INFO "FILE Open Successful\n");

    return ret;
}

ssize_t pcd_read(struct file *file, char __user *buf, size_t len, loff_t *offset){
    printk(KERN_INFO "Read requested for %zu bytes\n", len);
    struct dev_private_date *dev_data = (struct dev_private_date*)file->private_data;
    if(*offset >= dev_data->size) // check if offset is beyond device buffer size
        return 0; // end of file
    
    if(*offset + len > dev_data->size) // adjust len if it exceeds buffer size
        len = dev_data->size - *offset;

    if(copy_to_user(buf, dev_data->buffer + *offset, len)){
        return -EFAULT;
    }

    *offset += len;
    return len;
}

ssize_t pcd_write(struct file *file, const char* __user buffer, ssize_t len, loff_t *offset){
    printk(KERN_INFO "WRITE requested for %zu bytes\n", len);
    struct dev_private_date *dev_data = (struct dev_private_date*)file->private_data;
    if(*offset >= dev_data->size) // check if offset is beyond device buffer size
        return 0; // end of file
    
    if(*offset + len > dev_data->size) // adjust len if it exceeds buffer size
        len = dev_data->size - *offset;
    
    if(copy_from_user(dev_data->buffer + *offset, buffer, len)){
        return -EFAULT;
    }
    *offset += len;
    return len;
}

loff_t pcd_lseek(struct file *file, loff_t offset, int whence){
    loff_t new_pos = 0;
    struct dev_private_date *dev_data = (struct dev_private_date*)file->private_data;

    switch(whence){
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = file->f_pos + offset;
            break;
        case SEEK_END:
            new_pos = dev_data->size + offset;
            break;
        default:
            return -EINVAL;
    }

    if(new_pos < 0 || new_pos > dev_data->size)
        return -EINVAL;

    file->f_pos = new_pos;
    return new_pos;
}

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



