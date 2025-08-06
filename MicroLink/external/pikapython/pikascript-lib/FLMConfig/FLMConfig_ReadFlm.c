#include "pikaScript.h"
#include "flash_blob.h"
#include "board.h"
#include "ff.h"
#include "file_stream.h"

ATTR_RAMFUNC flash_blob_t tFlashBlob;

uint32_t target_flash_blob(char *pchFileName,int wFlashAddr,int wRamBase)
{
    FIL file;
    FRESULT res;
    UINT bytesRead;
    FSIZE_t fileSize, remainingSize;
    uint32_t level;
    // 打开FLM文件
    level = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    res = f_open(&file, pchFileName, FA_READ);
    restore_global_irq(level);
    if (res != FR_OK) {
        pika_platform_printf("Failed to open %s file: %d\n",pchFileName, res);
        return 0;
    }
    // 获取文件大小
    level = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    fileSize = f_size(&file);
    restore_global_irq(level);
    // 读取文件内容到 tFlashBlob.target
    level = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    res = f_read(&file, &tFlashBlob.target, sizeof(tFlashBlob.target), &bytesRead);
    restore_global_irq(level);
    if (res != FR_OK || bytesRead != sizeof(tFlashBlob.target)) {
        pika_platform_printf("Failed to read file: %d\n", res);
        f_close(&file);
        return 0;
    }
    // 计算剩余数据大小
    remainingSize = fileSize - sizeof(tFlashBlob.target);

    // 确保剩余大小不超过 prog_blob 缓冲区
    if (remainingSize > sizeof(tFlashBlob.prog_blob)) {
        pika_platform_printf("Warning: Remaining file size (%llu) exceeds prog_blob buffer size (%zu), truncating...\n", 
               remainingSize, sizeof(tFlashBlob.prog_blob));
        remainingSize = sizeof(tFlashBlob.prog_blob);  // 只读取最多能存的大小
    }
    // **偏移文件指针到 algo_blob 数据位置**
    level = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    res = f_lseek(&file, sizeof(tFlashBlob.target));
    restore_global_irq(level);
    if (res != FR_OK) {
        pika_platform_printf("Failed to seek to algo_blob position, error: %d\n", res);
        f_close(&file);
        return 0;
    }
    // 读取剩余的文件数据到 tFlashBlob.prog_blob
    level = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    res = f_read(&file, tFlashBlob.prog_blob, remainingSize, &bytesRead);
    restore_global_irq(level);
    if (res != FR_OK || bytesRead != remainingSize) {
        pika_platform_printf("Failed to read algo_blob, read %u bytes\n", bytesRead);
        f_close(&file);
        return 0;
    }
    level = disable_global_irq(CSR_MSTATUS_MIE_MASK);
    f_close(&file);
    restore_global_irq(level);
    tFlashBlob.target.init += wRamBase;
    tFlashBlob.target.uninit += wRamBase;    
    tFlashBlob.target.erase_chip += wRamBase; 
    tFlashBlob.target.erase_sector += wRamBase; 
    tFlashBlob.target.program_page += wRamBase;  
    if(tFlashBlob.target.verify != 0){
        tFlashBlob.target.verify += wRamBase;  
    }else{
        tFlashBlob.target.verify = 0;
    }
    if(tFlashBlob.target.read != 0){
        tFlashBlob.target.read += wRamBase;  
    }else{
        tFlashBlob.target.read = 0;
    }
    tFlashBlob.target.sys_call_s.static_base += wRamBase;  
    tFlashBlob.target.sys_call_s.breakpoint  += wRamBase;  
    tFlashBlob.target.sys_call_s.stack_pointer  += wRamBase; 
    tFlashBlob.target.program_buffer   += wRamBase;
    tFlashBlob.target.algo_start  += wRamBase;
    tFlashBlob.target.algo_blob = (uint32_t *)tFlashBlob.prog_blob;
    tFlashBlob.sector_info_length = fileSize - sizeof(tFlashBlob.target)-tFlashBlob.target.algo_size;

    //printf("\nInit_addr = 0x%x\r\n",tFlashBlob.target.init);
    //printf("uninit_addr = 0x%x\r\n",tFlashBlob.target.uninit);
    //printf("erase_chip_addr = 0x%x\r\n",tFlashBlob.target.erase_chip);
    //printf("erase_sector_addr = 0x%x\r\n",tFlashBlob.target.erase_sector);
    //printf("program_page_addr = 0x%x\r\n",tFlashBlob.target.program_page);
    //printf("read_addr = 0x%x\r\n",tFlashBlob.target.read);
    //printf("verify_addr = 0x%x\r\n",tFlashBlob.target.verify);

    //printf("static_base = 0x%x\r\n",tFlashBlob.target.sys_call_s.static_base);
    //printf("stack_pointer = 0x%x\r\n",tFlashBlob.target.sys_call_s.stack_pointer);
    //printf("algo_start = 0x%x\r\n",tFlashBlob.target.algo_start);
    //printf("algo_size = 0x%x\r\n",tFlashBlob.target.algo_size);
    //printf("program_buffer = 0x%x\r\n",tFlashBlob.target.program_buffer);
    //printf("program_buffer_size = 0x%x\r\n",tFlashBlob.target.program_buffer_size);


    return (uint32_t)fileSize;
}


int FLMConfig_ReadFlm_load(PikaObj *self, PikaTuple* val)
{
    int  wFlashAddr, wRamBase;  
    if(pikaTuple_getSize(val) != 3){
        pika_platform_printf("Wrong number of parameters\r\n");
    }
    Arg* arg = pikaTuple_getArg(val, 0);
    char *pchFileName = arg_getStr(arg);
    arg = pikaTuple_getArg(val, 1);
    wFlashAddr = (int)arg_getInt(arg);
    arg = pikaTuple_getArg(val, 2);
    wRamBase = (int)arg_getInt(arg);
    if(target_flash_blob(pchFileName,wFlashAddr,wRamBase)){
        set_bin_start_address(wFlashAddr);
        pika_platform_printf("Load FLM Success,FileName = %s, FlashAddr = 0x%x, RamBase = 0x%x\n",pchFileName,wFlashAddr, wRamBase);
    }
    return 0;
}



