#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device/class.h>
#include<linux/uaccess.h>

#include<linux/interrupt.h>

#define MEM_SIZE 512

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

char buffer[MEM_SIZE];

/* Forward declarations */
ssize_t logger_read (struct file *file_p, char __user *buff, size_t count, loff_t *f_pos);
ssize_t logger_write (struct file *file_p, const char __user *buff, size_t count, loff_t *f_pos);
int logger_open (struct inode *inode, struct file *file_p);
loff_t logger_lseek (struct file *file_p, loff_t off, int whence);
int logger_release (struct inode *inode, struct file *file_p);

ssize_t logger_read (struct file *file_p, char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Read was Requested for %zu bytes\n",count);
    #if 0
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
    #endif
    return 0;
}
ssize_t logger_write (struct file *file_p, const char __user *buff, size_t count, loff_t *f_pos){     
    pr_info("Write was Requested for %zu bytes\n",count);
    #if 0
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
    #endif
    return 0;
}
int logger_open (struct inode *inode, struct file *file_p){     
    pr_info("Open was succesful\n");
    return 0; 
}
loff_t logger_lseek (struct file *file_p, loff_t off, int whence){     
    pr_info("Seek was Requested\n");
    #if 0
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
    #endif
    return 0;
}
int logger_release (struct inode *inode, struct file *file_p){     
    pr_info("CLose was succesful\n");
    return 0; 
}
 
void logger_tasklet(long unsigned int);

struct tasklet_struct *logger_tasklet_struct;

void logger_tasklet(long unsigned int data){
    pr_info("Tasklet called after top half\n");
}
//Interrupt Service Routine for handling interrupts from keyboard
// This will be called when an interrupt is generated from the keyboard
// We will read the scancode from the keyboard and store it in a buffer
// We will also print the scancode to the kernel log
// We will also implement a mechanism to read the scancode from the buffer in user space
#define IRQ_KEYBOARD 1
#define BUFFER_SIZE 1024

char scancode_buffer[BUFFER_SIZE];
int buffer_index = 0;

//For now increment counter and print the count value on kernel log
int keyboard_interrupt_count = 0;
// Used to identify handler on the IRQ line
static int keyboard_logger_id = 0; 

static irqreturn_t keyboard_isr(int irq, void *dev_id){
    // Top half of the interrupt handler
    keyboard_interrupt_count++;
    pr_info("Keyboard Interrupt Occurred. Count: %d\n",keyboard_interrupt_count);

    // Bottom half to be implemented
    tasklet_schedule(logger_tasklet_struct);
    return IRQ_HANDLED;
}


//1 Holds Device Number
dev_t device_number;

//2 cdev struct holds info of the driver
struct cdev logger_cdev;
//2 file operations structure holding info on which method is linked to which user call
struct file_operations logger_fops = {
    .open = logger_open,
    .read = logger_read,
    .write = logger_write,
    .release = logger_release,
    .llseek = logger_lseek,
    .owner = THIS_MODULE
};

struct class *class_logger;
struct device *device_logger;

static int __init logger_init(void)
{
    //1.Dynamically allocate a device number
    int ret = alloc_chrdev_region(&device_number,3,6,"logger_device_number");
    if(ret < 0){
        pr_err("Failed to allocate device number\n");
        goto alloc_error;
    } 
    pr_info("Device Numbers Maj: %d : Min: %d \n",MAJOR(device_number),MINOR(device_number));

    //2.Cdev Initialization with file operations
    cdev_init(&logger_cdev,&logger_fops);

    //3.Registration with Kernel Virtual File System
    ret = cdev_add(&logger_cdev,device_number,1);
    if(ret < 0){
        pr_err("Failed to add cdev to kernel\n");
        goto unregister_alloc;
    }
    logger_cdev.owner = THIS_MODULE;  

    //4.Creation of device files
    /*First create class of our device
    Create device that will create a dev file in the sysfs class folder
    Udev listens to uevents then will look for this sysfs folder and then 
    creates /dev/"devicename"--> this file in /dev directory
    this can be used in user space to communicate with driver
    */
    //Class is created in /sys/class 
    // class_logger = class_create(THIS_MODULE, "Logger_class");
    class_logger = class_create("Logger_class");    // Api is changed for WSL2 Kernel
    
    if(IS_ERR(class_logger)) {
        pr_err("Failed to create class\n");
        ret = PTR_ERR(class_logger);
        goto cdev_delete;
    }
    //Populate sysfs with device information                
    device_logger = device_create(class_logger,NULL,device_number,NULL,"logger");
                                                    //    |                         
                                                    //    V    
                                        //this name appears in /dev directory
    if(IS_ERR(device_logger)) {
        pr_err("Failed to create device\n");
        ret = PTR_ERR(device_logger);
        goto class_delete;
    }

    // Request IRQ
    // Is shared because keyboard has isr already implemented by the kernel 
    // and we are just adding another handler for the same interrupt line
    ret = request_irq(IRQ_KEYBOARD,keyboard_isr,IRQF_SHARED,"keyboard_logger",(void *)&keyboard_logger_id);
    if(ret < 0) {
        pr_info("Failed to request IRQ for keyboard logger\n");
        goto irq_error;
    }

    // Allocating memory to tasklet structure
    logger_tasklet_struct = kmalloc(sizeof(struct tasklet_struct),GFP_KERNEL);
    if(!logger_tasklet_struct){
        pr_info("Cannot allocate memory to tasklet\n");
        goto irq_error;
    }

    // Initialize the tasklet
    tasklet_init(logger_tasklet_struct,logger_tasklet,0);
 
    pr_info("Module Init successful\n");

    return 0;
irq_error:
    device_destroy(class_logger,device_number);
class_delete:
    class_destroy(class_logger);
cdev_delete:
    cdev_del(&logger_cdev);
unregister_alloc:
    unregister_chrdev_region(device_number,6);
alloc_error:    
    return ret;
}

static void __exit logger_cleanup(void){
    // Free the Registered IRQ
    free_irq(IRQ_KEYBOARD,(void *)&keyboard_logger_id);
    //Exit should be done in reverse order of init
    //4. Destroy device
    device_destroy(class_logger,device_number);
    //4. Destroy class
    class_destroy(class_logger);

    //2.cdev removal
    cdev_del(&logger_cdev);

    //1.Unallocate the allocated major and minor numbers
    unregister_chrdev_region(device_number,6);
    pr_info("Module Exit successful\n");
}


module_init(logger_init);
module_exit(logger_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIKARI_NO_OMO");
MODULE_DESCRIPTION("Keyboard Logger Driver");
MODULE_INFO(board,"BBB");