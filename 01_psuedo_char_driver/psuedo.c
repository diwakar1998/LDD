#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device/class.h>
#include<linux/uaccess.h>


#define MEM_SIZE 512

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

char buffer[MEM_SIZE];

/* Forward declarations */
ssize_t psuedo_read (struct file *file_p, char __user *buff, size_t count, loff_t *f_pos);
ssize_t psuedo_write (struct file *file_p, const char __user *buff, size_t count, loff_t *f_pos);
int psuedo_open (struct inode *inode, struct file *file_p);
loff_t psuedo_lseek (struct file *file_p, loff_t off, int whence);
int psuedo_release (struct inode *inode, struct file *file_p);

ssize_t psuedo_read (struct file *file_p, char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Read was Requested for %zu bytes\n",count);
    pr_info("Current file position is %lld\n",*f_pos);
    // Adjust count size
    if(*f_pos + count > MEM_SIZE){
        count = MEM_SIZE - *f_pos;
    }
    // copy to user
    if(copy_to_user(buff,&buffer[*f_pos],count)){
        pr_err("Failed to copy data to user\n");
        return -EFAULT;
    }
    // Update file position
    *f_pos += count;
    pr_info("Number of bytes read: %zu\n", count);
    pr_info("Updated file position is %lld\n",*f_pos);
    return count; 
}
ssize_t psuedo_write (struct file *file_p, const char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Write was Requested for %zu bytes\n",count);
    pr_info("Current file position is %lld\n",*f_pos);
    // Adjust count size
    if(*f_pos + count > MEM_SIZE){
        count = MEM_SIZE - *f_pos;
    }

    if(!count ){
        pr_err("No space left in buffer to write data\n");
        return -ENOMEM;
    }
    // copy from user
    if(copy_from_user(&buffer[*f_pos],buff,count)){
        pr_err("Failed to copy data from user\n");
        return -EFAULT;
    }
    // Update file position
    *f_pos += count;

    pr_info("Number of bytes written: %zu\n", count);
    pr_info("Updated file position is %lld\n",*f_pos);
    return count; 
}
int psuedo_open (struct inode *inode, struct file *file_p){     
    pr_info("Open was succesful\n");
    return 0; 
}
loff_t psuedo_lseek (struct file *file_p, loff_t off, int whence){     
    pr_info("Seek was Requested\n");
    pr_info("Current file position is %lld\n",file_p->f_pos);
    switch(whence){
        case SEEK_SET:
            if(off < 0 || off > MEM_SIZE){
                pr_err("Invalid offset\n");
                return -EINVAL;
            }
            file_p->f_pos = off;
            break;
        case SEEK_CUR:
            if(file_p->f_pos + off < 0 || file_p->f_pos + off > MEM_SIZE){
                pr_err("Invalid offset\n");
                return -EINVAL;
            }
            file_p->f_pos += off;
            break;
        case SEEK_END:
            if(MEM_SIZE + off < 0 || MEM_SIZE + off > MEM_SIZE){
                pr_err("Invalid offset\n");
                return -EINVAL;
            }
            file_p->f_pos = MEM_SIZE + off;
            break;
        default:
            pr_err("Invalid whence\n");
            return -EINVAL;
    }
    pr_info("Updated file position is %lld\n",file_p->f_pos);
    return file_p->f_pos; 
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
    int ret = alloc_chrdev_region(&device_number,3,6,"psuedo_device_number");
    if(ret < 0){
        pr_err("Failed to allocate device number\n");
        goto alloc_error;
    } 
    pr_info("Device Numbers Maj: %d : Min: %d \n",MAJOR(device_number),MINOR(device_number));

    //2.Cdev Initialization with file operations
    cdev_init(&psuedo_cdev,&psuedo_fops);

    //3.Registration with Kernel Virtual File System
    ret = cdev_add(&psuedo_cdev,device_number,1);
    if(ret < 0){
        pr_err("Failed to add cdev to kernel\n");
        goto unregister_alloc;
    }
    psuedo_cdev.owner = THIS_MODULE;  

    //4.Creation of device files
    /*First create class of our device
    Create device that will create a dev file in the sysfs class folder
    Udev listens to uevents then will look for this sysfs folder and then 
    creates /dev/"devicename"--> this file in /dev directory
    this can be used in user space to communicate with driver
    */
    //Class is created in /sys/class 
    // class_psuedo = class_create(THIS_MODULE, "Psuedo_class");
    class_psuedo = class_create("Psuedo_class");    // Api is changed for WSL2 Kernel
    
    if(IS_ERR(class_psuedo)) {
        pr_err("Failed to create class\n");
        ret = PTR_ERR(class_psuedo);
        goto cdev_delete;
    }
    //Populate sysfs with device information                
    device_psuedo = device_create(class_psuedo,NULL,device_number,NULL,"psuedo");
                                                    //    |                         
                                                    //    V    
                                        //this name appears in /dev directory
    if(IS_ERR(device_psuedo)) {
        pr_err("Failed to create device\n");
        ret = PTR_ERR(device_psuedo);
        goto class_delete;
    }

    pr_info("Module Init successful\n");

    return 0;

class_delete:
    class_destroy(class_psuedo);
cdev_delete:
    cdev_del(&psuedo_cdev);
unregister_alloc:
    unregister_chrdev_region(device_number,6);
alloc_error:    
    return ret;
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
    pr_info("Module Exit successful\n");
}


module_init(psuedo_init);
module_exit(psuedo_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIKARI_NO_OMO");
MODULE_DESCRIPTION("Psuedo Character Driver");
MODULE_INFO(board,"BBB");