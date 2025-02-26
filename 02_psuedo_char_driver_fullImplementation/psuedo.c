#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__
    
//Macros of sizes of different devices
#define MEM_SIZE_DEV1 1024
#define MEM_SIZE_DEV2 512
#define MEM_SIZE_DEV3 1024
#define MEM_SIZE_DEV4 512
#define NUM_OF_DEVICES 4

//Memory buffers in stack
char buffer_dev1[MEM_SIZE_DEV1];
char buffer_dev2[MEM_SIZE_DEV2];
char buffer_dev3[MEM_SIZE_DEV3];
char buffer_dev4[MEM_SIZE_DEV4];
struct psuedo_dev_data{
    char *buffer;
    unsigned size;
    const char *serial_number;
    int perm;
    //2 cdev struct holds info of the driver
    struct cdev cdev;
};


struct psuedo_driver_data{
    int totalDevices;
    //1 Holds Device Number
    dev_t device_number;
    struct class *class_psuedo;
    struct device *device_psuedo;
    struct psuedo_dev_data psuedo_devices[NUM_OF_DEVICES];
};


//Initializing the driver data with default values
struct psuedo_driver_data driver_data = 
{
    .totalDevices = NUM_OF_DEVICES,
    .psuedo_devices = {
        [0] = {
            .buffer = buffer_dev1,
            .size = MEM_SIZE_DEV1,
            .serial_number = "PCDEV1",
            .perm = 0x01 //RDONLY
        },
        [1] = {
            .buffer = buffer_dev2,
            .size = MEM_SIZE_DEV2,
            .serial_number = "PCDEV2",
            .perm = 0x10 //WRONLY
        },
        [2] = {
            .buffer = buffer_dev3,
            .size = MEM_SIZE_DEV3,
            .serial_number = "PCDEV3",
            .perm = 0x11 //RDWR
        },
        [3] = {
            .buffer = buffer_dev4,
            .size = MEM_SIZE_DEV4,
            .serial_number = "PCDEV4",
            .perm = 0x11 //RDWR
        },
    }
};

ssize_t psuedo_read (struct file *file_p, char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Read was Requested for %d bytes\n",count);
    
    return 0; 
}
ssize_t psuedo_write (struct file *file_p, const char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Write was Requested for %d bytes\n",count);
    return 0; 
}

int check_permission(void);

int check_permission(){
    return 0;//Access for both read and write
}

int psuedo_open (struct inode *inode, struct file *file_p){    
    int ret;
    int minor_n;
    struct psuedo_dev_data *dev_data;

    //figuring out which device is calling open
    minor_n = MINOR(inode->i_rdev);
    //gets device's private data structure
    dev_data = container_of(inode->i_cdev,struct psuedo_dev_data,cdev);

    //saving the pointer in private field of file struct
    file_p->private_data = dev_data;

    //checkpermisson
    ret = check_permission();
    (!ret) ? pr_info("Open was succesful\n") : pr_info("File Open Failed\n") ;
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

//2 file operations structure holding info on which method is linked to which user call
struct file_operations psuedo_fops = {
    .open = psuedo_open,
    .read = psuedo_read,
    .write = psuedo_write,
    .release = psuedo_release,
    .llseek = psuedo_lseek,
    .owner = THIS_MODULE
};

static int i=0;

static int __init psuedo_init(void)
{
    //1.Dynamically allocate a device number
    int ret = alloc_chrdev_region(&driver_data.device_number,0,NUM_OF_DEVICES,"psuedo_device_numbers");
    
    if(ret < 0) {   
        pr_err("Alloc chrdev failed\n");  
        goto failed;
    }

    for(i=0;i<NUM_OF_DEVICES;i++){
        pr_info("Device Numbers Maj: %d : Min: %d \n",MAJOR(driver_data.device_number+i),MINOR(driver_data.device_number+i));
    }

    /*
    struct psuedo_driver_data{
    int totalDevices;
    //1 Holds Device Number
    dev_t device_number;
    struct class *class_psuedo;
    struct device *device_psuedo;
    struct psuedo_dev_data psuedo_devices[NUM_OF_DEVICES];
    };
    */

    //Class is created in /sys/class 
    driver_data.class_psuedo = class_create(THIS_MODULE, "Psuedo_class");
    //Error handling
    if(IS_ERR(driver_data.class_psuedo)){
        pr_err("Error occured at class creation\n");
        ret = PTR_ERR(driver_data.class_psuedo);
        goto unreg_chrdev;
    }
    
    //2,3,4 steps
    for(i=0;i<NUM_OF_DEVICES;i++){
        //2.Cdev Initialization with file operations
        cdev_init(&driver_data.psuedo_devices[i].cdev,&psuedo_fops);
        //3.Registration with Kernel Virtual File System
        ret = cdev_add(&driver_data.psuedo_devices[i].cdev,driver_data.device_number+i,1);
        driver_data.psuedo_devices[i].cdev.owner = THIS_MODULE;    

        if(ret < 0){
            pr_err("Error at cdev add %d",i);
            goto class_destroy;
        }
        //4.Creation of device files
        /*First create class of our device
        Create device that will create a dev file in the sysfs class folder
        Udev listens to uevents then will look for this sysfs folder and then 
        creates /dev/"devicename"--> this file in /dev directory
        this can be used in user space to communicate with driver
        */
        //Populate sysfs with device information                
        driver_data.device_psuedo = device_create(driver_data.class_psuedo,NULL,driver_data.device_number+i,NULL,"psuedo %d",i);
                                                                        //    |                         
                                                                        //    V    
                                                                    //this name appears in /dev directory

        if(IS_ERR(driver_data.device_psuedo)){
            pr_err("Error at device create %d\n",i);
            ret = PTR_ERR(driver_data.device_psuedo);
            goto device_destroy;
        }
    }

    pr_info("Module Init successful\n");
    return 0;
class_destroy:
device_destroy:
    //2.cdev removal
    for(i =0;i<NUM_OF_DEVICES;i++){
        //4. Destroy device
        device_destroy(driver_data.class_psuedo,driver_data.device_number+i);
        cdev_del(&driver_data.psuedo_devices[i].cdev);
    }
    pr_info("Unallocated cdevs\n");

    //4. Destroy class
    class_destroy(driver_data.class_psuedo);
    pr_info("Unallocated class\n");

unreg_chrdev:
    unregister_chrdev_region(driver_data.device_number,NUM_OF_DEVICES);

failed:
    //1.Unallocate the allocated major and minor numbers
    pr_info("Module Insertion Failed\n");

    return ret; 
}

static void __exit psuedo_cleanup(void){
    //Exit should be done in reverse order of init
    //2.cdev removal //3.device removal
    for(i =0;i<NUM_OF_DEVICES;i++){
        device_destroy(driver_data.class_psuedo,driver_data.device_number+i);
        cdev_del(&driver_data.psuedo_devices[i].cdev);
    }
    pr_info("Unallocated cdevs\n");

    //4. Destroy class
    class_destroy(driver_data.class_psuedo);
    pr_info("Unallocated class\n");

    //1.Unallocate the allocated major and minor numbers
    unregister_chrdev_region(driver_data.device_number,NUM_OF_DEVICES);
    pr_info("Unallocated device numbers\n");
   
    pr_info("Driver uninitialised successful\n");
}


module_init(psuedo_init);
module_exit(psuedo_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIKARI_NO_OMO");
MODULE_DESCRIPTION("Psuedo Character Driver that handles n devices");
MODULE_INFO(board,"BBB");