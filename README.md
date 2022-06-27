# VirtualNetworkCardDriver
This driver creates a network device and all packets sent are bounced back.

## Build and run
```
make
sudo insmod main.ko
```
Check that the module successfully loaded
```
dmesg
```
