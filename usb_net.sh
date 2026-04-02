
#enable USB internet for BBB
#wlp2so0 could vary based on your machine ifconfig's output(For VM it was ens33)
sudo iptables --table nat --append POSTROUTING --out-interface ens33 -j MASQUERADE
sudo iptables --append FORWARD --in-interface ens33 -j ACCEPT
sudo echo 1 > /proc/sys/net/ipv4/ip_forward


#For internet in BBB
#/etc/resolv.conf
#        dns-nameservers 8.8.8.8
#        dns-nameservers 8.8.4.4
#
#
#/etc/network/interfaces
#iface usb0 inet static
#        address 192.168.7.2
#        netmask 255.255.255.252
#        network 192.168.7.0
#        gateway 192.168.7.1
#        dns-nameservers 8.8.8.8
#        dns-nameservers 8.8.4.4
#route add default gw 192.168.7.1
