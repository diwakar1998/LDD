obj-m := platform_driver.o 

ARCH=arm
CROSS=arm-linux-gnueabihf-
KERN_DIR=../../linux
UBOOT_DIR=../../u-boot
HOST_DIR=/lib/modules/$(shell uname -r)/build/
DES_DIR=/home/debian/drivers
DTSFILE=am335x_boneblack_platform
OVERLAY_FILE=psuedo_dev1 psuedo_dev2
BBB_PASS=123456

Nproc=6

.PHONY: all compile clean help host host_clean send send_dtb send_ko dtb

all:clean compile dtb

compile:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) modules -j$(Nproc)
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) clean
	rm -f *.dtb u-boot* uImage MLO
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) help
setup_host:
	../setup_dd.sh host
setup_bbb:
	-../setup_dd.sh bbb
	make kernel_compile
setup_clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(UBOOT_DIR) clean
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) distclean -j$(Nproc)
kernel_compile:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) bb.org_defconfig -j$(Nproc)
# 	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) menuconfig #optional
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) uImage dtbs LOADADDR=0x82000000 -j$(Nproc)
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) modules -j$(Nproc)
	sudo make ARCH=$(ARCH) -C $(KERN_DIR) modules_install
	ls -lh $(KERN_DIR)/arch/arm/boot/uImage
	ls -lh $(KERN_DIR)/arch/arm/boot/dts/am335x-boneblack*.dtb
	cp $(KERN_DIR)/arch/arm/boot/dts/am335x-boneblack*.dtb .
	cp $(KERN_DIR)/arch/arm/boot/uImage .
	ls $(KERN_DIR)/arch/arm/boot/dts/am335x-*.dtb | wc -l
uboot_compile:
	make ARCH=$(ARCH) -C $(UBOOT_DIR) am335x_evm_defconfig
	make ARCH=$(ARCH) -C $(UBOOT_DIR) menuconfig
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(UBOOT_DIR) -j$(Nproc)
	ls -lh $(UBOOT_DIR)/MLO $(UBOOT_DIR)/u-boot.img
	cp $(UBOOT_DIR)/MLO .
	cp $(UBOOT_DIR)/u-boot.img $(UBOOT_DIR)/u-boot .
host:host_clean
	make -C $(HOST_DIR) M=$(PWD) modules
host_clean:
	make -C $(HOST_DIR) M=$(PWD) clean
send:
	make send_dtb send_ko

send_dtb:
	sshpass -p "$(BBB_PASS)" scp *.dtb debian@192.168.7.2:$(DES_DIR)
send_ko:
	sshpass -p "123456" scp *.ko debian@192.168.7.2:$(DES_DIR)
send_dtbo:
	sshpass -p "123456" scp *.dtbo debian@192.168.7.2:$(DES_DIR)

dtb:
	cp $(DTSFILE).dts $(KERN_DIR)/arch/arm/boot/dts/
	make -C $(KERN_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) DTC_FLAGS="-@" $(DTSFILE).dtb -j$(Nproc)
	cp $(KERN_DIR)/arch/arm/boot/dts/$(DTSFILE).dtb .

dtbo: $(OVERLAY_FILE:%=%.dtbo)
%.dtbo: %.dts
	@echo "Compiling $< -> $@"
	dtc -@ -I dts -O dtb -o $@ $<

