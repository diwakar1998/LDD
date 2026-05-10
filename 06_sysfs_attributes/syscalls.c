#include"platform_driver_sysfs.h"

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
    