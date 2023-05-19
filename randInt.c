#include <linux/module.h>   // Init and exit
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>       // Allocating device number
#include <linux/device.h>   // Automatically creating device file
#include <linux/cdev.h>
#include <linux/uaccess.h>  // copy_to_user, copy_from_user
#include <linux/random.h>
#include <linux/string.h>


#define DRIVER_AUTHOR "7615_7689_7734"
#define DRIVER_DESC   "A module to read random integer"

struct randInt_driver {
    dev_t dev_num;
    struct class* dev_class;
    struct device* dev;
    struct cdev vcdev;
    unsigned int open_cnt;
} randInt_driver;


static int randInt_driver_open(struct inode* inode, struct file* flip)
{
    randInt_driver.open_cnt++;
    printk(KERN_INFO "Handle opened event (%d).\n", randInt_driver.open_cnt);
    return 0;
}

static int randInt_driver_release(struct inode* inode, struct file* flip)
{
    randInt_driver.open_cnt--;
    printk("Handle closed event (%d).\n", randInt_driver.open_cnt + 1);
    return 0;
}

static int randInt(void)
{
    unsigned int randNum = 0;
    get_random_bytes(&randNum, sizeof(randNum));
    randNum = randNum % __INT_MAX__;
    printk("Random integer: %u\n", randNum);
    return randNum;
}

static ssize_t randInt_driver_read(struct file* flip, char __user* userBuf, size_t len, loff_t *off)
{
    
    int numBytes = 0, num = 0;
    int count = 0, i = 0, j = 0;
    char temp, buffer[10];
    memset(buffer, 0, 10);

    if(*off > 0)
        return 0; // End

    printk("Driver: Read()\n");
    num = randInt();

    // Convert int to string
    while(num > 0)
    {
        if (count == 9)
            break;
        buffer[count++] = (num % 10) + '0';
        num /= 10;
    }

    numBytes = count;
    count--;
    // Reverse integer
    while(count > i)
    {
        temp = buffer[i];
        buffer[i] = buffer[count];
        buffer[count] = temp;
        i++; count--;
    }
    buffer[numBytes] = '\0';

    for (i = 0; i < numBytes; i++)
    {
        if (buffer[i] == '0')
        {
            for(j = i; j < numBytes; j++)
            {
                buffer[j] = buffer[j+1];
            }
            numBytes = j - 1;
        }
        break;
    }


    if(numBytes < 0)
        return -EFAULT;
    if(copy_to_user(userBuf, buffer, numBytes))
        return -EFAULT;

    (*off) += numBytes;
    return numBytes;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = randInt_driver_open,
    .release = randInt_driver_release,
    .read = randInt_driver_read,
};

static int __init randIntDriverInit(void)
{
    int ret = 0;
    // Allocate device number
    randInt_driver.dev_num = 0;
    ret = alloc_chrdev_region(&randInt_driver.dev_num, 0, 1, "randInt_device");
    if (ret < 0)
    {
        printk("Error: Failed to register device number dynamically.\n");
        return ret;
    }
    printk(KERN_INFO "<Major, Minor>: <%d, %d>\n", MAJOR(randInt_driver.dev_num), MINOR(randInt_driver.dev_num));

    randInt_driver.dev_class = class_create(THIS_MODULE, "class_randInt_dev");
    if (IS_ERR(randInt_driver.dev_class))
    {
        printk(KERN_INFO "Error: Failed to create a device class.\n");
        unregister_chrdev_region(randInt_driver.dev_num, 1);
        return -1;
    }
    randInt_driver.dev = device_create(randInt_driver.dev_class, NULL, randInt_driver.dev_num, NULL, "randInt_dev");
    if (IS_ERR(randInt_driver.dev))
    {
        printk(KERN_INFO "Error: Failed to create a device.\n");
        class_destroy(randInt_driver.dev_class);
        unregister_chrdev_region(randInt_driver.dev_num, 1);
        return -1;
    }

    cdev_init(&randInt_driver.vcdev, &fops);
    ret = cdev_add(&randInt_driver.vcdev, randInt_driver.dev_num, 1);
    if (ret < 0)
    {
        printk(KERN_INFO "Error: Failed to add a char device to the system.\n");
        device_destroy(randInt_driver.dev_class, randInt_driver.dev_num);
        class_destroy(randInt_driver.dev_class);
        unregister_chrdev_region(randInt_driver.dev_num, 1);
        return -1;
    }

    printk(KERN_INFO "Linux Kernel Module: Random integer (Successfully)\n");
    return 0;
}


static void __exit randIntDriverExit(void)
{
    cdev_del(&randInt_driver.vcdev);
    device_destroy(randInt_driver.dev_class, randInt_driver.dev_num);
    class_destroy(randInt_driver.dev_class);

    // De-allocate device number.
    unregister_chrdev_region(randInt_driver.dev_num, 1);
    printk(KERN_INFO "Ending program...\n");
}

module_init(randIntDriverInit);
module_exit(randIntDriverExit);

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("Test");
