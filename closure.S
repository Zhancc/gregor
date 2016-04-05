# include "asm.h"

.globl spawn
spawn:
	call	create_job
	pushl	%eax
	call 	add_job
	subl 	$512, %esp
	fxsave   (%esp)
	pushf
	pushl 	%ecx
	pushl 	%edx
	pushl	%ebp
	pushl	%esi
	pushl 	%edi
	pushl 	%esp
	call 	reschedule
1:	
	jmp 	1b


.globl switch_context
switch_context:	
	# movl 	4(%esp), %ecx
	# movl 	(%ecx), %eax
	# pushf
	# addl	$24, %eax #edx,ebp,esi,edi,eax,eflag
	# addl 	$512, %eax #
	# pushl 	%eax
	# call 	internal_malloc
	# movl 	%eax, %edx
	# popl 	%eax


	# addl 	%eax, %ecx
	# movl 	(%esp), %eax
	# movl	%eax, (%ecx)

	movl 4(%esp), %esp
	# popa
	ret

# END(switch_context)

# ENTRY(save_context)	
.globl save_context
save_context:

	movl 4(%esp), %eax
	pusha 
	movl %esp, (%eax)
	popa
	movl 8(%esp), %eax
	popl %eax
	popl %eax
	popl %eax
	jmp *%eax

# END(save_context)