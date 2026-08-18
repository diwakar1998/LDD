#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device/class.h>
#include<linux/uaccess.h>
#include<linux/platform_device.h>
#include<linux/mod_devicetable.h>   
#include<linux/slab.h>  
#include<linux/of.h>
#include<linux/device.h>
#include<linux/gpio/consumer.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__






module_init();
module_exit();

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIKARI_NO_OMO");
MODULE_DESCRIPTION("");
MODULE_INFO(board,"BBB");