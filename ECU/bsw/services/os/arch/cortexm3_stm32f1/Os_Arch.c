/* bsw/services/os/arch/cortexm3_stm32f1/Os_Arch.c
 *
 * Cortex-M3 architecture layer (STM32F1) for a minimal AUTOSAR-like OS.
 * Responsibilities:
 *  - Configure SysTick to generate the OS tick at `tick_hz`
 *  - Set exception priorities so that context switching via PendSV is smooth
 *  - Provide the SysTick IRQ handler that bridges into the OS core (Os_TickISR)
 *
 * Notes:
 *  - SVC and PendSV handlers are implemented in Os_Arch_Asm.S (ASM) for context switching.
 *  - This file only defines SysTick_Handler and the tick bring-up function.
 */

#include "Std_Types.h"
#include "Os_Arch.h"
#include "Os.h"
#include "stm32f10x.h"      /* CMSIS device header: SystemCoreClock, SysTick, NVIC_*, IRQn defs */

/* --------------------------------------------------------------------------
 * Exception priorities (0 = highest, 15 = lowest on STM32F1 with 4-bit prio):
 *   - PendSV: lowest priority (15) → runs after all other ISRs, ideal for context switch
 *   - SysTick: slightly above PendSV (14) → tick can pend a switch which happens right after
 *   - SVCall: above SysTick (13) → cooperative yield via SVC can request a switch cleanly
 * -------------------------------------------------------------------------- */
static void Os_Arch_SetCorePriorities(void)
{
    NVIC_SetPriority(PendSV_IRQn,  0x0F);  /* lowest */
    NVIC_SetPriority(SysTick_IRQn, 0x0E);  /* lower than most, higher than PendSV */
    NVIC_SetPriority(SVCall_IRQn,  0x0D);  /* above SysTick */
}

/* --------------------------------------------------------------------------
 * Configure SysTick to fire at `tick_hz`.
 * - Uses HCLK as the clock source.
 * - Ensures LOAD fits the 24-bit SysTick counter (clamps if necessary).
 * - Enables interrupt and the counter.
 * -------------------------------------------------------------------------- */
void Os_Arch_InitTick(uint32 tick_hz)
{
    uint32 hz = (tick_hz == 0u) ? 1000u : tick_hz;  /* default 1 kHz if invalid */

    /* Update priorities first (so first tick follows the intended scheme) */
    Os_Arch_SetCorePriorities();

    /* Compute reload from current core clock */
    /* If your startup doesn't call SystemInit()/SystemCoreClockUpdate() before main,
       ensure SystemCoreClock has the correct value here. */
    uint32 reload = SystemCoreClock / hz;
    if (reload == 0u) { reload = 1u; }
    if (reload > 0x00FFFFFFu) { reload = 0x00FFFFFFu; } /* clamp to 24-bit max */

    /* Stop SysTick while reconfiguring */
    SysTick->CTRL = 0u;

    /* Program reload and clear current value */
    SysTick->LOAD = (reload - 1u);
    SysTick->VAL  = 0u;

    /* CLKSOURCE = processor clock (HCLK), TICKINT = enable IRQ, ENABLE = start */
    SysTick->CTRL = (1u << 2) | (1u << 1) | (1u << 0);
}

/* --------------------------------------------------------------------------
 * SysTick IRQ → forward to OS core
 * The CPU automatically stacks the HW frame (r0-r3,r12,lr,pc,xPSR) on entry.
 * We simply notify the OS: increment counter, manage alarms, schedule callbacks.
 * Context switching (if enabled) will be requested by the OS using PendSV.
 * -------------------------------------------------------------------------- */
void SysTick_Handler(void)
{
    Os_TickISR();
}
