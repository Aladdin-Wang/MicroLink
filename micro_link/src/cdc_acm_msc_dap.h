#ifndef DAP_MAIN_H
#define DAP_MAIN_H
#include "usb_osal.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_msc.h"
#include "chry_ringbuffer.h"

extern chry_ringbuffer_t g_uartrx;
extern chry_ringbuffer_t g_usbrx;
extern usb_osal_sem_t semaphore_acm_tx_done, semaphore_dap_tx_done;


void chry_dap_init(uint8_t busid, uint32_t reg_base);
void chry_dap_handle(uint8_t busid);
void chry_dap_usb2uart_handle(uint8_t busid);

void chry_dap_usb2uart_uart_config_callback(struct cdc_line_coding *line_coding);
void chry_dap_usb2uart_uart_send_bydma(uint8_t *data, uint16_t len);
void chry_dap_usb2uart_uart_send_complete(uint32_t size);

#endif