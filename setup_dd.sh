#!/bin/bash

# Default to 'host' if no argument provided
MODE="${1:-host}"

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

# Check which setup mode
if [ "$MODE" == "bbb" ]; then
    echo "Downloading Linux kernel image..."
    git clone https://github.com/beagleboard/linux.git -b v5.10.168-ti-r72 --depth=1 ../linux

    echo "Setting up toolchain..."
    rm -rf gcc*.xz*
    wget https://releases.linaro.org/components/toolchain/binaries/7.5-2019.12/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz
    tar -xvf gcc*.xz
    rm -rf gcc*.xz*
    mv gcc* ../.

    cd ../gcc*/bin
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