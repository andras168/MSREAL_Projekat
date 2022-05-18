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

#define DEVICE_NAME "bram"
#define DRIVER_NAME "brama_driver"
#define BUFF_SIZE 100

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
int brama_open(struct inode *pinode, struct file *pfile);
int brama_close(struct inode *pinode, struct file *pfile);
ssize_t brama_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t brama_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = brama_open,
	.read = brama_read,
	.write = brama_write,
	.release = brama_close,
};


// # FILE OPERATIONS #

int brama_open(struct inode *pinode, struct file *pfile)
{
	printk(KERN_INFO "Succesfully opened bram. a\n");
	return 0;
}

int brama_close(struct inode *pinode, struct file *pfile)
{
	printk(KERN_INFO "Succesfully closed bram. a\n");
	return 0;
}
ssize_t brama_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	printk(KERN_INFO "Succesfully read from bram. a\n");
	return 0;
}
ssize_t brama_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{

  char buff[BUFF_SIZE];
  int ret = 0;
  char *token;
  const char s[2] = ";";

  char *matrix_input;
//  const int SIZE=100;
//  int matrix[SIZE];
  ret = copy_from_user(buff, buffer, length);
  if(ret){
    printk("Copy from user failed! \n");
    return -EFAULT;
  }
  buff[length] = '\0';

  ret = sscanf(buff,"%s", matrix_input);

  //get the first token
  token = strsep(&matrix_input, s);

  //walk through other tokens
   while(token != NULL) {
      printk("%s\n", token); //print each token
      token = strsep(NULL, s);
   }
 /*
// !!! STRING TO MATRIX !!!

  if(ret != -EINVAL)//checking for parsing error
  {
  for(int i=0; i<SIZE; i++) {
    if (matrix[i] > 4096)
    {
      printk(KERN_WARNING "Element value should be between 0 and 4096 \n");
    }
    else
    {
      printk(KERN_WARNING "Element value  ok!\n");
     // iowrite32((256*ypos + xpos)*4, vp->base_addr + 8);
     // iowrite32(rgb, vp->base_addr);
    } }}
  else
  {
    printk(KERN_WARNING "Bram A_write: Wrong write format\n");
    // return -EINVAL;//parsing error
  } */
	return length;
}


// # init + exit # 

static int __init brama_init(void)
{
	int ret = 0;
	ret = alloc_chrdev_region(&my_dev_id, 0, 1, "brama");
	if (ret){
		printk(KERN_ERR "failed to register char device\n");
		return ret;
	}
	printk(KERN_INFO "char device region allocated\n");
	
	my_class = class_create(THIS_MODULE, "brama_class");
	if (my_class == NULL){
		printk(KERN_ERR "Failed to create class!\n");
		goto fail_0;
	}
	printk(KERN_INFO "Class created.\n");
	
	my_device = device_create(my_class, NULL, my_dev_id, NULL, "brama");
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

	//printk(KERN_INFO "brama world\n");
	return 0;

	fail_2:
		device_destroy(my_class, my_dev_id);
	fail_1:
		class_destroy(my_class);
	fail_0:
		unregister_chrdev_region(my_dev_id, 1);
	return -1;
}
static void __exit brama_exit(void)
{
	cdev_del(my_cdev);
	device_destroy(my_class, my_dev_id);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id,1);
	printk(KERN_INFO "Bram_a exit.\n");
}

module_init(brama_init);
module_exit(brama_exit)


