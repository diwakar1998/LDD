#ifndef PLATFORM_DRIVER_SYSFS_H
#define PLATFORM_DRIVER_SYSFS_H

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device/class.h>
#include<linux/uaccess.h>
#include<linux/platform_device.h>
#include<linux/mod_devicetable.h>   
#include "platform.h"
#include<linux/slab.h>  
#include <linux/of.h>
#include <linux/device.h>
#define MEM_SIZE 512

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

// Holds device private data
struct psuedo_char_dev_data{
    struct psuedo_char_dev_platform_data *pdata;
    char * buffer;
    dev_t dev_num;
    struct cdev psuedo_cdev;
};

// Holds driver private data
struct psuedo_char_drv_data{
    int total_devices;
    dev_t device_num_base; // Base device number for the driver
    struct class *class;
    struct device *device;
};

/* Forward declarations */
ssize_t psuedo_read (struct file *file_p, char __user *buff, size_t count, loff_t *f_pos);
ssize_t psuedo_write (struct file *file_p, const char __user *buff, size_t count, loff_t *f_pos);
int psuedo_open (struct inode *inode, struct file *file_p);
loff_t psuedo_lseek (struct file *file_p, loff_t off, int whence);
int psuedo_release (struct inode *inode, struct file *file_p);

int psuedo_platform_driver_probe(struct platform_device *pdev);
int psuedo_platform_driver_remove(struct platform_device *pdev);


#endif