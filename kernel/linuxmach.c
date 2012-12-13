#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/security.h>
#include "mach_ports.h"
#include "linuxmach.h"

MODULE_LICENSE("GPL");

int mach_dev_open(struct inode* ino, struct file* file);
int mach_dev_release(struct inode* ino, struct file* file);
long mach_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_paramv);

typedef long (*syscall_handler)(void*, ...);

static struct file_operations mach_chardev_ops = {
	.open           = mach_dev_open,
	.release        = mach_dev_release,
	.unlocked_ioctl = mach_dev_ioctl,
	.compat_ioctl   = mach_dev_ioctl,
	.owner          = THIS_MODULE
};
static const syscall_handler syscalls[] = {
	(syscall_handler) mach_port_allocate, // 1
	(syscall_handler) mach_port_deallocate // 2
};
static struct security_operations watched_ops = {
	.task_create = task_create_hook,
	.task_free = task_free_hook
};

//#define mach_MINOR 1

static struct miscdevice mach_dev = {
	MISC_DYNAMIC_MINOR,
	"mach_ipc",
	&mach_chardev_ops,
};

static int mach_init(void)
{
	//printk(KERN_ALERT "Linux Mach kernel emulation loaded\n");

	int err = register_security(&watched_ops);
	if (err == -EAGAIN)
	{
		printk(KERN_WARNING "Darling Mach kernel emulation failed to load.\n");
		printk(KERN_WARNING "You seem to have another LSM such as SELinux loaded. This is currently not supported.\n");
	}
	if (err < 0)
		goto fail;

	err = misc_register(&mach_dev);
	if (err < 0)
		goto fail;

	printk(KERN_INFO "Darling Mach kernel emulation loaded\n");
	return 0;

fail:
	printk(KERN_WARNING "Error loading Darling Mach: %d\n", err);
	return err;
}
static void mach_exit(void)
{
	reset_security_ops();
	misc_deregister(&mach_dev);
	printk(KERN_INFO "Darling Mach kernel emulation unloaded\n");
}

int mach_dev_open(struct inode* ino, struct file* file)
{
	return 0;
}

int mach_dev_release(struct inode* ino, struct file* file)
{
	return 0;
}

long mach_dev_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_paramv)
{
	const unsigned int num_syscalls = sizeof(syscalls) / sizeof(syscalls[0]);
	
	if (ioctl_num > num_syscalls)
		return -ENOSYS;

	if (!syscalls[ioctl_num-1])
		return -ENOSYS;
	
	return syscalls[ioctl_num-1](file->private_data, ioctl_paramv);
}

int task_create_hook(unsigned long clone_flags)
{
	return 0;
}

void task_free_hook(struct task_struct *task)
{
}

module_init(mach_init);
module_exit(mach_exit);
