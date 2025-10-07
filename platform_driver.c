#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/kernel.h>

struct pcdev_private_data {
    struct platform_data *pdata;
    char* buffer;
    dev_t dev_num;
    struct cdev cdev;
};

struct driver_private_data {
    int total_devices;
    dev_t device_num_base;
    struct class *class_pcd;
    struct device *device_pcd;
};

// called when platform device is matched with driver
int probe(struct platform_device *pdev) {
    printk(KERN_INFO "Platform device probed: %s\n", pdev->name);
    return 0;
}

// called when platform device is removed
static int remove(struct platform_device *pdev) {
    printk(KERN_INFO "Platform device removed: %s\n", pdev->name);
    return 0;
}

struct platform_driver my_platform_driver = {
    .probe = probe,
    .remove = remove, // remove function pointer
    .driver = {
        .name = "my_platform_device", // same name as platform device
    }};

static __init int platform_device_driver_init(void) {
    int ret;

    ret = alloc_char_region(&driver_data.device_num_base, 0, 2, "pcd_devices");
    if (ret < 0) {
        printk(KERN_ALERT "Alloc char region failed\n");
        return ret;
    }
    // create class
    driver_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
    if (IS_ERR(driver_data.class_pcd)) {
        unregister_chrdev_region(driver_data.device_num_base, 2);
        printk(KERN_ALERT "Class creation failed\n");
        return PTR_ERR(driver_data.class_pcd);
    }
    // create device
    driver_data.device_pcd = device_create(driver_data.class_pcd, NULL, driver_data.device_num_base, NULL, "pcd_device");
    if (IS_ERR(driver_data.device_pcd)) {
        class_destroy(driver_data.class_pcd);
        unregister_chrdev_region(driver_data.device_num_base, 2);
        printk(KERN_ALERT "Device creation failed\n");
        return PTR_ERR(driver_data.device_pcd);
    }
    // register platform driver
    platform_driver_register(&my_platform_driver);
    printk(KERN_INFO "Platform device and driver example initialized\n");
    return 0;
}

static __exit void platform_device_driver_exit(void) {
    //platform device unregistration is handled in platform_device_setup.c
    platform_driver_unregister(&my_platform_driver);
    device_destroy(driver_data.class_pcd, driver_data.device_num_base);
    class_destroy(driver_data.class_pcd);
    unregister_chrdev_region(driver_data.device_num_base, 2);
    printk(KERN_INFO "Platform device and driver example exited\n");
}

module_init(platform_device_driver_init);
module_exit(platform_device_driver_exit);

MODULE_LICENSE("GPL");
MODULE_NAME("platform_device_setup");
MODULE_AUTHOR("KARTHIK G K");
MODULE_DESCRIPTION("A simple platform device and driver example");
