#!/bin/bash

# Default to 'host' if no argument provided
MODE="${1:-host}"

# Get the script's directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
echo $SCRIPT_DIR
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
echo $PARENT_DIR
# Always install packages
sudo apt-get update -y
sudo apt-get upgrade -y
sudo apt-get install build-essential lzop u-boot-tools net-tools bison flex libssl-dev libncurses5-dev libncursesw5-dev unzip chrpath xz-utils minicom wget git-core -y
sudo apt install iptables -y
sudo apt-get install u-boot-tools -y
sudo apt-get install libssl-dev sshfs -y
sudo apt-get install lz4 lzop lzma -y
sudo apt-get install ncurses5-dev bison flex gettext -y
sudo apt-get install build-essential git libmpc-dev -y
sudo apt-get install minicom -y
sudo apt install gcc-arm-none-eabi -y

# Check which setup mode
if [ "$MODE" == "bbb" ]; then
    echo "Downloading Linux kernel image..."
    if [ ! -d "$PARENT_DIR/linux" ]; then
        git clone https://github.com/beagleboard/linux.git -b v5.10.168-ti-r72 --depth=1 "$PARENT_DIR/linux"
    else
        echo "Linux kernel already downloaded at $PARENT_DIR/linux, skipping..."
    fi

    echo "Downloading U-Boot bootloader..."
    if [ ! -d "$PARENT_DIR/u-boot" ]; then
        git clone --depth=1 --branch v2023.04 https://github.com/u-boot/u-boot.git "$PARENT_DIR/u-boot"
    else
        echo "U-Boot already downloaded at $PARENT_DIR/u-boot, skipping..."
    fi

    echo "Setting up toolchain..."
    cd "$PARENT_DIR"
    if [ ! -f "gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz" ]; then
        echo "Downloading GCC toolchain..."
        wget -P "$PARENT_DIR" https://releases.linaro.org/components/toolchain/binaries/7.5-2019.12/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz
        tar -xvf gcc*.xz
    else
        echo "GCC toolchain already downloaded, skipping..."
    fi
    
    echo "Download Debian image from https://www.beagleboard.org/distros/"
    # rm -rf gcc*.xz*

    cd gcc*/bin
    echo "export PATH=\"\$PATH:$PWD\"" >> ~/.bashrc
    source ~/.bashrc
    
    echo "BBB setup completed successfully!"
elif [ "$MODE" == "host" ]; then
    uname -r
    sudo apt-get install linux-headers-$(uname -r) -y
    echo "Host setup completed successfully!"
else
    echo "Invalid argument. Please use 'bbb' for BeagleBone Black setup or 'host' for host setup."
fi