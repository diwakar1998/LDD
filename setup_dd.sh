#!/bin/bash
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

#download linux kernel image
git clone https://github.com/beagleboard/linux.git -b v5.10.168-ti-r72 --depth=1

#toolchain setup
rm -rf gcc*.xz*
wget https://releases.linaro.org/components/toolchain/binaries/7.5-2019.12/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz
tar -xvf gcc*.xz
rm -rf gcc*.xz*

cd gcc*/bin
echo "export PATH=\"\$PATH:$PWD\"" >> ~/.bashrc
source ~/.bashrc
