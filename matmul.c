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
			printk(KERN_INFO "Succesfully wrote value %d in position %d\n", m, 1); 
		} else {
			printk(KERN_WARNING "Dimension m should be between 0 and 6\n");
		}
		
		if (p>=0 && p<=6)
		{
			matrix_dim_p = p;
			printk(KERN_INFO "Succesfully wrote value %d in position %d\n", p, 1); 
		} else {
			printk(KERN_WARNING "Dimension p should be between 0 and 6\n");
		}
		
		
	}
	return length;
}

static int __init matmul_start(void)
{
	printk(KERN_INFO "Loading matmul module...\n");
	printk(KERN_INFO "Matmul loaded.\n");
return 0;
}

static void __exit matmul_end(void)
{
printk(KERN_INFO "Matmul removed.\n");
}

module_init(matmul_start);
module_exit(matmul_end);
