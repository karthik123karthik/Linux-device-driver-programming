#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/kernel.h>

struct platform_data {
    int size;
    char* serial_number;
    int perm; // permission
};

struct platform_data platform_datas [] = {
    [0] = {
        .size = 1024,
        .serial_number = "PCD123456",
        .perm = 0x10 // read only
    },
    [1] = {
        .size = 2048,
        .serial_number = "PCD123457",
        .perm = 0x01 // write only
    }
};

struct platform_device my_platform_device1 = {
    .name = "my_platform_device",
    .id = 0,
    .dev = {
        .platform_data = &platform_datas[0],
        .release = NULL
    }
};

struct platform_device my_platform_device2 = {
    .name = "my_platform_device",
    .id = 1,
    .dev = {
        .platform_data = &platform_datas[1],
        .release = NULL
    }
};


static int __init platform_device_init(void){
    printk(KERN_INFO "Platform Device Module Init\n");
    platform_device_register(&my_platform_device1);
    platform_device_register(&my_platform_device2);
    return 0;
}

static void __exit platform_device_exit(void){
    printk(KERN_INFO "Platform Device Module Exit\n");
    platform_device_unregister(&my_platform_device1);
    platform_device_unregister(&my_platform_device2);
    return;
}

module_init(platform_device_init);
module_exit(platform_device_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Platform Device Example");
