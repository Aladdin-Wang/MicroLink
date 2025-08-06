#include "pikaScript.h"
#include "PikaObj.h"
#include "PikaVM.h"
#include "swd_host.h"



void cmd_read_ram(PikaObj *self, PikaTuple* val)
{

}

int cmd_write_ram(PikaObj *self, PikaTuple* val)
{



    return 0;
}


void cmd_read_flash(PikaObj *self, PikaTuple* val)
{




}

int cmd_write_flash(PikaObj *self, PikaTuple* val)
{



    return 0;
}
void erase_flash_disk(void);
int cmd_bootloader(PikaObj *self)
{
    erase_flash_disk();
}


int cmd_set_swd_clock(PikaObj *self, int clock)
{

    return 0;
}