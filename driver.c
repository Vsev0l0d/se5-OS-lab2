#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/vmalloc.h>

#include <linux/ioctl.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/pci.h>

#define WR_VALUE _IOW('a','a',struct message*)
#define MAX_PCI_DEV_COUNT 64

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Stab linux module for labs");
MODULE_VERSION("1.0");

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

struct pci_dev_info {
    unsigned short  device[MAX_PCI_DEV_COUNT];
    unsigned short  vendor[MAX_PCI_DEV_COUNT];
    int pci_dev_count;
};

struct signal_struct_info {
    bool valid;
    int     nr_threads;
    int     group_exit_code;
    int     notify_count;
    int     group_stop_count;
    unsigned int    flags;
};

struct message {
    struct signal_struct_info* ssi;
    struct pci_dev_info* pdi;
    pid_t pid;
};

struct pci_dev* pci_dev;
struct task_struct* ts;
struct message msg;

/*
** Function Prototypes
*/
static int      __init etx_driver_init(void);
static void     __exit etx_driver_exit(void);
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

void fill_structs(void);

/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .unlocked_ioctl = etx_ioctl,
        .release        = etx_release,
};

/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read Function\n");
        return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");
        return len;
}


/*
** This function will be called when we write IOCTL on the Device file
*/
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                        if( copy_from_user(&msg ,(struct message*) arg, sizeof(msg)) )
                        {
                                pr_err("Data Write : Err!\n");
                        }
                        pr_info("Pid = %d\n", msg.pid);
                        fill_structs();
                        break;
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}
 
/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&etx_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&etx_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        pr_info("Device Driver Insert...Done!!!\n");
        return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

void fill_structs() {
    int i = 0;
    while ((pci_dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pci_dev))) {
        if (i >= MAX_PCI_DEV_COUNT) break;
        msg.pdi->device[i] = pci_dev->device;
        msg.pdi->vendor[i] = pci_dev->vendor;
        i++;
    }

    ts = get_pid_task(find_get_pid(msg.pid), PIDTYPE_PID);
    if (ts == NULL){
        msg.ssi->valid = false;
    } else {
        msg.ssi->valid = true;
        msg.ssi->nr_threads = ts->signal->nr_threads;
        msg.ssi->group_exit_code = ts->signal->group_exit_code;
        msg.ssi->notify_count = ts->signal->notify_count;
        msg.ssi->group_stop_count = ts->signal->group_stop_count;
        msg.ssi->flags = ts->signal->flags;
    }

    msg.pdi->pci_dev_count = i;
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);