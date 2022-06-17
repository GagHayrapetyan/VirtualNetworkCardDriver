#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

#include <linux/netdevice.h>

static int chriz_open(struct net_device *dev)
{
    printk(KERN_INFO "chriz open...\n");
    netif_start_queue(dev);
    return 0;
}

static int chriz_release(struct net_device *dev)
{
    printk(KERN_INFO "chriz release...\n");
    netif_stop_queue(dev);
    return 0;
}

static int chriz_xmit(struct sk_buff *skb, struct net_device *dev)
{
    printk(KERN_INFO "chriz xmit...\n");
    dev_kfree_skb(skb);
    return 0;
}

static int chriz_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    printk(KERN_INFO "chriz ioctl: command: %x\n", cmd);
    return 0;
}

// ---------------------------------------------------------------

static const struct net_device_ops chriz_netdev_ops = {
        .ndo_open = chriz_open,
        .ndo_stop = chriz_release,
        .ndo_start_xmit = chriz_xmit,
        .ndo_do_ioctl = chriz_ioctl
};

void chriz_init(struct net_device *dev)
{
    ether_setup(dev);
    dev->netdev_ops = &chriz_netdev_ops;
    dev->watchdog_timeo = 5;
    dev->flags           |= IFF_NOARP;
    dev->features        |= NETIF_F_HW_CSUM;
    printk(KERN_INFO "chriz device initialized!\n");
}

// ---------------------------------------------------------------

struct net_device *chriz_dev = NULL;

static int __init chriz_init_module(void)
{
    printk(KERN_INFO "chriz init module...\n");
    chriz_dev = alloc_netdev(0, "chriz", NET_NAME_UNKNOWN, chriz_init);
    if (register_netdev(chriz_dev)) {
        printk("chriz: device init error\n");
        return 1;
    }
    return 0;
}

static void __exit chriz_exit_module(void)
{
    unregister_netdev(chriz_dev);
    printk(KERN_INFO "chriz exit module... Goodbye!\n");
}


module_init(chriz_init_module);
module_exit(chriz_exit_module);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gag Hayrapetyan");
MODULE_VERSION("1.0.0");
