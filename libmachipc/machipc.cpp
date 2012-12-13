#include "machipc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define KERN_FAILURE 5

static int g_fd = -1;
static pid_t g_pidOwner = 0;

int getMachIPCFd()
{
	if (g_fd == -1 || g_pidOwner != getpid())
	{
		g_pidOwner = getpid();
		g_fd = ::open(LINUX_MACH_DEV, O_RDWR | O_CLOEXEC);
	}
	
	return g_fd;
}

kern_return_t machCall(int callNo, void* arg)
{
	int fd = getMachIPCFd();
	if (fd == -1)
		return KERN_FAILURE;
	
	int rv = ::ioctl(fd, callNo, arg);
	if (rv == -1)
		return KERN_FAILURE;
	return rv;
}

kern_return_t mach_port_allocate(ipc_space_t task, mach_port_right_t right, mach_port_name_t* name)
{
	mach_port_allocate_args args = { task, right, name };
	
	return machCall(SYS_mach_port_allocate, &args);
}
