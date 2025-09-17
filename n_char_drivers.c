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
    int perm;
    cdev_t cdev;
};

struct drv_private_data{
    size_t total_devices;
    struct dev_private_date device_data[NUM_OF_DEVICES];
};



