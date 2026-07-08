#include "chrdev.h"
#include "common.h"

#include <linux/init.h>
#include <linux/module.h>

#include "procfs.h"

static int __init procscope_init(void)
{
	int ret;

	ret = create_procscope_procfs();
	if (ret) {
		pr_err("failed to create procfs entries: %d\n", ret);
		goto err_out;
	}

	ret = chrdev_init();
	if (ret) {
		pr_err("chrdev_init failed\n");
		goto err_out;
	}

	pr_info("module initialized\n");
	return 0;

err_out:
	destroy_procscope_procfs();
	chrdev_exit();
	return ret;
}

static void __exit procscope_exit(void)
{
	destroy_procscope_procfs();
	chrdev_exit();
	pr_info("module exited\n");
}

module_init(procscope_init);
module_exit(procscope_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("procscope contributors");
MODULE_DESCRIPTION("Process scope kernel module");
