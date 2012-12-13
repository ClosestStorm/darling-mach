#ifndef LINUXMACH_H
#define LINUXMACH_H

typedef int kern_return_t;
typedef int mach_port_name_t;
typedef int mach_port_right_t;
typedef mach_port_name_t ipc_space_t;

#define SYS_mach_port_allocate 1
#define SYS_mach_port_deallocate 2

struct mach_port_allocate_args
{
	ipc_space_t task;
	mach_port_right_t right;
	mach_port_name_t* name;
};

int task_create_hook(unsigned long clone_flags);
void task_free_hook(struct task_struct *task);

#endif
