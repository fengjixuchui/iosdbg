#ifndef _MEMUTILS_H_
#define _MEMUTILS_H_

#include <mach/vm_types.h>

extern int PM;

unsigned int CFSwapInt32(unsigned int);
unsigned long long CFSwapInt64(unsigned long long);

kern_return_t disassemble_at_location(unsigned long, int, char **);
kern_return_t dump_memory(unsigned long, vm_size_t, char **);
kern_return_t read_memory_at_location(unsigned long, void *, vm_size_t);
kern_return_t write_memory_to_location(vm_address_t, vm_offset_t, vm_size_t);
kern_return_t valid_location(long);

#endif
