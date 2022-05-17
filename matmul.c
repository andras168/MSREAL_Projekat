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

int matrix_dim_n, matrix_dim_m, matrix_dim_p;
int pos = 0;
int endRead = 0;
int storage[10];

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



int matmul_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened file\n");
		return 0;
}

int matmul_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed file\n");
		return 0;
}

ssize_t matmul_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	long int len;
	if (endRead){
		endRead = 0;
		pos = 0;
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
	}
	len = scnprintf(buff,BUFF_SIZE , "%d ", storage[pos]);
	ret = copy_to_user(buffer, buff, len);
	if(ret)
		return -EFAULT;
	pos ++;
	if (pos == 10) {
		endRead = 1;
	}
	return len;
}


ssize_t matmul_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	int n, m, p;
	int ret;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';

	ret = sscanf(buff,"%d,%d,%d",&n,&m,&p);

	if(ret==3) //three parameters parsed in sscanf
	{
		printk(KERN_INFO "Inside if.");
		if(n>=0 && n<=6)
		{
			matrix_dim_n = n;
			printk(KERN_INFO "Succesfully wrote value %d in position %d\n", n, 1); 
		} else {
			printk(KERN_WARNING "Dimension n should be between 0 and 6\n");
		}
		
		if(m>=0 && m<=6)
		{
			matrix_dim_m = m;
			printk(KERN_INFO "Succesfully wrote value %d in position %d\n", m, 2); 
		} else {
			printk(KERN_WARNING "Dimension m should be between 0 and 6\n");
		}
		
		if (p>=0 && p<=6)
		{
			matrix_dim_p = p;
			printk(KERN_INFO "Succesfully wrote value %d in position %d\n", p, 3); 
		} else {
			printk(KERN_WARNING "Dimension p should be between 0 and 6\n");
		}
		
		
	}
	return length;
}

static int __init matmul_init ( void )
{
	int ret = 0 ;
	ret = alloc_chrdev_region(&my_dev_id, 0 , 1 , "matmul" );
	if (ret){
		printk(KERN_ERR "failed to register char device\n" );
		return ret;
	}
	printk(KERN_INFO "char device region allocated\n" );

	my_class = class_create(THIS_MODULE, "matmul_class" );
	if (my_class == NULL ){
		printk(KERN_ERR "failed to create class\n" );
		goto fail_0;
	}
	printk(KERN_INFO "class created\n" );

	my_device = device_create(my_class, NULL , my_dev_id, NULL ,"matmul" );
	if (my_device == NULL ){
		printk(KERN_ERR "failed to create device\n" );
		goto fail_1;
	}
	printk(KERN_INFO "device created\n" );

	my_cdev = cdev_alloc();
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1 );
	if (ret)
	{
		printk(KERN_ERR "failed to add cdev\n" );
		goto fail_2;
	}
	printk(KERN_INFO "cdev added\n" );
	printk(KERN_INFO "Hello world\n" );
	return 0 ;
	
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
	cdev_del(my_cdev);
	device_destroy(my_class, my_dev_id);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id, 1 );
	printk(KERN_INFO "Goodbye, cruel world\n" );
}

module_init(matmul_init);
module_exit(matmul_exit);
