#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>

#define MEM_SIZE 512

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

char buffer[MEM_SIZE];

ssize_t psuedo_read (struct file *file_p, char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Read was Requested for %d bytes\n",count);
    return 0; 
}
ssize_t psuedo_write (struct file *file_p, const char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Write was Requested for %d bytes\n",count);
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

static int __init psuedo_init(void)
{
    //1.Dynamically allocate a device number
    alloc_chrdev_region(&device_number,3,6,"psuedo_device_number");

    pr_info("Device Numbers Maj: %d : Min: %d \n",MAJOR(device_number),MINOR(device_number));

    //2.Cdev Initialization with file operations
    cdev_init(&psuedo_cdev,&psuedo_fops);

    //3.Registration with Kernel Virtual File System
    cdev_add(&psuedo_cdev,device_number,1);
    psuedo_cdev.owner = THIS_MODULE;  

    //4.Creation of device files
    /*First create class of our device
    Create device that will create a dev file in the sysfs class folder
    Udev listens to uevents then will look for this sysfs folder and then 
    creates /dev/"devicename"--> this file in /dev directory
    this can be used in user space to communicate with driver
    */
    //Class is created in /sys/class 
    class_psuedo = class_create(THIS_MODULE, "Psuedo_class");
    
    //Populate sysfs with device information                
    device_psuedo = device_create(class_psuedo,NULL,device_number,NULL,"psuedo");
                                                    //    |                         
                                                    //    V    
                                        //this name appears in /dev directory

    pr_info("Module Init successful\n");
    return 0;
}

static void __exit psuedo_cleanup(void){
    //Exit should be done in reverse order of init
    //4. Destroy device
    device_destroy(class_psuedo,device_number);
    //4. Destroy class
    class_destroy(class_psuedo);

    //2.cdev removal
    cdev_del(&psuedo_cdev);

    //1.Unallocate the allocated major and minor numbers
    unregister_chrdev_region(device_number,6);
}


module_init(psuedo_init);
module_exit(psuedo_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIKARI_NO_OMO");
MODULE_DESCRIPTION("Psuedo Character Driver");
MODULE_INFO(board,"BBB");