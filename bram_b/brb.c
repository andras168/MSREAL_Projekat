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

#define DEVICE_NAME "BRAM_B"
#define DRIVER_NAME "bramb_driver"
#define BUFF_SIZE 100

/*----------------------------------- FUNCTION PROTOTYPES -------------------------------------*/
static int bramb_probe(struct platform_device *pdev);
static int bramb_remove(struct platform_device *pdev);

int bramb_open(struct inode *pinode, struct file *pfile);
int bramb_close(struct inode *pinode, struct file *pfile);
ssize_t bramb_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t bramb_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);
static int __init bramb_init(void);
static void __exit bramb_exit(void);


/*------------------------------------- GLOBAL VARIABLES --------------------------------------*/
struct bramb_info {
	unsigned long mem_start;
	unsigned long mem_end;
	void __iomem *base_addr;
};

static dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
static struct bramb_info *bp = NULL;

int endRead = 0;

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = bramb_open,
	.read = bramb_read,
	.write = bramb_write,
	.release = bramb_close,
};

static struct of_device_id bramb_of_match[] =
{
  { .compatible = "bram_gpio",
    .compatible = "xlnx,axi-bram-ctrl-B", },
  { /* end of list */ },
};

static struct platform_driver bramb_driver = {
  .driver = {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table	= bramb_of_match,
  },
  .probe		= bramb_probe,
  .remove		= bramb_remove,
};

MODULE_DEVICE_TABLE(of, bramb_of_match);

/*-------------------------------- PROBE & REMOVE -------------------------------------*/
static int bramb_probe(struct platform_device *pdev)
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
  bp = (struct bramb_info *) kmalloc(sizeof(struct bramb_info), GFP_KERNEL);
  if (!bp) {
    printk(KERN_ALERT "Could not allocate led device!\n");
    return -ENOMEM;
  }
  // Put physical adresses in bramb_info structure
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

  printk(KERN_WARNING "BRAM_B platform driver registered.\n");
  return 0;//ALL OK

error2:
  release_mem_region(bp -> mem_start, bp -> mem_end - bp -> mem_start + 1);
error1:
  return rc;
}

static int bramb_remove(struct platform_device *pdev)
{
  printk(KERN_WARNING "BRAM_B platform driver removed!\n");
  iowrite32(0, bp->base_addr);
  printk(KERN_INFO "BRAM_B remove in process.");
  iounmap(bp->base_addr);
  release_mem_region(bp->mem_start, bp->mem_end - bp->mem_start + 1);
  kfree(bp);
  printk(KERN_INFO "BRAM_B driver removed.");
  return 0;
}

/*------------------------------------ FILE OPERATIONS ---------------------------------------*/

int bramb_open(struct inode *pinode, struct file *pfile)
{
	printk(KERN_INFO "Succesfully opened BRAM_B.\n");
	return 0;
}

int bramb_close(struct inode *pinode, struct file *pfile)
{
	printk(KERN_INFO "Succesfully closed BRAM_B.\n");
	return 0;
}
ssize_t bramb_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int i;
	int ret;
	char buff[BUFF_SIZE];
        uint32_t matrix;
	
	long int len;
	if (endRead){
		endRead = 0;
		printk(KERN_INFO "Succesfully read from file.\n");
		return 0;
	}
	
	ret = copy_to_user(buffer, buff, len);	
	
	for(i=0; i <= 8; i++) {
		matrix = ioread32(bp->base_addr+(4*i));
		printk(KERN_INFO "matrix[%d]= %u",i ,matrix);
	}

	endRead = 1;
	return len;

}

ssize_t bramb_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {

  char buff[BUFF_SIZE];
  int ret = 0;
  int string_to_int = 0; 
  int row;
  int column;
  int total_elements;
  int total_elements_count;  
  char copy[100];
  int i = 0;
  int j = 0;
  int k = 0;
  int mat[10][10];
  int q = 0;
  char matrix_input[100];
  char *ch;
  int arr[50]; 
  int x;
  int c = 0;
  int a,b,d,e,f;
  ch = matrix_input;

  ret = copy_from_user(buff, buffer, length);
  if(ret){
    printk("Copy from user failed! \n");
    return -EFAULT;
  }
  buff[length] = '\0';  
  
  sscanf(buff,"%s", matrix_input);
 
  ret = kstrtoint(matrix_input,0,&string_to_int);
  //printk (KERN_INFO "RET: %d\n", ret);
  
  if(ret != 0) {	
 
/*------------------------------ STRING TO ARRAY ---------------------------------*/
 
  printk(KERN_WARNING "Matrix input: %s\n", matrix_input); 

	for(i = 0; matrix_input[i]; i++) {
		x = kstrtoint(copy, 0, &c);
	
		if(c <= 4096) {  //Restricting the range of values
		
			if(matrix_input[i]==',') {

				arr[j+k] = c;
				strcpy(copy, "");
				j++;
			}
			else if (matrix_input[i]==';') {
				arr[j+k] = c;
				strcpy(copy, "");
				k++;
			} else {
				strncat(copy,&matrix_input[i],1);	
			}
		} else {
			printk(KERN_ERR "WARNING! Only values between 0 and 4096 are allowed. Please input the matrix again with correct values.");
			return -1;
		}
			
	}
	
	
	//printk(KERN_INFO "\n\n Here are the values: \n");
	
	for(q=0; q < j+k; q++) {
		//printk(KERN_INFO "%d\t",arr[q]);
		total_elements_count++;
	}	


	printk(KERN_INFO "Number of rows: %d\n",k);
	printk(KERN_INFO "Number of columns: %d\n",(j+k)/k);

	row = k;
	column = (j+k)/k;
	total_elements = row * column;
	printk(KERN_WARNING "Total elements = %d\n", total_elements);


	printk(KERN_INFO "\nMatrix\n");
	if(total_elements == total_elements_count) {
		d = 0;
		for(a = 0; a < k; a++) {
			for(b = 0; b < ((j+k)/k); b++){
				mat[a][b] = arr[d];
				d++;
			} 
		}

		for(e = 0; e < k; e++) {
			for(f = 0; f < ((j+k)/k); f++) {
				printk(KERN_INFO "%d\t",mat[e][f]);
				if(e == 0) {	
					iowrite32((u32)mat[e][f], bp -> base_addr+4*f);
				} else {
					iowrite32((u32)mat[e][f], bp -> base_addr+((4*(j+k)/k*e)+(4*f)));	
				}	
			
			}
			printk(KERN_INFO "\n\n");
		}
	} else {
		printk(KERN_ERR "Elements are missing from matrix, please define it again with correct element count!");
		return -1;
	}

  }
	return length;
} 

	
/*------------------------------ END OF MATRIX PART -----------------------------*/

static int __init bramb_init(void)
{
	int ret = 0;
	ret = alloc_chrdev_region(&my_dev_id, 1 , 1, "brb");
	if (ret){
		printk(KERN_ERR "Failed to register char device.\n");
		return ret;
	}
	printk(KERN_INFO "Char device region allocated.\n");
	
	my_class = class_create(THIS_MODULE, "brb_class");
	if (my_class == NULL){
		printk(KERN_ERR "Failed to create class!\n");
		goto fail_0;
	}
	printk(KERN_INFO "Class created.\n");
	
	my_device = device_create(my_class, NULL, my_dev_id, NULL, "brb");
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

	return platform_driver_register(&bramb_driver);

	fail_2:
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
	fail_1:
		class_destroy(my_class);
	fail_0:
		unregister_chrdev_region(my_dev_id, 1);
	return -1;
}

static void __exit bramb_exit(void)
{
	platform_driver_unregister(&bramb_driver);
	cdev_del(my_cdev);
	device_destroy(my_class, my_dev_id);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id,1);
	printk(KERN_WARNING "BRAM_B removed!\n");
}

module_init(bramb_init);
module_exit(bramb_exit);
