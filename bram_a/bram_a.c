
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

#define DEVICE_NAME "bram_a"
#define DRIVER_NAME "bram_driver"
#define BUFF_SIZE 100

/*----------------------------------- FUNCTION PROTOTYPES -------------------------------------*/
static int bram_probe(struct platform_device *pdev);
static int bram_remove(struct platform_device *pdev);

int bram_open(struct inode *pinode, struct file *pfile);
int bram_close(struct inode *pinode, struct file *pfile);
ssize_t bram_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t bram_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);
static int __init bram_init(void);
static void __exit bram_exit(void);


/*------------------------------------- GLOBAL VARIABLES --------------------------------------*/
struct bram_info {
	unsigned long mem_start;
	unsigned long mem_end;
	void __iomem *base_addr;
};

static dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
static struct bram_info *bp = NULL;

int endRead = 0;

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = bram_open,
	.read = bram_read,
	.write = bram_write,
	.release = bram_close,
};

static struct of_device_id bram_of_match[] =
{
  { .compatible = "bram_gpio", },
  { /* end of list */ },
};

static struct platform_driver bram_driver = {
  .driver = {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table	= bram_of_match,
  },
  .probe		= bram_probe,
  .remove		= bram_remove,
};

MODULE_DEVICE_TABLE(of, bram_of_match);

/*-------------------------------- PROBE & REMOVE -------------------------------------*/
static int bram_probe(struct platform_device *pdev)
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
  bp = (struct bram_info *) kmalloc(sizeof(struct bram_info), GFP_KERNEL);
  if (!bp) {
    printk(KERN_ALERT "Could not allocate led device!\n");
    return -ENOMEM;
  }
  // Put physical adresses in timer_info structure
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

  printk(KERN_WARNING "Bram platform driver registered.\n");
  return 0;//ALL OK

error2:
  release_mem_region(bp -> mem_start, bp -> mem_end - bp -> mem_start + 1);
error1:
  return rc;
}

static int bram_remove(struct platform_device *pdev)
{
  printk(KERN_WARNING "Bram platform driver removed!\n");
  iowrite32(0, bp->base_addr);
  printk(KERN_INFO "Bram remove in process.");
  iounmap(bp->base_addr);
  release_mem_region(bp->mem_start, bp->mem_end - bp->mem_start + 1);
  kfree(bp);
  printk(KERN_INFO "Bram driver removed.");
  return 0;
}

/*------------------------------------ FILE OPERATIONS ---------------------------------------*/

int bram_open(struct inode *pinode, struct file *pfile)
{
	printk(KERN_INFO "Succesfully opened bram_a.\n");
	return 0;
}

int bram_close(struct inode *pinode, struct file *pfile)
{
	printk(KERN_INFO "Succesfully closed bram_a.\n");
	return 0;
}
ssize_t bram_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	printk("Succefully read from bram_a. \n");
	return 0;
}

ssize_t bram_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {

  char buff[BUFF_SIZE];
  int ret = 0;
  int string_to_int = 0; 
  /*----------------- While variables ---------------*/
  char copy[100];
  int i = 0;
  int j = 0;
  int mat[10][10];

  unsigned long long iwrite = 3;

  char matrix_input[100];
  char *ch;
  int arr[50]; 
  int x = 0;
  int y = 0;
  int a,b,c,d,e,f;
  ch = matrix_input;
  /*-------------------------------------------------*/

  ret = copy_from_user(buff, buffer, length);
  if(ret){
    printk("Copy from user failed! \n");
    return -EFAULT;
  }
  buff[length] = '\0';  
  
  sscanf(buff,"%s", matrix_input);
 
  ret = kstrtoint(matrix_input,0,&string_to_int);
  printk (KERN_INFO "RET: %d\n", ret);
  
  if(ret != 0) {	
 
/*------------------------------ STRING TO ARRAY ---------------------------------*/
 
  printk(KERN_INFO "Matrix input: %s\n", matrix_input); 
  while(*ch != '\0') {
//	printk(KERN_INFO "Entered while: %s\n", ch);

	if(*ch == ';'){
		i++;
//		printk(KERN_INFO "Incremented i: %d\n", i);
	} else if (*ch == ',') {
		j++;
//		printk(KERN_INFO "Incremented j: %d\n", j);

	} else {
	//printk(KERN_INFO "AKLSJDALSKJDÉLKASJDÉLASKDJ");
		strncat(copy, ch, 2);
		y = kstrtoint(ch,0,&x);
		printk(KERN_INFO "CH = %c\n", *ch);
		arr[i+j] = *ch;
	}
	ch++;
  }

	printk(KERN_INFO "Number string: \n%s\n", copy);
	printk(KERN_INFO "\nNumber of rows: %d\n",i);
	printk(KERN_INFO "\nNumber of columns: %d\n",(j+i)/i);


	for(c = 0; c < j+i; c++) {
		printk(KERN_INFO "%d \t",arr[c]);
	}

	printk(KERN_INFO "\n\n Matrix  \n\n");

	d = 0;
	for(a = 0; a < i; a++) {
		for(b = 0; b < ((j+i)/i); b++){
			mat[a][b] = arr[d]-48;
			d++;
		} 
	}

	for(e = 0; e < i; e++) {
		for(f = 0; f < ((j+i)/i); f++) {
			printk(KERN_INFO "%d\t",mat[e][f]);
//			iowrite32(iwrite, bp -> base_addr);
//			printk(KERN_INFO "Succesfully wrote value %llu",iwrite); 
		}
		printk(KERN_INFO "\n\n");
	}

  }
	iowrite32((u32)iwrite, bp -> base_addr + 8);
	printk(KERN_INFO "Succesfully wrote value %llu",iwrite); 
	return length;
} 

	/*printk(KERN_INFO "This is in the buffer: %s\n", matrix_input);
	while((found = strsep(&pm,s)) != NULL) {
		printk(KERN_INFO "%s\n",found);
	}//while*/
	
/*------------------------------ END OF MATRIX PART -----------------------------*/

static int __init bram_init(void)
{
	int ret = 0;
	ret = alloc_chrdev_region(&my_dev_id, 0, 1, "bra");
	if (ret){
		printk(KERN_ERR "failed to register char device\n");
		return ret;
	}
	printk(KERN_INFO "Char device region allocated.\n");
	
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
	return platform_driver_register(&bram_driver);

	fail_2:
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
	fail_1:
		class_destroy(my_class);
	fail_0:
		unregister_chrdev_region(my_dev_id, 1);
	return -1;
}

static void __exit bram_exit(void)
{
	platform_driver_unregister(&bram_driver);
	cdev_del(my_cdev);
	device_destroy(my_class, my_dev_id);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id,1);
	printk(KERN_INFO "Bram_a exit.\n");
}

module_init(bram_init);
module_exit(bram_exit);
