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

struct psuedo_char_drv_data drv_data;

char buffer[MEM_SIZE];

/* Forward declarations */
ssize_t psuedo_read (struct file *file_p, char __user *buff, size_t count, loff_t *f_pos);
ssize_t psuedo_write (struct file *file_p, const char __user *buff, size_t count, loff_t *f_pos);
int psuedo_open (struct inode *inode, struct file *file_p);
loff_t psuedo_lseek (struct file *file_p, loff_t off, int whence);
int psuedo_release (struct inode *inode, struct file *file_p);

ssize_t psuedo_read (struct file *file_p, char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Read was Requested for %zu bytes\n",count);
    return 0;
}
ssize_t psuedo_write (struct file *file_p, const char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Write was Requested for %zu bytes\n",count);
    return 0;
}
int psuedo_open (struct inode *inode, struct file *file_p){     
    pr_info("Open was succesful\n");
    return 0; 
}
loff_t psuedo_lseek (struct file *file_p, loff_t off, int whence){     
    pr_info("Seek was Requested\n");
    return 0;
}
int psuedo_release (struct inode *inode, struct file *file_p){     
    pr_info("CLose was succesful\n");
    return 0; 
}
    
//1 Holds Device Number
dev_t device_number;

//2 cdev struct holds info of the driver
struct cdev psuedo_cdev;
//2 file operations structure holding info on which method is linked to which user call
struct file_operations psuedo_fops = {
    .open = psuedo_open,
    .read = psuedo_read,
    .write = psuedo_write,
    .release = psuedo_release,
    .llseek = psuedo_lseek,
    .owner = THIS_MODULE
};

struct class *class_psuedo;
struct device *device_psuedo;

int psuedo_platform_driver_probe(struct platform_device *pdev);
int psuedo_platform_driver_remove(struct platform_device *pdev);

/*
    Platform driver Part2,3,4 Pending
*/


struct psuedo_char_dev_platform_data * dev_get_platdata_from_dt(struct device *dev){
    struct device_node *dev_node = dev->of_node;
    struct psuedo_char_dev_platform_data *pdata;

    if(!dev_node){
        pr_err("No device node found\n");
        return NULL;
    }

    pdata = devm_kzalloc(dev,sizeof(*pata),GFP_KERNEL);

    if(!pdata){
        dev_info(dev,"Cannot allocate mempry/n");
        return ERR_PTR(-EINVAL);
    }
    if(of_properity_read_string(dev_node,"org,device_serial",&pdata->serial_number)){
        dev_info(dev,"Missing properity:Serial\n");
        return ERR_PTR(-EINVAL);
    }
    if(of_properity_read_string(dev_node,"org,size",&pdata->size)){
        dev_info(dev,"Missing properity:Size\n");
        return ERR_PTR(-EINVAL);
    }
    if(of_properity_read_string(dev_node,"org,perm",&pdata->perm)){
        dev_info(dev,"Missing properity:Perm\n");
        return ERR_PTR(-EINVAL);
    }
    return pdata;
}

int psuedo_platform_driver_probe(struct platform_device *pdev){
    
    int ret = 0;
    pr_info("Device is detected via probing\n");
    
    struct psuedo_char_dev_data *devdata;
    struct psuedo_char_dev_platform_data *pdata;
    pr_info("Probing for device\n");
    // 1 Get the platform data from the device tree
    pdata = (struct psuedo_char_dev_platform_data *)dev_get_platdata_from_dt(&pdev->dev);
    if(IS_ERR(pdata)){
        pr_err("Device setup not done from DT\n");
        ret = -EINVAL;
        goto exit;
    }
    // 1 If pdata is NULL it means device Setup is not done in DT, so get platform data from Device
    if(!pdata){
        pdata = (struct psuedo_char_dev_platform_data *)dev_get_platdata(&pdev->dev);
        if(!pdata){
            pr_info("No platform data available\n");
            ret = -EINVAL;
            goto exit;
        }
    }
    // 2 Dynamically allocate memory for device private data
    devdata = kzalloc(sizeof(struct psuedo_char_dev_data),GFP_KERNEL);
    if(!devdata){
        pr_err("Failed to allocate memory for device data\n");
        ret = -ENOMEM;
        goto exit;
    }

    devdata->pdata = pdata;
    
    dev_set_drvdata(&pdev->dev,devdata); // Link the device data with the platform device structure so that it can be accessed in other file operations methods using dev_get_drvdata()
    // 3 Allocate memory for the device buffer using size info from platform data
    devdata->buffer = kzalloc(devdata->pdata->size,GFP_KERNEL);
    if(!devdata->buffer){
        pr_err("Failed to allocate memory for device buffer\n");
        ret = -ENOMEM;
        goto free_devdata;
    }
    // 4 Get Device Number
    devdata->dev_num = drv_data.device_num_base + drv_data.total_devices;

    // 5 Do cdev init and add
    cdev_init(&devdata->psuedo_cdev,&psuedo_fops);
    devdata->psuedo_cdev.owner = THIS_MODULE;
    ret = cdev_add(&devdata->psuedo_cdev,devdata->dev_num,1);
    if(ret < 0){
        pr_err("Failed to add cdev\n");
        goto free_buffer;
    }
    // 6 Create device file for detected platform device    
    drv_data.device = device_create(drv_data.class,NULL,devdata->dev_num,NULL,"psuedo_char_dev_%d",drv_data.total_devices);
    if(IS_ERR(drv_data.device)){
        pr_err("Failed to create device file\n");
        ret = PTR_ERR(drv_data.device);
        goto del_cdev;
    }
    pr_info("Device loaded and Probed\n");
    drv_data.total_devices++;
    return 0;
    // 7 Error handling
del_cdev:
    cdev_del(&devdata->psuedo_cdev);
free_buffer:
    kfree(devdata->buffer);
free_devdata:
    kfree(devdata);
exit:
    pr_info("Device probed failed\n");
    return ret;
}

int psuedo_platform_driver_remove(struct platform_device *pdev){
    int ret = 0; 
    #if 0 
    struct psuedo_char_dev_data *devdata = dev_get_drvdata(&pdev->dev);
    // 1 Remove device file
    device_destroy(drv_data.class,devdata->dev_num);
    // 2 Delete cdev
    cdev_del(&devdata->psuedo_cdev);
    // 3 Free device buffer memory
    kfree(devdata->buffer);
    // 4 Free device data memory
    kfree(devdata);
    #endif
    pr_info("Device unloaded and Removed \n");
    return ret;
}

struct of_device_id org_psuedo_platform_driver_dt_match[] = {
    {.compatible = "psuedo-char-device-001"},
    {.compatible = "psuedo-char-device-002"},
    {.compatible = "psuedo-char-device-003"},
    {}
};

struct platform_driver psuedo_platform_driver = {
    .probe = psuedo_platform_driver_probe,
    .remove = psuedo_platform_driver_remove,
    .driver = {
        .name = "psuedo_char_device", // Uses this name to match with platform device name and binds driver to it
        .of_match_table = org_psuedo_platform_driver_dt_match, // Uses this table to match with compatible string in device tree and binds driver to it 
    },
}; 

#define MAX_DEVICES 10

static int __init psuedo_platform_driver_init(void)
{
    //1.Dynamically allocate a device number
    int ret = alloc_chrdev_region(&drv_data.device_num_base,0,MAX_DEVICES,"Platform_Psuedo_Char_Driver");
    if(ret < 0){
        pr_err("Failed to allocate device number\n");
        goto alloc_error;
    } 
    drv_data.total_devices = 0;
    pr_info("Device Numbers Maj: %d : Min: %d \n",MAJOR(drv_data.device_num_base),MINOR(drv_data.device_num_base));

    //2.Creation of Classs
    //Class is created in /sys/class 
    class_psuedo = class_create(THIS_MODULE, "Psuedo_class");
    // drv_data.class = class_create("Psuedo_class");    // Api is changed for WSL2 Kernel
    
    if(IS_ERR(drv_data.class)) {
        pr_err("Failed to create class\n");
        ret = PTR_ERR(drv_data.class);
        goto unregister_alloc;
    }

    //3. Register the platform driver
    ret = platform_driver_register(&psuedo_platform_driver);
    if(ret < 0){
        pr_err("Failed to register platform driver\n");
        goto class_delete;
    }

    pr_info("Platform driver registered\n");
    return 0;

class_delete:
    class_destroy(drv_data.class);
unregister_alloc:
    unregister_chrdev_region(drv_data.device_num_base, MAX_DEVICES);
alloc_error:    
    pr_info("Platform driver failed to load\n");
    return ret;

}

static void __exit psuedo_platform_driver_cleanup(void){
    //1.Unregister the platform driver
    platform_driver_unregister(&psuedo_platform_driver);
    pr_info("Platform driver unregistered\n");
    //Exit should be done in reverse order of init
    //2. Destroy class
    class_destroy(drv_data.class);
    //4.Unallocate the allocated major and minor numbers
    unregister_chrdev_region(drv_data.device_num_base, MAX_DEVICES);
}


module_init(psuedo_platform_driver_init);
module_exit(psuedo_platform_driver_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIKARI_NO_OMO");
MODULE_DESCRIPTION("Psuedo Platform Character Driver");
MODULE_INFO(board,"BBB");