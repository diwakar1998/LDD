obj-m := platform_driver.o 

ARCH=arm
CROSS=arm-linux-gnueabihf-
KERN_DIR=../../linux
UBOOT_DIR=../../u-boot
HOST_DIR=/lib/modules/$(shell uname -r)/build/
DTSFILE=am335x_boneblack_platform

.PHONY: all compile clean help host host_clean send send_dtb send_ko dtb

all:clean compile dtb

compile:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) modules -j4
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) clean
	rm -f *.dtb
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) help
setup_host:
	../setup_dd.sh host
setup_bbb:
	-../setup_dd.sh bbb
	make kernel_compile
kernel_compile:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) distclean -j4
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) bb.org_defconfig -j4
# 	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) menuconfig #optional
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) uImage dtbs LOADADDR=0x82000000 -j4
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) modules
	sudo make ARCH=$(ARCH) -C $(KERN_DIR) modules_install
	ls -lh $(KERN_DIR)/arch/arm/boot/uImage
	ls -lh $(KERN_DIR)/arch/arm/boot/dts/am335x-boneblack*.dtb
	cp $(KERN_DIR)/arch/arm/boot/dts/am335x-boneblack*.dtb .
	cp $(KERN_DIR)/arch/arm/boot/uImage .
	ls $(KERN_DIR)/arch/arm/boot/dts/am335x-*.dtb | wc -l
uboot_compile:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(UBOOT_DIR) am335x_evm_defconfig
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(UBOOT_DIR) -j4
	ls -lh $(UBOOT_DIR)/MLO $(UBOOT_DIR)/u-boot.img
	cp $(UBOOT_DIR)/MLO .
	cp $(UBOOT_DIR)/u-boot.img .
host:host_clean
	make -C $(HOST_DIR) M=$(PWD) modules
host_clean:
	make -C $(HOST_DIR) M=$(PWD) clean
send:
	make send_dtb send_ko

send_dtb:
	sshpass -p "temppwd" scp *.dtb debian@192.168.7.2:/home/debian/drivers/
send_ko:
	sshpass -p "temppwd" scp *.ko debian@192.168.7.2:/home/debian/drivers/
dtb:
	cp $(DTSFILE).dts $(KERN_DIR)/arch/arm/boot/dts/
	make -C $(KERN_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) $(DTSFILE).dtb -j4
	cp $(KERN_DIR)/arch/arm/boot/dts/$(DTSFILE).dtb .