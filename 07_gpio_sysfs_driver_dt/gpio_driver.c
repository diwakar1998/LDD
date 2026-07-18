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
};

struct gpio_drv_data *gpio_drv;
int gpio_remove(struct platform_device *pdev);
int gpio_probe(struct platform_device *pdev);

int gpio_probe(struct platform_device *pdev){
    struct device *dev = &pdev->dev;
    const char *name;
    
    struct device_node *parent = pdev->dev.of_node;
    struct device_node *child = NULL;

    struct gpio_dev_data *device_data;
    int i = 0;

    pr_info("Inside gpio Probe\n");
    
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

        device_data->des = devm_fwnode_get_gpiod_from_child(dev, "bone", &child->fwnode, GPIOD_ASIS,device_data->label);
        i++;
    }
    return 0;
}

int gpio_remove(struct platform_device *pdev){
    pr_info("Inside gpio remove\n");
    
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
    gpio_drv->class_gpio = class_create(THIS_MODULE, "Gpio_Class");
    if(IS_ERR(gpio_drv->class_gpio)){
        pr_err("Error while creating class\n");
        return PTR_ERR(gpio_drv->class_gpio);
    }
    pr_info("GPIO driver Module loaded\n");
    return 0;
}

module_init(gpio_sysfs_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIKARI_NO_OMO");
MODULE_DESCRIPTION("GPIO Driver");
MODULE_INFO(board,"BBB");