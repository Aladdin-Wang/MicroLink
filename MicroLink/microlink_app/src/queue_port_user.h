#ifndef __IRQ_PORT_USER_H__
#define __IRQ_PORT_USER_H__
#include "board.h"

typedef uint32_t queue_global_interrupt_status_t;

static
inline 
queue_global_interrupt_status_t queue_port_disable_global_interrupt(void)
{
    queue_global_interrupt_status_t tStatus;
    
    /* get global interrupt status */
    /* disable global interrupt */
    /* return the status */
    tStatus = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    return tStatus;
    
}

static
inline  
void queue_port_resume_global_interrupt(queue_global_interrupt_status_t tStatus)
{
    restore_global_irq(tStatus);
    /* resume the stored global interrupt status */
}


#endif
/* EOF */


