#ifndef MACH_PORTS_H
#define MACH_PORTS_H

//struct 

long mach_port_allocate(void* priv, unsigned long args);
long mach_port_deallocate(void* priv, unsigned long args);

#endif
