obj-m := gpio_driver.o 
ARCH=arm
CROSS=arm-linux-gnueabihf-
KERN_DIR=../../linux
UBOOT_DIR=../../u-boot
HOST_DIR=/lib/modules/$(shell uname -r)/build/
DTSFILE=am335x_boneblack_platform
ccflags-y += -fno-stack-protector

Nproc=6

.PHONY: all compile clean help \
        setup_host setup_bbb setup_clean \
        kernel_compile \
        compile_commands clean_compile_commands \
        uboot_compile \
        host host_clean \
        send send_dtb send_ko \
        dtb

all:clean compile dtb

compile:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERN_DIR) M=$(PWD) modules -j$(Nproc)
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
setup_clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(UBOOT_DIR)clean
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
compile_commands:
	python3 $(KERN_DIR)/scripts/clang-tools/gen_compile_commands.py
clean_compile_commands:
	rm -f compile_commands.json
uboot_compile:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(UBOOT_DIR) am335x_evm_defconfig
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
	sshpass -p "123456" scp *.dtb debian@192.168.7.2:/home/debian/drivers/
send_ko:
	sshpass -p "123456" scp *.ko debian@192.168.7.2:/home/debian/drivers/
dtb:
	cp $(DTSFILE).dts $(KERN_DIR)/arch/arm/boot/dts/
	make -C $(KERN_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) $(DTSFILE).dtb -j$(Nproc)
	cp $(KERN_DIR)/arch/arm/boot/dts/$(DTSFILE).dtb .