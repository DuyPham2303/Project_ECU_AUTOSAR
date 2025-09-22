/* bsw/services/os/arch/cortexm3_stm32f1/Os_Arch_Asm.S
 *
 * Cortex-M3 context switch support for a minimal AUTOSAR-like OS.
 * - PendSV_Handler: save current (r4-r11), call C scheduler to get next PSP, restore (r4-r11), return
 * - SVC_Handler:    request PendSV to perform a cooperative yield
 * - Os_Arch_Yield:  user-callable wrapper that triggers SVC 0
 * - Os_Arch_StartFirstTask(sp): start first task in thread mode using PSP (EXC_RETURN)
 *
 * Expected C symbol (MUST be provided by OS core):
 *   uint32* Os_Arch_ResolveNextContext(uint32* saved_psp);
 *     - Input : saved_psp = current PSP after pushing {r4-r11}
 *     - Return: next_psp  = PSP of next task (stack already prepared with {r4-r11} + HW exception frame)
 */

    .syntax unified
    .cpu cortex-m3
    .thumb

    .extern Os_Arch_ResolveNextContext

/* ------------------------------------------------------
 *  PendSV_Handler  — context switch
 *  Priority: lowest (set in Os_Arch_InitTick via NVIC_SetPriority(PendSV_IRQn, LOW))
 *  Assumptions:
 *   - Thread stacks use PSP
 *   - HW already stacked {r0-r3,r12,lr,pc,xPSR} for preempted thread (or prebuilt for a fresh task)
 *   - We save/restore {r4-r11} here
 * ------------------------------------------------------ */
    .global PendSV_Handler
    .type   PendSV_Handler, %function
PendSV_Handler:
    /* Read current PSP into r0 */
    mrs     r0, psp

    /* Save callee-saved registers r4-r11 on current thread stack */
    stmdb   r0!, {r4-r11}

    /* Call C scheduler: r0 = saved_psp; returns r0 = next_psp */
    bl      Os_Arch_ResolveNextContext

    /* r0 = next_psp to restore from */
    /* Restore r4-r11 of next thread */
    ldmia   r0!, {r4-r11}

    /* Write PSP with updated value */
    msr     psp, r0

    /* Return from exception → pops HW-stacked frame of next thread
       (r0-r3,r12,lr,pc,xPSR) and resumes in thread mode using PSP. */
    bx      lr

/* ------------------------------------------------------
 *  SVC_Handler — cooperative yield via SVC 0
 *  Simple: just pend PendSV so the real switch happens after SVC exit.
 * ------------------------------------------------------ */
    .global SVC_Handler
    .type   SVC_Handler, %function
SVC_Handler:
    /* ICSR (Interrupt Control and State Register) address */
    ldr     r0, =0xE000ED04      /* SCB->ICSR */
    ldr     r1, =0x10000000      /* ICSR_PENDSVSET (bit 28) */
    str     r1, [r0]             /* set PENDSVSET */
    bx      lr

/* ------------------------------------------------------
 *  Os_Arch_Yield() — user API: trigger SVC to request a switch
 *  Prototype (C): void Os_Arch_Yield(void);
 * ------------------------------------------------------ */
    .global Os_Arch_Yield
    .thumb_func
Os_Arch_Yield:
    svc     #0
    bx      lr

/* ------------------------------------------------------
 *  Os_Arch_StartFirstTask(sp) — start scheduler on first task
 *  Prototype (C): void Os_Arch_StartFirstTask(uint32* sp);
 *  - r0 = PSP for the first task (its stack must contain a valid HW exception frame)
 *  Steps:
 *   1) Set PSP = r0
 *   2) Switch Thread to use PSP (CONTROL.SPSEL=1)
 *   3) EXC_RETURN to Thread/PSP: lr = 0xFFFFFFFD, bx lr
 * ------------------------------------------------------ */
    .global Os_Arch_StartFirstTask
    .thumb_func
Os_Arch_StartFirstTask:
    /* Set PSP to task's stack pointer */
    msr     psp, r0

    /* Use PSP in Thread mode: CONTROL.SPSEL=1 */
    movs    r0, #2
    msr     control, r0
    isb

    /* EXC_RETURN: Thread mode, use PSP, return to non-floating-point context */
    ldr     r0, =0xFFFFFFFD
    bx      r0
