#ifndef __FLASH_PORT_USER_H__
#define __FLASH_PORT_USER_H__
#include "board.h"

typedef uint32_t flash_global_interrupt_status_t;

static
inline 
flash_global_interrupt_status_t flash_port_disable_global_interrupt(void)
{
    flash_global_interrupt_status_t tStatus;
    
    /* get global interrupt status */
    /* disable global interrupt */
    /* return the status */
    tStatus = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    return tStatus;
    
}

static
inline  
void flash_port_resume_global_interrupt(flash_global_interrupt_status_t tStatus)
{
    restore_global_irq(tStatus);
    /* resume the stored global interrupt status */
}

/* ===================== Flash device Configuration ========================= */ 
typedef struct flash_algo_t flash_algo_t;
extern const  flash_algo_t  onchip_flash_device;
/* flash device table */
#ifndef FLASH_DEV_TABLE
    #define FLASH_DEV_TABLE                                          \
    {                                                                   \
        &onchip_flash_device                                            \
    };                                                                
#endif
/*============================ INCLUDES ======================================*/
/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
 

#endif
/* EOF */


