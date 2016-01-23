# Netgear-A6210
This driver supports Ralink / Mediatek mt766u, mt7632u and mt7612u chipsets.

To build the driver, follow these steps:

    $ git clone https://github.com/jurobystricky/Netgear-A6210
    $ cd Netgear-A6210
    $ make
    $ sudo make install

The driver is tested on Ubuntu 15.10 in conjunction with NETGEAR AC1200
High Gain Wifi USB Adapter. The driver should work with other Linux releases
as well. 

The supported chipsets can be present in other devices. To include additional 
devices, you need to add corresponding VendorID, DeviceID into the file 
"rtusb_dev_id.c"

The original code was downloaded from: 
http://cdn-cw.mediatek.com/Downloads/linux/MT7612U_DPO_LinuxSTA_3.0.0.1_20140718.tar.bz2

This is work in progress. The driver is functional. However, there are still
several issues that need to be addressed. In particular, hot-unplugging may
cause the network manager to segfault. After plugging the dongle back in, 
you may need to restart the manager:

    $ sudo service network-manager restart
    

    
    
