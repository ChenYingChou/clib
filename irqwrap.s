/* irqwrap.s */

#define IRQ_STACKS		8

#define IRQ_SIZE		16
#define IRQ_HANDLER		0
#define IRQ_NUMBER		4
#define IRQ_OLDVEC		8

#define STACK_IRQ_HANDLER	44
#define CHAIN_CS		44
#define CHAIN_IP		40

.text

.globl	__irq_wrapper_0
.globl	__irq_wrapper_1
.globl	__irq_wrapper_2
.globl	__irq_wrapper_3

	.align 4
__irq_wrapper_0:
	pushl	$__irq_handler + IRQ_SIZE*0
	jmp	irq_start

	.align 2
__irq_wrapper_1:
	pushl	$__irq_handler + IRQ_SIZE*1
	jmp	irq_start

	.align 2
__irq_wrapper_2:
	pushl	$__irq_handler + IRQ_SIZE*2
	jmp	irq_start

	.align 2
__irq_wrapper_3:
	pushl	$__irq_handler + IRQ_SIZE*3
	jmp	irq_start

	.align 2
irq_start:
	pushl	%eax				/* reserved for chain */

	pushw	%ds				/* save registers */
	pushw	%es
	pushw	%fs
	pushw	%gs
	pushal

	.byte	0x2e
	movw	___djgpp_ds_alias, %ax
	movw	%ax, %ds			/* set up selectors */
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	movl	$IRQ_STACKS-1, %ecx		/* look for a free stack */

stack_search_loop:
	leal	__irq_stack(,%ecx,4), %ebx
	cmpl	$0, (%ebx)
	jnz	found_stack			/* found one! */

	decl	%ecx
	jge	stack_search_loop
	jmp	get_out 			/* oh shit.. */

found_stack:
	movl	STACK_IRQ_HANDLER(%esp), %esi	/* __irq_handler[] */
	movl	%esp, %ecx			/* old stack in ecx + dx */
	movw	%ss, %dx

	movw	%ax, %ss
	movl	(%ebx), %esp			/* set up our stack */
	movl	$0, (%ebx)			/* flag the stack is in use */

	pushl	%ebx				/* __irq_stack[]   */
	pushl	%esi				/* __irq_handler[] */
	pushl	%edx				/* old ss	   */
	pushl	%ecx				/* old esp	   */

	cld					/* clear the direction flag */
	call	IRQ_HANDLER(%esi)		/* call the C handler */

	popl	%ecx				/* restore the old stack */
	popl	%edx
	popl	%esi				/* __irq_handler[] */
	popl	%ebx				/* __irq_stack[] */

	movl	%esp, (%ebx)
	movw	%dx, %ss
	movl	%ecx, %esp

	orl	%eax, %eax			/* check return value */
	jz	get_out

	/* store __handler[].irq_oldvec to stack */
	movl	IRQ_OLDVEC(%esi), %ebx		/* eip of old vector */
	movl	%ebx, CHAIN_IP(%esp)
	movzwl	IRQ_OLDVEC+4(%esi), %ebx	/* cs of old vector */
	movl	%ebx, CHAIN_CS(%esp)

	popal					/* chain to old handler */
	popw	%gs
	popw	%fs
	popw	%es
	popw	%ds
	lret

get_out:
	movb	$32, %al
	cmpl	$7, IRQ_NUMBER(%esi)
	jle	get_out1
	outb	%al, $160
get_out1:
	outb	%al, $32
	popal
	popw	%gs
	popw	%fs
	popw	%es
	popw	%ds
	add	$8, %esp			/* discard chain space */
	sti
	iret


.globl	__irq_wrapper_0_end
	.align 4
__irq_wrapper_0_end:
	ret

