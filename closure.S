.globl spawn
spawn:
	call	create_job
	pushl	%eax
	call 	set_next_job
	popl	%eax
	call 	reschedule
	ret
.size spawn, .-spawn

.globl reschedule, reschedule_from_pthread
reschedule:
	# subl 	$512, %esp
	# fxsave   (%esp)
	# call fstate_save
reschedule_from_pthread:
	# pushf they are caller saved
	# pushl 	%ecx
	# pushl 	%edx
	pushl	%ebp
	pushl	%esi
	pushl 	%edi
	pushl 	%esp #push esp before this's executed
	call   do_reschedule
	#reschedule should never retrun
1:	
	jmp 	1b
.size reschedule, .-reschedule
.size reschedule_from_pthread, .-reschedule_from_pthread

.globl gregor_main
gregor_main:
	pushl %esp
	call do_gregor_main
	popl %eax
	ret
.size gregor_main, .-gregor_main

.globl swicth_free_current
swicth_free_current:
	movl 4(%esp), %esp
	call free_current
	popl %edi
	popl %esi
	popl %ebp
	# popl %edx they are caller saved
	# popl %ecx
	# popf
	ret
.size swicth_free_current, .-swicth_free_current
	

.globl switch_context, switch_context_to_new
switch_context:	
	movl 4(%esp), %esp
	popl %edi
	popl %esi
	popl %ebp
	# popl %edx they are caller saved
	# popl %ecx
	# popf
	call do_reschedule_reset_current	
	# call fstate_restore
	ret
switch_context_to_new:
	movl 4(%esp), %esp
	call do_reschedule_reset_current
	ret
.size switch_context, .-switch_context
.size switch_context_to_new, .-switch_context_to_new

# END(switch_context)


.globl cleanup
cleanup:
	pushl %edx
	pushl %eax
	call do_cleanup
1:  #cleanup should never return
	jmp 	1b
.size cleanup, .-cleanup