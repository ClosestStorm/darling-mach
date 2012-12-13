#ifndef MACHIPC_H
#define MACHIPC_H
#include "../linuxmach.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LINUX_MACH_DEV "/dev/mach"

int getMachIPCFd();
kern_return_t machCall(int callNo, void* arg);

kern_return_t mach_port_allocate(ipc_space_t task, mach_port_right_t right, mach_port_name_t* name);


#ifdef __cplusplus
}
#endif

#endif
