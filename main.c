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


struct snull_priv {
    spinlock_t lock;
};

static int snull_open(struct net_device *dev)
{
    
    printk(KERN_INFO "SNULL open...\n");
    netif_start_queue(dev);
    return 0;
}

static int snull_release(struct net_device *dev)
{
    printk(KERN_INFO "SNULL release...\n");
    netif_stop_queue(dev);
    return 0;
}

static int snull_xmit(struct sk_buff *skb, struct net_device *dev)
{
    
    skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
    netif_rx(skb);

    printk(KERN_INFO "SNULL xmit...\n");

    return 0;
}

static int snull_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    printk(KERN_INFO "SNULL ioctl: command: %x\n", cmd);
    return 0;
}

// ---------------------------------------------------------------

static const struct net_device_ops snull_netdev_ops = {
        .ndo_open = snull_open,
        .ndo_stop = snull_release,
        .ndo_start_xmit = snull_xmit,
        .ndo_do_ioctl = snull_ioctl
};


void snull_init(struct net_device *dev)
{
    struct snull_priv *priv;

    ether_setup(dev);
    dev->netdev_ops = &snull_netdev_ops;
    dev->watchdog_timeo = 5;
    dev->flags           |= IFF_NOARP;
    dev->features        |= NETIF_F_HW_CSUM;

    priv = netdev_priv(dev);
    memset(priv, 0, sizeof(struct snull_priv));
    spin_lock_init(&priv->lock);

    printk(KERN_INFO "SNULL device initialized!\n");
}

// ---------------------------------------------------------------

struct net_device *snull_dev = NULL;

static int __init snull_init_module(void)
{
    printk(KERN_INFO "SNULL init module...\n");
    snull_dev = alloc_netdev(0, "SNULL", NET_NAME_UNKNOWN, snull_init);
    if (register_netdev(snull_dev)) {
        printk("SNULL: device init error\n");
        return 1;
    }
    return 0;
}

static void __exit snull_exit_module(void)
{
    unregister_netdev(snull_dev);
    printk(KERN_INFO "SNULL exit module... Goodbye!\n");
}


module_init(snull_init_module);
module_exit(snull_exit_module);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gag Hayrapetyan");
MODULE_VERSION("1.0.0");
