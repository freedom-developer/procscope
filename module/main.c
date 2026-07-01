#include "common.h"

#include <linux/init.h>
#include <linux/module.h>

#include "procfs.h"

static int __init procscope_init(void)
{
	int ret;

	ret = create_procscope_procfs();
	if (ret) {
		destroy_procscope_procfs();
		pr_err("failed to create procfs entries: %d\n", ret);
		return ret;
	}

	pr_info("module initialized\n");
	return 0;
}

static void __exit procscope_exit(void)
{
	destroy_procscope_procfs();
	pr_info("module exited\n");
}

module_init(procscope_init);
module_exit(procscope_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("procscope contributors");
MODULE_DESCRIPTION("Process scope kernel module");
