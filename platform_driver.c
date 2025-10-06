#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/kernel.h>

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
    platform_driver_register(&my_platform_driver);
    printk(KERN_INFO "Platform device and driver example initialized\n");
    return 0;
}

static __exit void platform_device_driver_exit(void) {
    platform_driver_unregister(&my_platform_driver);
    printk(KERN_INFO "Platform device and driver example exited\n");
}

module_init(platform_device_driver_init);
module_exit(platform_device_driver_exit);

MODULE_LICENSE("GPL");
MODULE_NAME("platform_device_setup");
MODULE_AUTHOR("KARTHIK G K");
MODULE_DESCRIPTION("A simple platform device and driver example");
