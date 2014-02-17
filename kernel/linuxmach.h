#ifndef LINUXMACH_H
#define LINUXMACH_H
#include <linux/list.h>
#include <linux/idr.h>
#include <linux/atomic.h>

typedef int kern_return_t;
typedef int mach_port_name_t;
typedef int mach_port_right_t;
typedef mach_port_name_t ipc_space_t;

#define PORT_TYPE_DEAD	0
#define PORT_TYPE_TASK	1

struct mach_task_data
{
	pid_t task_id; // tgid
	// mach port rights in this task (mach_port_insert_right)
	struct idr port_rights;
	// mach port send right for this task
	mach_port_right_t task_self;
	// TODO: thread list
	// TODO: bootstrap port
};

struct mach_port_task
{
	pid_t pid;
};

struct mach_port
{
	atomic_t refs;
	int type;

	union
	{
		// TODO: type specific data
		struct mach_port_task task;
	};

	struct list_head list;
};


struct mach_port_right
{
	struct mach_port* port;
	//mach_port_right_t id;
	int rights;
	//struct list_head* list;
};

#endif
