#ifndef __ASM_H
#define __ASM_H

#define ASM_NL \
#define ENTRY(name) \
		.globl name ASM_NL \
		ALIGN ASM_NL \
		name:

#define END(name) \
		.size name, .-name

#endif