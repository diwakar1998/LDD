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

// Holds device private data
struct gpio_dev_data{
    char label[20];
    struct gpio_desc *des;
};

// Holds driver private data
struct gpio_drv_data{
    int total_devices;
    struct class *class_gpio;
    struct device **dev;
};

struct gpio_drv_data gpio_drv;


ssize_t direction_show(struct device *dev, struct device_attribute *attr,char *buf)
{ 
    char *direction;
    struct gpio_dev_data *device_data = dev_get_drvdata(dev);
    int dir = gpiod_get_direction(device_data->des);
    if(dir < 0) return dir;
    direction = (dir) ? "in" : "out";
    return sprintf(buf,"%s\n",direction);
}
ssize_t direction_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{ 
    int ret = 0;
    struct gpio_dev_data *device_data = dev_get_drvdata(dev);
    if(sysfs_streq(buf, "in"))
        ret  = gpiod_direction_input(device_data->des);
    else if(sysfs_streq(buf, "out"))
        ret  = gpiod_direction_output(device_data->des,0);
    else
        ret = -EINVAL;
    return ret ? : count;
}

ssize_t value_show(struct device *dev, struct device_attribute *attr,char *buf)
{ 
    struct gpio_dev_data *device_data = dev_get_drvdata(dev);
    int value = gpiod_get_value(device_data->des);
    return sprintf(buf,"%d\n",value);
}
ssize_t value_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{ 
    struct gpio_dev_data *device_data = dev_get_drvdata(dev);
    int ret;
    long value;
    ret = kstrtol(buf, 0, &value);
    if(ret) return ret;
    gpiod_set_value(device_data->des, value);
    return count;
}

ssize_t label_show(struct device *dev, struct device_attribute *attr,char *buf)
{ 
    struct gpio_dev_data *device_data = dev_get_drvdata(dev);
    return sprintf(buf,"%s\n",device_data->label);
}

static DEVICE_ATTR_RW(direction);
static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RO(label);

static struct attribute *gpio_attr[] = 
{
    &dev_attr_direction.attr,
    &dev_attr_value.attr,
    &dev_attr_label.attr,
    NULL
}; 

static struct attribute_group gpio_attr_grp = { .attrs = gpio_attr };

const static struct attribute_group *gpio_attr_grps[] = {
    &gpio_attr_grp,
    NULL
};


int gpio_remove(struct platform_device *pdev);
int gpio_probe(struct platform_device *pdev);

int gpio_probe(struct platform_device *pdev){
    struct device *dev = &pdev->dev;
    const char *name;
    
    struct device_node *parent = pdev->dev.of_node;
    struct device_node *child = NULL;

    struct gpio_dev_data *device_data;
    int i = 0,ret = 0;

    pr_info("Inside gpio Probe\n");
    
    gpio_drv.total_devices = of_get_child_count(parent);
    if(gpio_drv.total_devices == 0){
        dev_err(dev,"No devices found\n");
        return -EINVAL;
    }

    dev_info(dev,"Child nodes count: %d\n",gpio_drv.total_devices);

    gpio_drv.dev = devm_kzalloc(dev, sizeof(struct device *)*gpio_drv.total_devices, GFP_KERNEL);

    for_each_available_child_of_node(parent, child)
    {
        // Getting label from DTS nodes using helper functions
        device_data = devm_kzalloc(&pdev->dev, sizeof(*device_data), GFP_KERNEL);
        if(!device_data){
            dev_err(dev,"Cannot allocate memory\n");
            return -ENOMEM;
        }
        // Reading label of each child node, to find which GPIO it is and pin also
        if(of_property_read_string(child,"label",&name)){
            dev_warn(dev,"Missing label info\n");
            snprintf(device_data->label,sizeof(device_data->label),"unknowngpio%d",i);
        }
        else{
            strcpy(device_data->label,name);
            dev_info(dev,"GPIO label = %s\n",device_data->label);
        }

        // Getting descriptor from dev node
        device_data->des = devm_fwnode_get_gpiod_from_child(dev, "bone", 
                &child->fwnode, GPIOD_ASIS,device_data->label);
        if(IS_ERR(device_data->des)){
            ret = PTR_ERR(device_data->des);
            if(ret == -ENOENT){
                dev_err(dev, "No GPIO has been assigned to req func\n");
                return ret;
            }
        }

        //Set GPIO Direction to output using the obtained descriptor
        ret = gpiod_direction_output(device_data->des, 0);
        if(ret){
            dev_err(dev, "GPIO Direction set failed\n");
            return ret;            
        }

        // Step 2 Create devices for each gpio pin
        gpio_drv.dev[i] = device_create_with_groups(gpio_drv.class_gpio, dev, 0, (void *)device_data, gpio_attr_grps, device_data->label);
        i++;

    }
    return 0;
}

int gpio_remove(struct platform_device *pdev){
    int i;
    dev_info(&pdev->dev,"Inside gpio remove\n");
    for(i = 0 ; i < gpio_drv.total_devices; i++){
        device_unregister(gpio_drv.dev[i]);
    }
    return 0;
}

struct of_device_id gpio_device_match[] = {
    {.compatible = "org,bone-gpio-sysfs"},
    { }
};

// Registering this driver as platform driver
struct platform_driver gpio_platform_driver = {
    .probe = gpio_probe,
    .remove = gpio_remove,
    .driver = {
        .name = "hikari gpios",
        .of_match_table = of_match_ptr(gpio_device_match)
    }
};

static int __init gpio_sysfs_init(void){
    // Step 1 Create GPIO class under sysfs
    gpio_drv.class_gpio = class_create(THIS_MODULE, "Gpio_Class");
    if(IS_ERR(gpio_drv.class_gpio)){
        pr_err("Error while creating class\n");
        return PTR_ERR(gpio_drv.class_gpio);
    }

    platform_driver_register(&gpio_platform_driver);
    pr_info("GPIO driver Module loaded\n");
    return 0;
}

static void __exit gpio_sysfs_exit(void){
    platform_driver_unregister(&gpio_platform_driver);
    class_destroy(gpio_drv.class_gpio);
}



module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIKARI_NO_OMO");
MODULE_DESCRIPTION("GPIO Driver");
MODULE_INFO(board,"BBB");