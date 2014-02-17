#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/idr.h>
#include "mach_ports.h"
#include "linuxmach.h"
#include "linuxmach_user.h"

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

static LIST_HEAD(list_ports); // TODO: change to rbtree (needed for task_for_pid())
static DEFINE_SPINLOCK(lock_ports);

static struct miscdevice mach_dev = {
	MISC_DYNAMIC_MINOR,
	"mach_ipc",
	&mach_chardev_ops,
};

static int mach_init(void)
{
	int err = misc_register(&mach_dev);
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
	misc_deregister(&mach_dev);
	printk(KERN_INFO "Darling Mach kernel emulation unloaded\n");
}

int mach_dev_open(struct inode* ino, struct file* file)
{
	struct mach_task_data* mpdata = NULL;
	struct mach_port* task_port = NULL;
	int err;

	mpdata = (struct mach_task_data*)
		kmalloc(sizeof(struct mach_task_data), GFP_KERNEL);

	if (mpdata == NULL)
	{
		err = -ENOSPC;
		goto fail;
	}

	task_port = (struct mach_port*)
		kmalloc(sizeof(struct mach_port), GFP_KERNEL);

	if (task_port == NULL)
	{
		err = -ENOSPC;
		goto fail;
	}

	file->private_data = mpdata;
	mpdata->task_id = current->tgid;

	idr_init(&mpdata->port_rights);

	// TODO: move some of the logic into mach_port_insert_right
	
	task_port->type = PORT_TYPE_TASK;
	task_port->task.pid = current->tgid;
	atomic_set(&task_port->refs, 1);

	do
	{
		if (!idr_pre_get(&mpdata->port_rights, GFP_KERNEL))
		{
			err = -ENOSPC;
			goto fail;
		}

		err = idr_get_new(&mpdata->port_rights, task_port, &mpdata->task_self);
	}
	while (err == -EAGAIN);

	spin_lock(&lock_ports);
	list_add(&task_port->list, &list_ports);
	spin_unlock(&lock_ports);

	return 0;
fail:

	if (mpdata != NULL)
		kfree(mpdata);

	return err;
}

int mach_dev_release(struct inode* ino, struct file* file)
{
	// TODO: change the task port type to dead (and later thread ports as well)
	// TODO: unref all ports this task has rights to
	kfree(file->private_data);
	return 0;
}

long mach_dev_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_paramv)
{
	struct mach_task_data* mpdata = (struct mach_task_data*) file->private_data;
	const unsigned int num_syscalls = sizeof(syscalls) / sizeof(syscalls[0]);

	if (mpdata->task_id != current->tgid)
	{
		/*
		 * The user-space implementation should have re-opened /dev/mach to get a new fd
		 * after fork()
		 */

		return -EBADF;
	}
	
	if (ioctl_num > num_syscalls)
		return -ENOSYS;

	if (!syscalls[ioctl_num-1])
		return -ENOSYS;
	
	return syscalls[ioctl_num-1](file->private_data, ioctl_paramv);
}

module_init(mach_init);
module_exit(mach_exit);

