obj-m := psuedo.o

ARCH=arm
CROSS=arm-linux-gnueabihf-
KERN_DIR=/home/diwakar/Desktop/ldd/source/linux_bbb_4.14
HOST_DIR=/lib/modules/$(shell uname -r)/build/

all:clean compile

compile:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) help
host:
	make -C $(HOST_DIR) M=$(PWD) modules
send:
	sshpass -p "temppwd" scp psuedo.ko debian@192.168.7.2:/home/debian/drivers/
