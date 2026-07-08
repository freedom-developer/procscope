#include "chrdev.h"
#include "common.h"
#include "ioctl.h"

#include <linux/cdev.h>

#define CDEV_BASEMINOR 0
#define CDEV_COUNT 2
#define CDEV_NAME "procscope_cdev"

// todo: 定义我的字符设备结构
struct procscope_cdev {

    struct cdev cdev;
};

static struct procscope_cdev procscope_cdev; 

static struct file_operations procscope_cdev_fops = {
    .unlocked_ioctl = procscope_ioctl,
};

static struct class *procscope_class;
static dev_t start_devt = 0;

int chrdev_init(void)
{
    int i;

    // 1. 申请设备号
    int ret = alloc_chrdev_region(&start_devt, CDEV_BASEMINOR, CDEV_COUNT, CDEV_NAME);
    if (ret) {
        pr_err("alloc_chrdev_region failed: %d\n", ret);
        goto err_out;
    }

    // 2. 初始化cdev
    cdev_init(&procscope_cdev.cdev, &procscope_cdev_fops);

    // 3. 添加cdev
    ret = cdev_add(&procscope_cdev.cdev, start_devt, CDEV_COUNT);
    if (ret) {
        pr_err("cdev_add failed\n");
        goto err_out;
    }

    // 4. 创建设备文件
    procscope_class = class_create("procscope");
    if (!procscope_class) {
        pr_err("class_create failed\n");
        ret = -EINVAL;
        goto err_out;
    }
    for (i = 0; i < CDEV_COUNT; i++) {
        dev_t devt = MKDEV(MAJOR(start_devt), MINOR(start_devt) + i);
        struct device *dev = device_create(procscope_class, NULL, devt, &procscope_cdev.cdev, "%s", "procscope_cdev%d", i);
        if (IS_ERR(dev)) {
            pr_err("device_create %s%d failed", "procscope_cdev", i);
            ret = -EINVAL;
            goto err_out;
        }

    }

    return 0;
err_out:
    chrdev_exit();
    return ret;
}

void chrdev_exit(void)
{
    int i;
    unregister_chrdev_region(start_devt, CDEV_COUNT);

    if (procscope_class) {
        class_destroy(procscope_class);
        procscope_class = NULL;
    }

    for (i = 0; i < CDEV_COUNT; i++) {
        dev_t devt = MKDEV(MAJOR(start_devt), MINOR(start_devt) + i);
        device_destroy(procscope_class, devt);
    }
    
}