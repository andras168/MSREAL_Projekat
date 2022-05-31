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

#define DEVICE_NAME "bra"
#define DRIVER_NAME "bra_driver"
#define BUFF_SIZE 100

struct bra_info {
	unsigned long mem_start;
	unsigned long mem_end;
	void __iomem *base_addr;
};

static dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
static struct bra_info *bp = NULL;

int endRead = 0;

static int bra_probe(struct platform_device *pdev);
static int bra_remove(struct platform_device *pdev);

int bra_open(struct inode *pinode, struct file *pfile);
int bra_close(struct inode *pinode, struct file *pfile);
ssize_t bra_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t bra_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = bra_open,
	.read = bra_read,
	.write = bra_write,
	.release = bra_close,
};

static struct of_device_id bra_of_match[] = {
  { .compatible = "bra_gpio", },
  { /* end of list */ },
};

static struct platform_driver bra_driver = {
  .driver = {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table	= bra_of_match,
  },
  .probe		= bra_probe,
  .remove		= bra_remove,
};

MODULE_DEVICE_TABLE(of, bra_of_match);

static int bra_probe(struct platform_device *pdev)
{
  struct resource *r_mem;
  int rc = 0;
  r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (!r_mem) {
    printk(KERN_ALERT "Failed to get resource\n");
    return -ENODEV;
  }
  bp = (struct bra_info *) kmalloc(sizeof(struct bra_info), GFP_KERNEL);
  if (!bp) {
    printk(KERN_ALERT "Could not allocate led device\n");
    return -ENOMEM;
  }

  bp->mem_start = r_mem->start;
  bp->mem_end = r_mem->end;
  //printk(KERN_INFO "Start address:%x \t end address:%x", r_mem->start, r_mem->end);

  if (!request_mem_region(bp->mem_start,bp->mem_end - bp->mem_start + 1,	DRIVER_NAME))
  {
    printk(KERN_ALERT "Could not lock memory region at %p\n",(void *)bp->mem_start);
    rc = -EBUSY;
    goto error1;
  }

  bp->base_addr = ioremap(bp->mem_start, bp->mem_end - bp->mem_start + 1);
  if (!bp->base_addr) {
    printk(KERN_ALERT "Could not allocate memory\n");
    rc = -EIO;
    goto error2;
  }

  printk(KERN_WARNING "bram platform driver registered\n");
  return 0;//ALL OK

error2:
  release_mem_region(bp->mem_start, bp->mem_end - bp->mem_start + 1);
error1:
  return rc;
}

static int bra_remove(struct platform_device *pdev)
{
  printk(KERN_WARNING "bram platform driver removed\n");
  iowrite32(0, bp->base_addr);
  iounmap(bp->base_addr);
  release_mem_region(bp->mem_start, bp->mem_end - bp->mem_start + 1);
  kfree(bp);
  return 0;
}

// # FILE OPERATIONS #

int bra_open(struct inode *pinode, struct file *pfile)
{
	printk(KERN_INFO "Succesfully opened bram. a\n");
	return 0;
}

int bra_close(struct inode *pinode, struct file *pfile)
{
	printk(KERN_INFO "Succesfully closed bram. a\n");
	return 0;
}
ssize_t bra_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	printk("Succefully read from bram. \n");
	return 0;
/*	int ret;
	int len = 0;
	u32 bra_val = 0;
	
	char buff[BUFF_SIZE];
	if(endRead) {
		endRead = 0;
		return 0;
	}

	bra_val = ioread32(bp->base_addr);
	
	ret = copy_to_user(buffer, buff, len);
	if(ret)
		return -EFAULT;
	printk(KERN_INFO "Succesfully read from bram.\n");
	endRead = 1;

	return len;*/ 
}

ssize_t bra_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {
  
  char buff[BUFF_SIZE];
  int ret = 0;
  int string_to_int = 0;

  /*-----------------While variables---------------*/
  char copy[100];
  int i,j = 0;
  char matrix_input[100];
  char s[2] = ";"; 
  char m[2] = ",";
  /*-----------------------------------------------*/

  ret = copy_from_user(buff, buffer, length);
  if(ret){
    printk("Copy from user failed! \n");
    return -EFAULT;
  }
  buff[length] = '\0';  
  
  sscanf(buff,"%s", matrix_input);
 
  ret = kstrtoint(matrix_input,0,&string_to_int);
  

  if(ret != 0) {	
 
	printk(KERN_INFO "This is in the buffer: %s\n", matrix_input);

	while (matrix_input != '\0') {

		printk(KERN_INFO "Entered while. Brace yourselves!");

		if(strcmp(matrix_input, s) == 0) {
			i++;
			printk(KERN_INFO "Incremented i.");
		} else if(strcmp(matrix_input,m) == 0) {
			j++;
			printk(KERN_INFO "Incremented j.");
		} else {
			strncat(copy, matrix_input, 1);	
		}
			//matrix_input++;
	} //while 
			
			printk(KERN_INFO "\n\n%s", copy);
			//return 0; //ostaje u while-u sa i bez return 0
  }
	
	return length;
}


// # init + exit # 

static int __init bra_init(void)
{
	int ret = 0;
	ret = alloc_chrdev_region(&my_dev_id, 0, 1, "bra");
	if (ret){
		printk(KERN_ERR "failed to register char device\n");
		return ret;
	}
	printk(KERN_INFO "char device region allocated\n");
	
	my_class = class_create(THIS_MODULE, "bra_class");
	if (my_class == NULL){
		printk(KERN_ERR "Failed to create class!\n");
		goto fail_0;
	}
	printk(KERN_INFO "Class created.\n");
	
	my_device = device_create(my_class, NULL, my_dev_id, NULL, "bra");
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

	//printk(KERN_INFO "bra world\n");
	return platform_driver_register(&bra_driver);

	fail_2:
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
	fail_1:
		class_destroy(my_class);
	fail_0:
		unregister_chrdev_region(my_dev_id, 1);
	return -1;
}
static void __exit bra_exit(void)
{
	cdev_del(my_cdev);
	device_destroy(my_class, my_dev_id);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id,1);
	printk(KERN_INFO "Bram_a exit.\n");
}

module_init(bra_init);
module_exit(bra_exit);
