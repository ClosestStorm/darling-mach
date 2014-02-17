#ifndef LINUXMACH_USER_H
#define LINUXMACH_USER_H

typedef int kern_return_t;
typedef int mach_port_name_t;
typedef int mach_port_right_t;
typedef mach_port_name_t ipc_space_t;

struct mach_port;
struct mach_port_right;

#define SYS_mach_port_allocate 1
#define SYS_mach_port_deallocate 2

struct mach_port_allocate_args
{
	ipc_space_t task;
	mach_port_right_t right;
	mach_port_name_t* name;
};

#endif
