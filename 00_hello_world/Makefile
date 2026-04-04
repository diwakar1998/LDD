FILENAME=main
obj-m := $(FILENAME).o

ARCH=arm
CROSS=arm-linux-gnueabihf-
KERN_DIR=/home/hikari/device_driver/linux_bbb
HOST_DIR=/lib/modules/$(shell uname -r)/build/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) modules
	$(MAKE) send clean

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) help
host:
	make -C $(HOST_DIR) M=$(PWD) modules
send:
	sshpass -p "temppwd" scp $(FILENAME).ko debian@192.168.7.2:/home/debian/drivers/
	