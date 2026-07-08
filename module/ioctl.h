#ifndef _PROCSCOPE_IOCTL_H_
#define _PROCSCOPE_IOCTL_H_

#include <linux/fs.h>

long procscope_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#endif
