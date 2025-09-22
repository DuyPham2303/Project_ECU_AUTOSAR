/* bsw/services/os/arch/cortexm3_stm32f1/Os_Arch.h
 *
 * Cortex-M3 architecture layer for minimal AUTOSAR-like OS.
 * - Tick bring-up (SysTick @ tick_hz)
 * - Cooperative yield (SVC) + preemptive switch (PendSV)
 * - Start first task using PSP
 * - C ↔ ASM scheduler handshake
 */
#ifndef OS_ARCH_H
#define OS_ARCH_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------
 * Tick & IRQ priorities
 *  - Configures SysTick reload from SystemCoreClock/tick_hz
 *  - Sets priority: PendSV lowest, SysTick slightly above
 *  - Enables SysTick interrupt & counter
 * -------------------------------------------------------------- */
void Os_Arch_InitTick(uint32 tick_hz);

/* --------------------------------------------------------------
 * Cooperative yield from Thread mode.
 *  - Triggers SVC #0; SVC handler will pend PendSV for switching.
 *  - Safe to call from any thread context.
 * -------------------------------------------------------------- */
void Os_Arch_Yield(void);

/* --------------------------------------------------------------
 * Start first task in Thread mode using PSP.
 *  - 'psp' must point to a stack prebuilt with:
 *      [r4..r11 saved area][xPSR, PC, LR, r12, r3, r2, r1, r0]
 *  - Does not return.
 * -------------------------------------------------------------- */
void Os_Arch_StartFirstTask(uint32* psp);

/* --------------------------------------------------------------
 * ASM → C scheduler handshake (MUST be provided by OS core).
 *  - 'saved_psp' is PSP after ASM pushed r4..r11 of current task
 *  - Return PSP of the next task (stack already prepared)
 * -------------------------------------------------------------- */
uint32* Os_Arch_ResolveNextContext(uint32* saved_psp);

/* --------------------------------------------------------------
 * Optional helper: request a context switch after current ISR.
 *  - Pends PendSV by setting SCB->ICSR.PENDSVSET
 *  - Useful for preemptive round-robin from tick ISR.
 * -------------------------------------------------------------- */
static inline void Os_Arch_TriggerPendSV(void)
{
    /* SCB->ICSR address (0xE000ED04), bit 28 = PENDSVSET */
    *((volatile uint32*)0xE000ED04u) = (uint32)0x10000000u;
}

#ifdef __cplusplus
}
#endif

#endif /* OS_ARCH_H */
