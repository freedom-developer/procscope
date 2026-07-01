#ifndef _PROCSCOPE_PROCFS_H_
#define _PROCSCOPE_PROCFS_H_

#include <linux/proc_fs.h>

int create_procscope_procfs(void);
void destroy_procscope_procfs(void);

#endif
