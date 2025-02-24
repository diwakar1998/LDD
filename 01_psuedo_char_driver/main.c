#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>

#define MEM_SIZE 512

char buffer[MEM_SIZE];

//1 Holds Device Number
dev_t device_number;

//2 cdev struct holds info of the driver
struct cdev psuedo_cdev;
//2 file operations structure holding info on which method is linked to which user call
struct file_operations psuedo_fops;

static int __init psuedo_init(void)
{
    //1.Dynamically allocate a device number
    alloc_chrdev_region(&device_number,3,6,"psuedo_d");

    //2.Cdev Initialization with file operations
    cdev_init(&psuedo_cdev,&psuedo_fops);

    //3.Registration with Kernel Virtual File System
    cdev_add(&psuedo_cdev,device_number,1);
    psuedo_cdev.owner = THIS_MODULE;  

    
    return 0;
}

static void __exit psuedo_cleanup(void){

}


module_init(psuedo_init);
module_exit(psuedo_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIKARI_NO_OMO");
MODULE_DESCRIPTION("Psuedo Character Driver");
MODULE_INFO(board,"BBB");