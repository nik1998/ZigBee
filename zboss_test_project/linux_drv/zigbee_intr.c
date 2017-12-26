/***************************************************************************
*                                                                          *
* INSERT COPYRIGHT HERE!                                                   *
*                                                                          *
****************************************************************************
PURPOSE: Driver to catch UZ2400 interrupt and pass it to userspace via 
select(). Driver provides open/close/select interface.
select() blocks until interrupt come.
*/

//#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/poll.h>
//#include <asm/uaccess.h>


#include <linux/kernel.h>       /* printk() */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <asm/system.h>         /* cli(), *_flags */
#include <asm/uaccess.h>        /* copy_*_user */

#include "iomux.h"
#include <mach/board-mx31jade_rtrs.h>
#include <mach/imx-uart.h>
#include <mach/iomux-mx3.h>
#include <mach/irqs.h>
#include <mach/mxc_nand.h>
#include "devices.h"


static int zigbee_irq = IOMUX_TO_IRQ(MX31_PIN_SD_D_CLK);
static struct class *zbintr_class;

static struct zigbee_intr_data_s {
  dev_t dev_id;
  wait_queue_head_t q;
  atomic_t count;
  struct cdev cdev;
} zigbee_data;



static int zigbee_intr_open(struct inode *inode, struct file *filp)
{
  printk(KERN_ERR ">>zigbee_intr_open\n");

  nonseekable_open(inode, filp);

  enable_irq(zigbee_irq);

  printk(KERN_ERR "<<zigbee_intr_open\n");
	return 0;
}


int zigbee_intr_release(struct inode *inode, struct file *filp)
{
  printk(KERN_ERR ">>zigbee_intr_release\n");
  disable_irq(zigbee_irq);
  /* ... */
  printk(KERN_ERR "<<zigbee_intr_release\n");
  return 0;
}


static irqreturn_t zigbee_intr_irq(int irq, void *dev_data)
{
  atomic_inc(&zigbee_data.count);
  wake_up(&zigbee_data.q);
	return IRQ_HANDLED;
}


static unsigned int zigbee_intr_poll(struct file *filp, poll_table *wait)
{
//  printk(KERN_ERR ">>zigbee_intr_poll\n");

  if (atomic_read(&zigbee_data.count) > 0)
  {
//    printk(KERN_ERR "<< zigbee_intr_poll ret POLLIN no wait\n");
    atomic_dec(&zigbee_data.count);
    return POLLIN | POLLRDNORM;
  }
  poll_wait(filp, &zigbee_data.q,  wait);
  if (atomic_read(&zigbee_data.count) > 0)
  {
//    printk(KERN_ERR "<< zigbee_intr_poll ret POLLIN after wait\n");
    atomic_dec(&zigbee_data.count);
    return POLLIN | POLLRDNORM;
  }
//  printk(KERN_ERR "<< zigbee_intr_poll ret 0\n");
  return 0;
}


static struct file_operations zigbee_intr_fops = {
	.owner =	THIS_MODULE,
	.poll = zigbee_intr_poll,
	.open =		zigbee_intr_open,
	.release =	zigbee_intr_release,
};


static int __init zigbee_intr_init(void)
{
	int ret = 0;

  printk(KERN_INFO ">>zigbee intr driver init.\n");

  ret = alloc_chrdev_region(&zigbee_data.dev_id, 0, 1, "zbintr");
  if (ret < 0)
  {
		printk(KERN_WARNING "zigbee_intr: alloc_chrdev_region ret %d\n", ret);
		return ret;
  }

  cdev_init(&zigbee_data.cdev, &zigbee_intr_fops);
  zigbee_data.cdev.owner = THIS_MODULE;
  zigbee_data.cdev.ops = &zigbee_intr_fops;
  ret = cdev_add(&zigbee_data.cdev, zigbee_data.dev_id, 1);
  if (ret < 0)
  {
    printk(KERN_NOTICE "Error %d cdev_add", ret);
  }
  else
  {
    zbintr_class = class_create(THIS_MODULE, "zbintr");
    if (IS_ERR(zbintr_class)) {
      unregister_chrdev_region(zigbee_data.dev_id, 1);
      return PTR_ERR(zbintr_class);
    }
    else
    {
      struct device *dev;

      dev = device_create(zbintr_class, NULL, zigbee_data.dev_id,
                          NULL, "zbintr");
      if (IS_ERR(dev))
      {
        return PTR_ERR(dev);
      }
    }

    init_waitqueue_head(&zigbee_data.q);
    atomic_set(&zigbee_data.count, 0);
    set_irq_type(zigbee_irq, IRQ_TYPE_EDGE_RISING);
    ret = request_irq(zigbee_irq, zigbee_intr_irq, 0, "zbintr", &zigbee_data);
    if (ret < 0)
    {
      printk(KERN_NOTICE "Error %d request_irq %d\n", ret, zigbee_irq);
    }
    disable_irq(zigbee_irq);
  }

  printk(KERN_INFO "<<zigbee intr driver init %d.\n", ret);
	return ret;
}


static void __exit zigbee_intr_exit(void)
{
  printk(KERN_INFO ">>zigbee intr driver exit.\n");
  device_destroy(zbintr_class, zigbee_data.dev_id);
  free_irq(zigbee_irq, &zigbee_data);
	unregister_chrdev_region(zigbee_data.dev_id, 1);
  printk(KERN_INFO "<<zigbee intr driver exit.\n");
}

module_init(zigbee_intr_init);
module_exit(zigbee_intr_exit);

MODULE_AUTHOR("E.E.");
MODULE_DESCRIPTION("Zigbee interrupt to userspace Driver");
MODULE_LICENSE("GPL");

