#include<linux/module.h>
#include<linux/platform_device.h>
#include "platform.h"

#undef pr_fmt
#define pr_fmt(fmt) "%s: " fmt, __func__

void psuedo_release(struct device *dev);
void psuedo_release(struct device *dev) {
    pr_info("Device released\n");
}

// 1. Create 2 platform data
struct psuedo_char_dev_platform_data psuedoDeviceData[2] =  {
    {
        .size = 512,
        .perm = RDONLY,
        .serial_number = "PSUEDO_CHAR_DEV_1"
    },
    {
        .size = 2048,
        .perm = RDWR,
        .serial_number = "PSUEDO_CHAR_DEV_2"
    }
};


//1. Create 2 platform devices

struct platform_device psuedoDevice1 = {
    .name = "psuedo_char_device",
    .id = 0,
    .dev = {
        .platform_data = &psuedoDeviceData[0],
        .release = psuedo_release
    }
};

struct platform_device psuedoDevice2 = {
    .name = "psuedo_char_device",
    .id = 1,
    .dev = {
        .platform_data = &psuedoDeviceData[1],
        .release = psuedo_release
    }
};


static int __init psuedoDevice_platform_init(void) {
    platform_device_register(&psuedoDevice1);
    platform_device_register(&psuedoDevice2);

    pr_info("2 platform devices registered\n");
    return 0;
}

static void __exit psuedoDevice_platform_exit(void) {
    platform_device_unregister(&psuedoDevice1);
    platform_device_unregister(&psuedoDevice2);
    pr_info("2 platform devices unregistered\n");
}


module_init(psuedoDevice_platform_init);
module_exit(psuedoDevice_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hikari_No_Omo");
MODULE_DESCRIPTION("Simple platform device registration\n");