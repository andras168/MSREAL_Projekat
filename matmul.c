#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

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

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MATRIX MULTIPLIER");
#define DEVICE_NAME "matmul"
#define DRIVER_NAME "matrix_multiplier"
#define BUFF_SIZE 20

struct matmul_info {
  unsigned long mem_start;
  unsigned long mem_end;
  void __iomem *base_addr;
};

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

int flag_dim = 0;
int matrix_dim_n, matrix_dim_m, matrix_dim_p;
int pos = 0;
int endRead = 0;
int storage[10];
static struct matmul_info *mp = NULL;

static int matmul_probe(struct platform_device *pdev);
static int matmul_remove(struct platform_device *pdev);

int matmul_open(struct inode *pinode, struct file *pfile);
int matmul_close(struct inode *pinode, struct file *pfile);
ssize_t matmul_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t matmul_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = matmul_open,
	.read = matmul_read,
	.write = matmul_write,
	.release = matmul_close,
};

static struct of_device_id matmul_of_match[] =
{
  { .compatible = "bram_gpio",
    .compatible = "matmul_gpio",
    .compatible = "xlnx,matrix-multiplier",},
  { /* end of list */ },
};

static struct platform_driver matmul_driver = {
  .driver = {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table	= matmul_of_match,
  },
  .probe		= matmul_probe,
  .remove		= matmul_remove,
};

MODULE_DEVICE_TABLE(of, matmul_of_match);

int matmul_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened file.\n");
		return 0;
}

int matmul_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed file.\n");
		return 0;
}

/*--------------------------- PROBE & REMOVE --------------------------------------*/

static int matmul_probe(struct platform_device *pdev)
{
  struct resource *r_mem;
  int rc = 0;
  printk(KERN_INFO "Probing...\n");
  // Get physical register adress space from device tree
  r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);

 
  if (!r_mem) {
    printk(KERN_ALERT "Failed to get resource!\n");
    return -ENODEV;
  }
  // Get memory for structure bra_info
  mp = (struct matmul_info *) kmalloc(sizeof(struct matmul_info), GFP_KERNEL);
  if (!mp) {
    printk(KERN_ALERT "Could not allocate matmul device!\n");
    return -ENOMEM;
  }
  // Put physical adresses in matmul_info structure
  mp->mem_start = r_mem->start;
  mp->mem_end = r_mem->end;

  // Reserve that memory space for this driver
  if (!request_mem_region(mp->mem_start,mp->mem_end - mp->mem_start + 1, DRIVER_NAME))
  {
    printk(KERN_ALERT "Could not lock memory region at %p\n.",(void *)mp->mem_start);
    rc = -EBUSY;
    goto error1;
  }

  // Remap physical to virtual adresses
  mp->base_addr = ioremap(mp->mem_start, mp->mem_end - mp->mem_start + 1);
  if (!mp->base_addr) {
    printk(KERN_ALERT "Could not allocate memory!\n");
    rc = -EIO;
    goto error2;
  }

  if(mp->mem_start == r_mem->start) {
  	printk(KERN_NOTICE "mem_start == r_mem");
  } else {
  	printk(KERN_NOTICE "mem_start != r_mem");
  }

  printk(KERN_WARNING "Matmul platform driver registered.\n");
  return 0;//ALL OK

error2:
  release_mem_region(mp -> mem_start, mp -> mem_end - mp -> mem_start + 1);
error1:
  return rc;
}

static int matmul_remove(struct platform_device *pdev)
{
  printk(KERN_WARNING "Matmul platform driver removed!\n");
  iowrite32(0, mp->base_addr);
  printk(KERN_INFO "Matmul remove in process.");
  iounmap(mp->base_addr);
  release_mem_region(mp->mem_start, mp->mem_end - mp->mem_start + 1);
  kfree(mp);
  printk(KERN_INFO "Matmul driver removed.");
  return 0;
}

ssize_t matmul_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
        uint32_t m,n,p,ready,start = 0;
	long int len;
	
	if (endRead){
		endRead = 0;
		pos = 0;
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
	}
	ret = copy_to_user(buffer, buff, len);

	ready = ioread32(mp->base_addr);	
	start = ioread32(mp->base_addr+4);
	n = ioread32(mp->base_addr+8);	
	m = ioread32(mp->base_addr+12);
	p = ioread32(mp->base_addr+16);

	printk(KERN_INFO "ready=%u,start=%u,n=%u,m=%u,p=%u", ready,start,n,m,p);
	endRead = 1;
	return len;
}


ssize_t matmul_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	
	char buff[BUFF_SIZE];
	int n, m, p, start;
	//int ready;
	int ret;		
	uint32_t trig0 = 0;
	uint32_t trig1 = 1;
	char trigger[10] = "trigger";
	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';
	
	if(strstr(buff,"start=trigger") != NULL ) {
		ret = 1;
		start = 2;
	} else if(strstr(buff,"dim=") != NULL) {
		ret = sscanf(buff,"dim=%d,%d,%d",&n,&m,&p); //ret = 3
	} else if(strstr(buff, "start=") != NULL) {
		ret = sscanf(buff, "start=%d", &start);
		printk(KERN_INFO "RET = %d", ret);
	}

	if(ret == 3) //three parameters parsed in sscanf
	{  
		flag_dim = 1;	
		if(n >= 1 && n <= 7)
		{
			matrix_dim_n = n;
			printk(KERN_INFO "Dimension N: %d\n", n);
			iowrite32((u32)n, mp->base_addr + 8);
		} else {
			printk(KERN_ERR "Dimension N should be between 1 and 7\n");
		}
		
		if(m >= 1 && m <= 7)
		{
			matrix_dim_m = m;
			printk(KERN_INFO "Dimension M: %d\n", m);
			iowrite32((u32)m, mp->base_addr+12);
		} else {
			printk(KERN_ERR "Dimension M should be between 1 and 7\n");
		}
		
		if (p >= 1 && p <= 7)
		{
			matrix_dim_p = p;
			printk(KERN_INFO "Dimension P: %d\n", p); 
			iowrite32((u32)p, mp->base_addr+16);
		} else {
			printk(KERN_ERR "Dimension P should be between 1 and 7\n");
		}	

	} else if(ret == 1) {
		if(flag_dim == 1) {	
			switch(start) {
				case(0):
					iowrite32((u32)start, mp->base_addr+4);
					printk(KERN_INFO "Matrix muliplication stopped.");
					printk(KERN_WARNING "Start -> %d",start);
					flag_dim = 0;
					break;
				case(1):
					iowrite32((u32)start, mp->base_addr+4);
					printk(KERN_INFO "Matrix multiplication started.");
					printk(KERN_WARNING "Start -> %d",start);
					break;

				case(2):
					iowrite32((u32)trig1, mp->base_addr+4);
					printk(KERN_INFO "Trigger = %u", trig1);
					iowrite32((u32)trig0, mp->base_addr+4);
					printk(KERN_INFO "Trigger = %u", trig0);
					printk(KERN_INFO "start= %d", start);
					break;
				default:
					printk(KERN_ERR "Wrong parameter for Start! (START = 1, STOP = 0)");
					break;
			}
		} else if(flag_dim == 0) {
			printk(KERN_ERR "Please specify the matrix dimensions first!");
		}	

	}	
	
	return length;
}

static int __init matmul_init ( void )
{
	int ret;
 	ret = alloc_chrdev_region(&my_dev_id, 0 , 1 , "matmul" );
	if (ret){
		printk(KERN_ERR "Failed to register char device!\n" );
		return ret;
	}
	printk(KERN_INFO "Char device region allocated.\n" );

	my_class = class_create(THIS_MODULE, "matmul_class" );
	if (my_class == NULL ){
		printk(KERN_ERR "Failed to create class!\n" );
		goto fail_0;
	}
	printk(KERN_INFO "Class created.\n" );

	my_device = device_create(my_class, NULL , my_dev_id, NULL ,"matmul" );
	if (my_device == NULL ){
		printk(KERN_ERR "Failed to create device!\n" );
		goto fail_1;
	}
	printk(KERN_INFO "Device created.\n" );

	my_cdev = cdev_alloc();
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1 );
	
	if (ret)
	{
		printk(KERN_ERR "failed to add cdev\n" );
		goto fail_2;
	}
	
	printk(KERN_INFO "Cdev added\n" );
	//printk(KERN_INFO "Hello world\n" );
	
	return platform_driver_register(&matmul_driver);
	
	fail_2:
		device_destroy(my_class, my_dev_id);
	fail_1:
		class_destroy(my_class);
	fail_0:
		unregister_chrdev_region(my_dev_id, 1 );
	return -1 ;
}

static void __exit matmul_exit ( void )
{
	platform_driver_unregister(&matmul_driver);
        cdev_del(my_cdev);
	device_destroy(my_class, my_dev_id);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id, 1 );
	printk(KERN_INFO "Matmul exit.\n" );
}

module_init(matmul_init);
module_exit(matmul_exit);
