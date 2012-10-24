		NAME	irqwrap

true		equ	1
false		equ	0

IFDEF	M386
		    .386p

	IRQ_STACKS	EQU	8

	IRQ_SIZE	EQU	16
	IRQ_HANDLER	EQU	0
	IRQ_NUMBER	EQU	4
	IRQ_OLDVEC	EQU	8

	STK_IRQ_HANDLER EQU	52
	CHAIN_CS	EQU	52
	CHAIN_IP	EQU	48
ELSE
    ifdef   __TINY__
	    LCODE   equ     false
	    LDATA   equ     false
    endif

    ifdef   __SMALL__
	    LCODE   equ     false
	    LDATA   equ     false
    endif

    ifdef   __MEDIUM__
	    LCODE   equ     true
	    LDATA   equ     false
    endif

    ifdef   __COMPACT__
	    LCODE   equ     false
	    LDATA   equ     true
    endif

    ifdef   __LARGE__
	    LCODE   equ     true
	    LDATA   equ     true
    endif

    ifdef   __HUGE__
	    LCODE   equ     true
	    LDATA   equ     true
    endif

    IFDEF __WASM__
	IRQ_STACKS	EQU	4
    ELSE
      IF LDATA
	IRQ_STACKS	EQU	8
      ELSE
	IRQ_STACKS	EQU	4
      ENDIF
    ENDIF

    IF LCODE
	IRQ_SIZE	EQU	10
	IRQ_HANDLER	EQU	0
	IRQ_NUMBER	EQU	4
	IRQ_OLDVEC	EQU	6
    ELSE
	IRQ_SIZE	EQU	8
	IRQ_HANDLER	EQU	0
	IRQ_NUMBER	EQU	2
	IRQ_OLDVEC	EQU	4
    ENDIF

	ORG_DI		EQU	18
	CHAIN_CS	EQU	20
	CHAIN_IP	EQU	18
ENDIF
;==============================================================================
IFDEF	__WASM__
    PubCode	MACRO	Name
		PUBLIC	&Name&_
	&Name&_:
		ENDM

    ExtCode	MACRO	Name,Type
	   IFNB <Type>
		EXTRN	&Name&_:&Type
	   ELSE
		EXTRN	&Name&_:PROC
	   ENDIF
	&Name	EQU	&Name&_
		ENDM
ELSE	;__TASM__
    PubCode	MACRO	Name
		PUBLIC	_&Name
	_&Name&:
		ENDM

    ExtCode	MACRO	Name,Type
	   IFNB <Type>
		EXTRN	_&Name&:&Type
	   ELSE
		EXTRN	_&Name&:PROC
	   ENDIF
	&Name	EQU	_&Name
		ENDM
ENDIF

ExtData 	MACRO	Name,Type
	   IFNB <Type>
		EXTRN	_&Name&:&Type
	   ELSE
		EXTRN	_&Name&:BYTE
	   ENDIF
	&Name	EQU	_&Name
		ENDM
;==============================================================================
DGROUP		GROUP	_DATA
IFDEF	M386
_DATA		SEGMENT DWORD PUBLIC USE32 'DATA'
ELSE
_DATA		SEGMENT WORD PUBLIC 'DATA'
ENDIF
		ExtData _irq_stack,BYTE
		ExtData _irq_handler,WORD
_DATA		ENDS
;==============================================================================
IFDEF	M386
_TEXT		SEGMENT DWORD PUBLIC USE32 'CODE'
		ASSUME	CS:_TEXT,DS:DGROUP,SS:DGROUP

PubCode _irq_wrapper_0
		push	0*IRQ_SIZE
		jmp	irq_start

PubCode _irq_wrapper_1
		push	1*IRQ_SIZE
		jmp	irq_start

PubCode _irq_wrapper_2
		push	2*IRQ_SIZE
		jmp	irq_start

PubCode _irq_wrapper_3
		push	3*IRQ_SIZE
		jmp	irq_start

irq_start:
		push	eax			      ; reserved for chain
		push	ds			      ; save registers
		push	es
		push	fs
		push	gs
		pushad

		mov	ax,DGROUP
		mov	ds,ax			      ; set up selectors
IFDEF SET_ALL_SEGS
		mov	es,ax
		mov	fs,ax
		mov	gs,ax
ENDIF
		; WASM can't accept "op reg/mem,offset EXTRN:data+Constant"
		mov	ebx,dword ptr [esp].STK_IRQ_HANDLER
		add	ebx,offset _irq_handler       ; _irq_handler[]

		mov	ecx,IRQ_STACKS-1	      ; look for a free stack
stack_search_loop:
		lea	esi,_irq_stack[4*ecx]
		cmp	dword ptr [esi],0
		jnz	found_stack		      ; found one!

		dec	ecx
		jge	stack_search_loop
		jmp	irq_chain		      ; out of stack, chain to old

found_stack:
		mov	ecx,esp 		      ; old stack in (dx:ecx)
		mov	dx,ss

		mov	ss,ax
		mov	esp,dword ptr [esi]	      ; set up our stack
		mov	dword ptr [esi],0	      ; flag the stack is in use

		push	esi			      ; _irq_stack[]
		push	ebx			      ; _irq_handler[]
		push	edx			      ; ss
		push	ecx			      ; esp

		cld				      ; clear the direction flag
		call	dword ptr [ebx].IRQ_HANDLER   ; call the C handler

		pop	ecx			      ; restore the old stack
		pop	edx
		pop	ebx			      ; _irq_handler[]
		pop	esi			      ; _irq_stack[]

		mov	dword ptr [esi],esp	      ; flag the stack is free
		mov	ss,dx			      ; restore the old stack
		mov	esp,ecx

		or	eax,eax
		jz	irq_exit

irq_chain:	; store _handler[].irq_oldvec to stack
		mov	eax,dword ptr [ebx].IRQ_OLDVEC	; eip of old vector
		mov	dword ptr [esp].CHAIN_IP,eax
		movzx	eax,word ptr [ebx].IRQ_OLDVEC+4 ; cs of old vector
		mov	dword ptr [esp].CHAIN_CS,eax

		popad				      ; chain to old handler
		pop	gs
		pop	fs
		pop	es
		pop	ds
		retf				      ; chain

irq_exit:
		mov	al,20h			      ; EOI
		cmp	dword ptr [ebx].IRQ_NUMBER,7
		jle	irq_exit1
		out	0a0h,al 		      ; 8259A-slaver
irq_exit1:
		out	020h,al 		      ; 8259A-master
		popad
		pop	gs
		pop	fs
		pop	es
		pop	ds
		add	esp,8			      ; discard chain space
		sti
		iretd

PubCode _irq_wrapper_0_end
		ret
;------------------------------------------------------------------------------
ELSE
;------------------------------------------------------------------------------
_TEXT		SEGMENT BYTE PUBLIC 'CODE'
		ASSUME	CS:_TEXT,DS:DGROUP,SS:DGROUP

PubCode _irq_wrapper_0
		push	di
	IFDEF __WASM__
		mov	di,0*IRQ_SIZE
	ELSE
		mov	di,offset _irq_handler+0*IRQ_SIZE
	ENDIF
		jmp	irq_start

PubCode _irq_wrapper_1
		push	di
	IFDEF __WASM__
		mov	di,1*IRQ_SIZE
	ELSE
		mov	di,offset _irq_handler+1*IRQ_SIZE
	ENDIF
		jmp	irq_start

PubCode _irq_wrapper_2
		push	di
	IFDEF __WASM__
		mov	di,2*IRQ_SIZE
	ELSE
		mov	di,offset _irq_handler+2*IRQ_SIZE
	ENDIF
		jmp	irq_start

PubCode _irq_wrapper_3
		push	di
	IFDEF __WASM__
		mov	di,3*IRQ_SIZE
	ELSE
		mov	di,offset _irq_handler+3*IRQ_SIZE
	ENDIF
;;;	       jmp     irq_start

irq_start:
		push	ax			      ; reserved for chain IP

		push	ds			      ; save registers
		push	es

		push	ax
		push	cx
		push	dx
		push	bx
		push	bp
		push	si
		mov	bp,sp
		push	[bp].ORG_DI		      ; push org. DI

		mov	ax,DGROUP
		mov	ds,ax			      ; set up data segment
IFDEF SET_ALL_SEGS
		mov	es,ax
ENDIF

	IFDEF __WASM__
		add	di,offset _irq_handler	      ; _irq_handler[]
	ENDIF

	IF LDATA
		mov	si,4*(IRQ_STACKS-1)	      ; look for a free stack
	ELSE
		mov	si,2*(IRQ_STACKS-1)	      ; look for a free stack
	ENDIF
stack_search_loop:
		lea	bx,_irq_stack[si]
	IF LDATA
		mov	ax,word ptr [bx+2]	      ; new SS
		or	ax,ax
	ELSE
		cmp	word ptr [bx],0 	      ; only offset
	ENDIF
		jnz	found_stack		      ; found one!

	IF LDATA
		sub	si,4
	ELSE
		sub	si,2
	ENDIF
		jge	stack_search_loop
		jmp	irq_chain		      ; out of stack

found_stack:
		mov	bp,sp			      ; old stack in (dx:bp)
		mov	dx,ss

		mov	ss,ax			      ; set up our stack
		mov	sp,word ptr [bx]
	IF LDATA
		mov	word ptr [bx+2],0	      ; flag the stack is in use
	ELSE
		mov	word ptr [bx],0 	      ; flag the stack is in use
	ENDIF

		push	bx			      ; _irq_stack[]
		push	ds		; Warning: WATCOM/C may destory DS in C/L/H
		push	di			      ; _irq_handler[]
		push	dx			      ; ss
		push	bp			      ; sp

		cld				      ; clear the direction flag
	IF LCODE
		call	dword ptr [di].IRQ_HANDLER    ; call the C handler
	ELSE
		call	word ptr [di].IRQ_HANDLER     ; call the C handler
	ENDIF

		pop	bp			      ; sp
		pop	dx			      ; ss
		pop	di			      ; _irq_handler[]
		pop	ds			      ; DGROUP
		pop	bx			      ; _irq_stack[]

	IF LDATA
		mov	word ptr [bx+2],ss	      ; flag the stack is free
	ELSE
		mov	word ptr [bx],sp	      ; flag the stack is free
	ENDIF
		mov	ss,dx			      ; restore the old stack
		mov	sp,bp

		or	ax,ax
		jz	irq_exit		      ; return (not chain)

irq_chain:	; store _handler[].irq_oldvec to stack
		mov	ax,word ptr [di].IRQ_OLDVEC   ; ip of old vector
		mov	word ptr [bp].CHAIN_IP,ax
		mov	ax,word ptr [di].IRQ_OLDVEC+2 ; cs of old vector
		mov	word ptr [bp].CHAIN_CS,ax

		pop	di
		pop	si
		pop	bp
		pop	bx
		pop	dx
		pop	cx
		pop	ax

		pop	es
		pop	ds
		retf				      ; chain

irq_exit:
		mov	al,20h
		cmp	byte ptr [di].IRQ_NUMBER,7
		jle	irq_exit1
		out	0a0h,al
irq_exit1:
		out	020h,al

		pop	di
		pop	si
		pop	bp
		pop	bx
		pop	dx
		pop	cx
		pop	ax

		pop	es
		pop	ds
		add	sp,4			      ; discard chain space
		iret

PubCode _irq_wrapper_0_end
		ret

ENDIF

_TEXT		ENDS
;==============================================================================
		END
