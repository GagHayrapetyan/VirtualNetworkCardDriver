#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/interrupt.h> /* mark_bh */
#include <linux/in.h>
#include <linux/netdevice.h>   /* struct device, and other headers */
#include <linux/etherdevice.h> /* eth_type_trans */
#include <linux/ip.h>          /* struct iphdr */
#include <linux/tcp.h>         /* struct tcphdr */
#include <linux/skbuff.h>
#include <linux/in6.h>
#include <asm/checksum.h>


struct net_device *drv_dev = NULL;

void swap_ip_address(struct sk_buff *skb){
    struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);
   
    __be32 ip_tmp = ip_header->saddr;
    ip_header->saddr = ip_header->daddr;
    ip_header->daddr = ip_tmp;
}

void swap_mac_address(struct sk_buff *skb){
    struct ethhdr *ether = eth_hdr(skb);   
    
    unsigned char mac_tmp[ETH_ALEN];
    memcpy(mac_tmp, ether->h_source, sizeof(mac_tmp));
    memcpy(ether->h_source, ether->h_dest, sizeof(mac_tmp));
    memcpy(ether->h_dest, mac_tmp, sizeof(mac_tmp));
}


static int drv_open(struct net_device *dev)
{
    printk(KERN_INFO "Driver open...\n");
    netif_start_queue(dev);
    return 0;
}

static int drv_release(struct net_device *dev)
{
    printk(KERN_INFO "Driver release...\n");
    netif_stop_queue(dev);
    return 0;
}

static int drv_xmit(struct sk_buff *skb, struct net_device *dev)
{
    swap_ip_address(skb);
    swap_mac_address(skb);

    skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
    netif_rx_ni(skb);
    
    printk(KERN_INFO "Driver xmit...\n");

    return 0;
}

static int drv_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    printk(KERN_INFO "Driver ioctl: command: %x\n", cmd);
    return 0;
}

// ---------------------------------------------------------------

static const struct net_device_ops drv_netdev_ops = {
        .ndo_open = drv_open,
        .ndo_stop = drv_release,
        .ndo_start_xmit = drv_xmit,
        .ndo_do_ioctl = drv_ioctl
};


void drv_init(struct net_device *dev)
{
    ether_setup(dev);
    dev->netdev_ops = &drv_netdev_ops;
    dev->dev_addr[0] = 1;
    dev->dev_addr[1] = 2;
    dev->dev_addr[2] = 3;
    dev->dev_addr[3] = 4;
    dev->dev_addr[4] = 5;
    dev->dev_addr[5] = 6;
    dev->watchdog_timeo = 5;
    dev->flags           |= IFF_NOARP;
    dev->features        |= NETIF_F_HW_CSUM;

    printk(KERN_INFO "Driver device initialized!\n");
}

static int __init drv_init_module(void)
{
    printk(KERN_INFO "Driver init module...\n");
    drv_dev = alloc_netdev(0, "myeth", NET_NAME_UNKNOWN, drv_init);
    if (register_netdev(drv_dev)) {
        printk("Driver: device init error\n");
        return 1;
    }
    return 0;
}

static void __exit drv_exit_module(void)
{
    unregister_netdev(drv_dev);
    printk(KERN_INFO "Driver exit module... Goodbye!\n");
}


module_init(drv_init_module);
module_exit(drv_exit_module);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gag Hayrapetyan");
MODULE_VERSION("1.0.0");