; G8RTOS_SchedulerASM.s
; Holds all ASM functions needed for the scheduler
; Note: If you have an h file, do not have a C file and an S file of the same name

	; Functions Defined
	.def G8RTOS_Start, PendSV_Handler

	; Dependencies
	.ref CurrentlyRunningThread, G8RTOS_Scheduler

	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section

; Need to have the address defined in file 
; (label needs to be close enough to asm code to be reached with PC relative addressing)
RunningPtr: .field CurrentlyRunningThread, 32

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
G8RTOS_Start:

	.asmfunc

	; load the current thread
	ldr r0, RunningPtr
	ldr r1, [r0]
	ldr r0, [r1]

	ldr lr, [r0, #(14*4)]

	add r0, r0, #(34*4)
	msr psp, r0

	; switch to psp
	mov r0, #0x06
	msr control, r0


	CPSIE I

	bx lr
	.endasmfunc

; PendSV_Handler
; - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
PendSV_Handler:

	.asmfunc
	CPSID I
	push {lr}

	; Save remaining registers
	mrs r0, psp
	stmdb r0!, {r4-r11}

	; Save the current stack pointer to tcb
	; load r0 into tcb->sp
	ldr r2, RunningPtr
	ldr r1, [r2]
	str r0, [r1]

	; Call G8RTOS_Scheduler to get new tcb
	push {lr, r2}
	bl G8RTOS_Scheduler
	pop {lr, r2}

	; Set stack pointer to new stack pointer from new tcb
	; put tcb->sp in r0
	ldr r1, [r2]
	ldr r0, [r1]

	; Pop registers from thread stack
	ldmia r0!, {r4-r11}

	msr psp, r0

	CPSIE I
	pop {pc}
	.endasmfunc
	
	; end of the asm file
	.align
	.end
