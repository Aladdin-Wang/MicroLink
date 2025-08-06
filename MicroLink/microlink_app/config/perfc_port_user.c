/****************************************************************************
*  Copyright 2024 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

/*============================ INCLUDES ======================================*/
#undef __PERF_COUNT_PLATFORM_SPECIFIC_HEADER__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define __IMPLEMENT_PERF_COUNTER
#include "perf_counter.h"
#include "hpm_mchtmr_drv.h"
#if defined(__IS_COMPILER_GCC__)
#   pragma GCC diagnostic ignored "-Wattributes"
#endif

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wconditional-uninitialized"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#endif


/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/* low level interface for porting */
extern
uint32_t perfc_port_get_system_timer_freq(void);
extern
int64_t perfc_port_get_system_timer_top(void);
extern
bool perfc_port_is_system_timer_ovf_pending(void);
extern
bool perfc_port_init_system_timer(bool bTimerOccupied);
extern
int64_t perfc_port_get_system_timer_elapsed(void);
extern
void perfc_port_clear_system_timer_ovf_pending(void);
extern
void perfc_port_stop_system_timer_counting(void);
extern
void perfc_port_clear_system_timer_counter(void);

/*============================ IMPLEMENTATION ================================*/
/*============================ INCLUDES ======================================*/

#if __PERFC_USE_USER_CUSTOM_PORTING__
 
bool perfc_port_init_system_timer(bool bIsTimeOccupied)
{
    bool bResult = true;
    do {
        if (bIsTimeOccupied) {
            break;
        }

        __IRQ_SAFE {
            /* Configure the system timer count with the longest possible period
             * clear counter 
             * Clear overflow pending flag
             * Enable interrupt if required
             * start counting
             */
             mchtmr_init_counter(HPM_MCHTMR,0);
        }
    } while(0);
    
    return true;
}

uint32_t perfc_port_get_system_timer_freq(void)
{
    /* return the system timer frequency */
    return clock_get_frequency(clock_mchtmr0);
}

bool perfc_port_is_system_timer_ovf_pending(void)
{
    /* whether the system timer overflow is pending */
    return false;
}

int64_t perfc_port_get_system_timer_top(void)
{
    /* the top value of the counting */
    return MCHTMR_MTIME_MTIME_MASK;
}

int64_t perfc_port_get_system_timer_elapsed(void)
{
    /* the elapsed count number since last overflow */
    return mchtmr_get_count(HPM_MCHTMR);
}

void perfc_port_clear_system_timer_ovf_pending(void)
{
    /* clear the overflow pending flag */
}

void perfc_port_stop_system_timer_counting(void)
{
    /* stop the system timer */
}

void perfc_port_clear_system_timer_counter(void)
{
    /* clear the system timer counter */
    mchtmr_init_counter(HPM_MCHTMR,0);
}


uintptr_t __perfc_port_get_sp(void)
{
    uintptr_t sp;
    __asm volatile("mv %0, sp" : "=r"(sp));
    return sp;
}


void __perfc_port_set_sp(uintptr_t nSP)
{
    __asm volatile("mv sp, %0" : : "r"(nSP));
}


#endif

