/*
 * Your code here for context_switch
 *
 * It should restores the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:
	popl %eax
	popl %eax
	movl %eax, %esp
	popal
	popl %es
	popl %ds
	add $8, %esp
	iret

spin:
	jmp spin
