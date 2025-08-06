#ifndef SEGGER_SYSTEMVIEW_H
#define SEGGER_SYSTEMVIEW_H
#include "board.h"
#include "microboot.h"
#include "SEGGER_SYSVIEW_REC.h"

void read_system_and_send_usb(void);
uint32_t write_system_and_receive_usb(uint8_t inputChar);
#endif


