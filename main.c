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
#include <linux/version.h> 


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
    printk(KERN_INFO "SNULL xmit...\n");
    dev_kfree_skb(skb);
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

static void snull_rx_ints(struct net_device *dev, int enable)
{
	struct snull_priv *priv = netdev_priv(dev);
	priv->rx_int_enabled = enable;
}

void snull_setup_pool(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	int i;
	struct snull_packet *pkt;

	priv->ppool = NULL;
	for (i = 0; i < 8; i++) {
		pkt = kmalloc (sizeof (struct snull_packet), GFP_KERNEL);
		if (pkt == NULL) {
			printk (KERN_NOTICE "Ran out of memory allocating packet pool\n");
			return;
		}
		pkt->dev = dev;
		pkt->next = priv->ppool;
		priv->ppool = pkt;
	}
}

void snull_init(struct net_device *dev)
{
    struct snull_priv *priv;

    ether_setup(dev);
    dev->netdev_ops = &snull_netdev_ops;
    dev->watchdog_timeo = 5;
    dev->flags           |= IFF_NOARP;
    dev->features        |= NETIF_F_HW_CSUM;

   priv = netdev_priv(dev);
    memset(prev, 0, sizeof(snull_priv));
    spin_lock_init(&priv->lock);
    priv->dev=dev;

    snull_rx_ints(dev, 1);		// enable receive interrupts 
	snull_setup_pool(dev);

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
