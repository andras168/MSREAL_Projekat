#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <linux/uaccess.h>


#include <linux/io.h> //iowrite ioread
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>//ioremap

MODULE_LICENSE("Dual BSD/GPL");

#define DEVICE_NAME "brc"
#define DRIVER_NAME "brc_driver"
#define BUFF_SIZE 100

/*----------------------------------- FUNCTION PROTOTYPES -------------------------------------*/
static int brc_probe(struct platform_device *pdev);
static int brc_remove(struct platform_device *pdev);

int brc_open(struct inode *pinode, struct file *pfile);
int brc_close(struct inode *pinode, struct file *pfile);
ssize_t brc_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t brc_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);
static int __init brc_init(void);
static void __exit brc_exit(void);


/*------------------------------------- GLOBAL VARIABLES --------------------------------------*/
struct brc_info {
        unsigned long mem_start;
        unsigned long mem_end;
        void __iomem *base_addr;
};

static dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
static struct brc_info *bp = NULL;

int endRead = 0;

struct file_operations my_fops =
{       
        .owner = THIS_MODULE,
        .open = brc_open,
        .read = brc_read,
        .write = brc_write,
        .release = brc_close,
};

static struct of_device_id brc_of_match[] =
{ 
  { .compatible = "bram_gpio",
    .compatible = "xlnx,axi-bram-ctrl-C", },
  { /* end of list */ },
};

static struct platform_driver brc_driver = {
  .driver = {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table     = brc_of_match,
  },
  .probe                = brc_probe,
  .remove               = brc_remove,
};

MODULE_DEVICE_TABLE(of, brc_of_match);

/*-------------------------------- PROBE & REMOVE -------------------------------------*/
static int brc_probe(struct platform_device *pdev)
{
  struct resource *r_mem;
  int rc = 0;
  printk(KERN_INFO "Probing ...\n");
  // Get physical register adress space from device tree
  r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);


  if (!r_mem) {
    printk(KERN_ALERT "Failed to get resource!\n");
    return -ENODEV;
  }
  // Get memory for structure bra_info
  bp = (struct brc_info *) kmalloc(sizeof(struct brc_info), GFP_KERNEL);
  if (!bp) {
    printk(KERN_ALERT "Could not allocate device!\n");
    return -ENOMEM;
  }
  // Put physical adresses in brc_info structure
  bp->mem_start = r_mem->start;
  bp->mem_end = r_mem->end;

  // Reserve that memory space for this driver
  if (!request_mem_region(bp->mem_start,bp->mem_end - bp->mem_start + 1, DRIVER_NAME))
  {
    printk(KERN_ALERT "Could not lock memory region at %p\n.",(void *)bp->mem_start);
    rc = -EBUSY;
    goto error1;
  }

  // Remap physical to virtual adresses
  bp->base_addr = ioremap(bp->mem_start, bp->mem_end - bp->mem_start + 1);
  if (!bp->base_addr) {
    printk(KERN_ALERT "Could not allocate memory!\n");
    rc = -EIO;
    goto error2;
  }

  if(bp->mem_start == r_mem->start) {
        printk(KERN_INFO "mem_start == r_mem");
  } else {
        printk(KERN_ERR "mem_start != r_mem");
  }

  printk(KERN_WARNING "BRAM_C platform driver registered.\n");
  return 0;//ALL OK

error2:
  release_mem_region(bp -> mem_start, bp -> mem_end - bp -> mem_start + 1);
error1:
  return rc;
}

static int brc_remove(struct platform_device *pdev)
{ 
  printk(KERN_WARNING "BRAM_C platform driver removed!\n");
  iowrite32(0, bp->base_addr);
  printk(KERN_INFO "BRAM_C remove in process.");
  iounmap(bp->base_addr);
  release_mem_region(bp->mem_start, bp->mem_end - bp->mem_start + 1);
  kfree(bp);
  printk(KERN_INFO "BRAM_C driver removed.");
  return 0;
}

/*------------------------------------ FILE OPERATIONS ---------------------------------------*/

int brc_open(struct inode *pinode, struct file *pfile)
{
        printk(KERN_INFO "Succesfully opened BRAM_C.\n");
        return 0;
}

int brc_close(struct inode *pinode, struct file *pfile)
{
        printk(KERN_INFO "Succesfully closed BRAM_C.\n");
        return 0;
}
ssize_t brc_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
        int i=0;
        uint32_t matrix;
        for(i=0;i<16;i++) {
                matrix=ioread32(bp->base_addr+4*i);
                printk(KERN_INFO "Value at [%d]: %u",i,matrix);
                }

        return 0;
}

ssize_t brc_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {



        return length;
}


/*------------------------------ END OF MATRIX PART -----------------------------*/

static int __init brc_init(void)
{
        int ret = 0;
        ret = alloc_chrdev_region(&my_dev_id, 0, 1, "brc");
        if (ret){
                printk(KERN_ERR "Failed to register char device!\n");
                return ret;
        }
        printk(KERN_INFO "Char device region allocated.\n");

        my_class = class_create(THIS_MODULE, "brc_class");
        if (my_class == NULL){
                printk(KERN_ERR "Failed to create class!\n");
                goto fail_0;
        }
        printk(KERN_INFO "Class created.\n");

        my_device = device_create(my_class, NULL, my_dev_id, NULL, "brc");
        if (my_device == NULL){
                printk(KERN_ERR "Failed to create device!\n");
                goto fail_1;
        }
        printk(KERN_INFO "Device created.\n");

        my_cdev = cdev_alloc();
        my_cdev->ops = &my_fops;
        my_cdev->owner = THIS_MODULE;
        ret = cdev_add(my_cdev, my_dev_id, 1);

        if (ret)
        {
                printk(KERN_ERR "Failed to add cdev!\n");
                goto fail_2;
        }
        printk(KERN_INFO "Cdev added.\n");

        return platform_driver_register(&brc_driver);

        fail_2:
                device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
        fail_1:
                class_destroy(my_class);
        fail_0:
                unregister_chrdev_region(my_dev_id, 1);
        return -1;
}

static void __exit brc_exit(void)
{
        platform_driver_unregister(&brc_driver);
        cdev_del(my_cdev);
        device_destroy(my_class, my_dev_id);
        class_destroy(my_class);
        unregister_chrdev_region(my_dev_id,1);
        printk(KERN_WARNING "BRAM_C removed!\n");
}

module_init(brc_init);
module_exit(brc_exit);
