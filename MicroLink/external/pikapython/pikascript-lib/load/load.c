#include "pikaScript.h"
#include "PikaObj.h"
#include "PikaVM.h"
#include "ff.h"
#include "diskio.h"
#include "file_stream.h"
#include "board.h"
#include "hpm_gpio_drv.h"
#include "perf_counter.h"
#undef this
#define this    (*ptThis)

extern PikaObj* pikaMain;
static shared_state_t shared_state;

ATTR_RAMFUNC_WITH_ALIGNMENT(4) 
static uint8_t bin_buffer[4096];

int load_bin_flash(char *pchFileName,int  wFlashAddr)
{
    static FIL file;
    static FRESULT res;
    static UINT bytesRead = 0;
    static FSIZE_t fileSize = 0, remainingSize = 0;
    static FSIZE_t offset = 0;

    uint32_t level;
    start_cycle_counter();
    level = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    res = f_open(&file, pchFileName, FA_READ);
    restore_global_irq(level);
    if (res != FR_OK) {
        pika_platform_printf("Failed to open %s file: %d\n",pchFileName, res);
        return -1;
    }
    level = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    fileSize = f_size(&file);
    restore_global_irq(level);
    pika_platform_printf("open fileName: %s success,file size: %d byte\n",pchFileName,fileSize);

    if(ERROR_SUCCESS != open_bin(&shared_state)){
        pika_platform_printf("Failed to open flash load blob\n");
        f_close(&file);
        close_bin(&shared_state);   
        return -1;
    }

    remainingSize = fileSize;
    set_bin_start_address(wFlashAddr);

    while (remainingSize > 0) {
        level = disable_global_irq(CSR_MSTATUS_MIE_MASK);
        f_lseek(&file, offset);
        res = f_read(&file, bin_buffer, sizeof(bin_buffer), &bytesRead);
        restore_global_irq(level);
        if (res != FR_OK) {
            f_close(&file);
            pika_platform_printf("Failed to read file: %d\n", res);
            return -1;
        }
        if(ERROR_SUCCESS_DONE_OR_CONTINUE != write_bin(&shared_state, wFlashAddr, bin_buffer, bytesRead)){
            f_close(&file);
            close_bin(&shared_state);   
            pika_platform_printf("Failed to write file,addr: 0x%x\n", wFlashAddr);
            return -1;
        }
        wFlashAddr += bytesRead;
        remainingSize -= bytesRead;
        offset += bytesRead;
        int progress = (int)((int)(fileSize - remainingSize) * 100) / (int)fileSize;
        pika_platform_printf("Download: %3d%% ,used %d ms\r\n", progress,(uint32_t)perfc_convert_ticks_to_ms(stop_cycle_counter()));
    } 
    offset = 0;
    f_close(&file);
    close_bin(&shared_state);   
    pika_platform_printf(" %s loaded successfully.\n", pchFileName);
    return 0;
}

int load_bin(PikaObj *self, PikaTuple* val)
{
    FIL file;
    FRESULT res;
    UINT bytesRead;
    FSIZE_t fileSize, remainingSize;
    int val_num = pikaTuple_getSize(val);
    if( val_num % 2 != 0){
        pika_platform_printf("Wrong number of parameters %d\r\n",val_num);
        return -1;
    }
    for(int i = 0;i < val_num;i = i+2){
        Arg* arg = pikaTuple_getArg(val, 0 + i);
        char *pchFileName = arg_getStr(arg);
        arg = pikaTuple_getArg(val, 1 + i);
        int wFlashAddr = (int)arg_getInt(arg);
        pika_platform_printf("fileName = %s, FlashAddr = 0x%x\n",pchFileName,wFlashAddr);
        if( 0 != load_bin_flash(pchFileName,wFlashAddr)){
            return -1;
        }
    }
    return 0;
}

int load_hex(PikaObj *self, PikaTuple* val)
{


    return 0;
}

void load_offline(PikaObj *self, PikaTuple* val)
{
    if(pikaMain){
        pikaVM_runSingleFile(pikaMain,"Python/offline_download.py");
    }
}

