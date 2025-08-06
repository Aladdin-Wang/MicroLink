#include "pikaScript.h"
#include "SEGGER_SystemView.h"
#include "SEGGER_RTTView.h"
#include "swd_host.h"

/*********************************************************************
*
*       Static variables
*
**********************************************************************
*/

static enum {
  WAIT_FOR_HOST_HELLO1_S,
  WAIT_FOR_HOST_HELLO2_V,
  WAIT_FOR_HOST_HELLO3_VERSION1,
  WAIT_FOR_HOST_HELLO4_VERSION2,
  SEND_CLIENT_HELLO1_S,
  SEND_CLIENT_HELLO2_V,
  SEND_CLIENT_HELLO3_VERSION1,
  SEND_CLIENT_HELLO4_VERSION2,
  RECORDING
} _State = WAIT_FOR_HOST_HELLO1_S;

/*********************************************************************
*
*       Private data types
*
**********************************************************************
*/
//
// Commands that Host can send to target
//
typedef enum {
  SEGGER_SYSVIEW_COMMAND_ID_START = 1,
  SEGGER_SYSVIEW_COMMAND_ID_STOP,
  SEGGER_SYSVIEW_COMMAND_ID_GET_SYSTIME,
  SEGGER_SYSVIEW_COMMAND_ID_GET_TASKLIST,
  SEGGER_SYSVIEW_COMMAND_ID_GET_SYSDESC,
  SEGGER_SYSVIEW_COMMAND_ID_GET_NUMMODULES,
  SEGGER_SYSVIEW_COMMAND_ID_GET_MODULEDESC,
  // Extended commands: Commands >= 128 have a second parameter
  SEGGER_SYSVIEW_COMMAND_ID_GET_MODULE = 128
} SEGGER_SYSVIEW_COMMAND_ID;
static uint32_t segger_rtt_addr = 0;

#define RTT_MAX_BUFFER_SIZE 4096

extern uint8_t rtt_buffer[RTT_MAX_BUFFER_SIZE];

extern rtt_data_msg_t  tRTTMsgObj;
extern int64_t get_system_time_ms(void);

static SEGGER_RTT_CB _SEGGER_RTT;
static uint32_t RTT_wAddr,RTT_wSize,RTT_wChannel;

static byte_queue_t       s_tByteQueue;
static fsm(check_string)  s_fsmCheckStr;
static get_byte_t         s_tGetByte;
static uint8_t            s_chByteBuf[64];
void SystemView_init(uint32_t wAddr,uint32_t wSize);
static uint16_t receive_usb_get_byte(get_byte_t *ptThis,uint8_t *pchByte, uint16_t hwLength)
{
    return peek_queue(&s_tByteQueue, pchByte, hwLength);
}

void read_system_and_send_usb(void)
{
    SEGGER_RTT_BUFFER_UP up_buffer;
    uint32_t len = 0; 
    static uint16_t read_delay_ms = 1;
    static uint8_t read_err_count = 0;
    static int64_t expected_ticks = 0;

    if (segger_rtt_addr != 0) {
        if ((get_system_time_ms()) > expected_ticks) {
            do {
                uint32_t up_addr = segger_rtt_addr + offsetof(SEGGER_RTT_CB, aUp[RTT_wChannel]);

                // 读取 RTT UpBuffer 结构
                if (!swd_read_memory(up_addr, (uint8_t*)&up_buffer, sizeof(SEGGER_RTT_BUFFER_UP))){
                    read_delay_ms = 1000;
                    read_err_count++;
                    break;
                }

                uint32_t buffer_addr = (uint32_t)up_buffer.pBuffer;  // RTT 缓冲区地址
                uint32_t size = up_buffer.SizeOfBuffer;
                uint32_t wrOff = up_buffer.WrOff;
                uint32_t rdOff = up_buffer.RdOff;

                // 检查是否有新数据
                if (wrOff != rdOff) {
                    len = (wrOff > rdOff) ? (wrOff - rdOff) : (size - rdOff);

                    if (len > RTT_MAX_BUFFER_SIZE) len = RTT_MAX_BUFFER_SIZE;

                    // **确保不会跨越缓冲区**
                    if (!swd_read_memory(buffer_addr + rdOff, rtt_buffer, len)) {
                        read_delay_ms = 1000;
                        read_err_count++;
                        break;
                    }
                    // **如果 `rdOff + len` 触及缓冲区末尾，继续读取缓冲区头部数据**
                    if (wrOff < rdOff) {
                        uint32_t second_part_len = wrOff;
                        if (second_part_len > RTT_MAX_BUFFER_SIZE - len) 
                            second_part_len = RTT_MAX_BUFFER_SIZE - len;
                        
                        if (!swd_read_memory(buffer_addr, rtt_buffer + len, second_part_len)) {
                            read_delay_ms = 1000;
                            read_err_count++;
                            break;
                        }
                        len += second_part_len;
                    }
                    emit(rtt_sig, &tRTTMsgObj,
                        args(
                            rtt_buffer,
                            len
                        ));

                    // **逐步更新 RdOff 而不是直接设为 wrOff**
                    rdOff = (rdOff + len) % size;
                    if (!swd_write_word(up_addr + offsetof(SEGGER_RTT_BUFFER_UP, RdOff), rdOff)){
                        read_delay_ms = 1000;
                        read_err_count++;
                        break;
                    }
                }
                read_delay_ms = 1;
                read_err_count = 0;
            } while(0);
            expected_ticks = get_system_time_ms() + read_delay_ms;
        }
    }
    if(read_err_count > 0) {
        read_err_count = 0;
        SystemView_init(RTT_wAddr, RTT_wSize);
    }
}

uint32_t write_system_and_receive_usb(uint8_t inputChar)
{
    uint8_t buffer[16];
    SEGGER_RTT_BUFFER_DOWN down_buffer;
    
    if (segger_rtt_addr != 0) {
        do {
            uint32_t down_addr = segger_rtt_addr + offsetof(SEGGER_RTT_CB, aDown[RTT_wChannel]);
            // 读取 RTT 控制块，确保 RTT 仍然有效
            if (swd_read_memory(segger_rtt_addr, buffer, 16)) {
                if (memcmp(buffer, RTT_SIGNATURE, 10) != 0) {
                     break; 
                }
            }
            // 读取 RTT DownBuffer 结构
            if (!swd_read_memory(down_addr, (uint8_t*)&down_buffer, sizeof(SEGGER_RTT_BUFFER_DOWN))) {
                break;
            }

            uint32_t buffer_addr = (uint32_t)down_buffer.pBuffer;
            uint32_t size = down_buffer.SizeOfBuffer;
            uint32_t wrOff = down_buffer.WrOff;
            uint32_t rdOff = down_buffer.RdOff;

            // **再次读取 `RdOff` 以确认其是否已更新**
            uint32_t new_rdOff;
            if (!swd_read_word(down_addr + offsetof(SEGGER_RTT_BUFFER_DOWN, RdOff), &new_rdOff)) {
                break;
            }

            if (new_rdOff != rdOff) {
                // 说明 `RdOff` 已更新，重新检查 `wrOff` 是否仍然有效
                rdOff = new_rdOff;
            }
            // **检查缓冲区是否已满**
            uint32_t nextWrOff = (wrOff + 1) % size;
            if (nextWrOff == rdOff) {
                printf("r\n wrOff = %x,rdOff = %x,size = %x\r\n",wrOff,rdOff,size);
                break; // 缓冲区满，丢弃数据
            }

            // **确保 `wrOff` 不会超出缓冲区**
            uint8_t tempBuffer[4] = { inputChar, 0, 0, 0 }; // 预留 4 字节
            if (!swd_write_memory(buffer_addr + wrOff, tempBuffer, 1)) {
                break;
            }

            // **更新 WrOff**
            wrOff = nextWrOff;
            if (!swd_write_word(down_addr + offsetof(SEGGER_RTT_BUFFER_DOWN, WrOff), wrOff)) {
                break;
            }
            //printf("r\n add = 0x%x,wrOff = %x,rdOff = %x,inputChar= %x\r\n",down_addr + offsetof(SEGGER_RTT_BUFFER_DOWN, WrOff),wrOff,rdOff,inputChar);

        } while(0);
    }
    return segger_rtt_addr;
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       SYSVIEW_REC_GetOutgoing()
*
*  Function description
*    Get outgoing data from SystemView buffer to be transmitted.
*
*  Parameters
*    pBuf     - Buffer to store data to.
*    BufSize  - Size of buffer.
*
*  Return value
*    > 0: Number of bytes stored to the buffer.
*    = 0: No data available.
*    < 0: Error (e.g. connection was not opened yet).
*/
int SYSVIEW_REC_GetOutgoing(void) 
{
  static const uint8_t HelloResponse[4] = {'S', 'V', SEND_CLIENT_HELLO3_VERSION1, SEND_CLIENT_HELLO4_VERSION2};
  int RetVal;
  uint8_t *pBuf;

    if (segger_rtt_addr != 0) {
      switch (_State) {
      case RECORDING:
        read_system_and_send_usb();
        break;
      case SEND_CLIENT_HELLO4_VERSION2:
        *pBuf = HelloResponse[3];
        RetVal = 1;
        _State = RECORDING;
        emit(rtt_sig, &tRTTMsgObj,
            args(
                pBuf,
                RetVal
            ));
        break;
      case SEND_CLIENT_HELLO3_VERSION1:
        *pBuf = HelloResponse[2];
        RetVal = 1;
        _State = SEND_CLIENT_HELLO4_VERSION2;
        emit(rtt_sig, &tRTTMsgObj,
            args(
                pBuf,
                RetVal
            ));
        break;
      case SEND_CLIENT_HELLO2_V:
        *pBuf = HelloResponse[1];
        RetVal = 1;
        _State = SEND_CLIENT_HELLO3_VERSION1;
        emit(rtt_sig, &tRTTMsgObj,
            args(
                pBuf,
                RetVal
            ));
        break;
      case SEND_CLIENT_HELLO1_S:
          *pBuf = HelloResponse[0];
          RetVal = 1;
          _State = SEND_CLIENT_HELLO2_V;
          emit(rtt_sig, &tRTTMsgObj,
              args(
                  pBuf,
                  RetVal
              ));
        break;
      case WAIT_FOR_HOST_HELLO4_VERSION2: // fall through!
      case WAIT_FOR_HOST_HELLO3_VERSION1: // fall through!
      case WAIT_FOR_HOST_HELLO2_V:        // fall through!
      case WAIT_FOR_HOST_HELLO1_S:        // fall through from above!
        RetVal = -1;  // connection not yet established
        break;
      }
        
  }

  return RetVal;
}

/*********************************************************************
*
*       SYSVIEW_REC_ProcessIncoming()
*
*  Function description
*    Event handler to be called upon reveiving incoming data.
*
*  Parameters
*    pBytes   - Buffer containing received data.
*    NumBytes - Number of bytes in the buffer.
*
*  Return value
*    > 0: OK. Response ready to be sent.
*    = 0: OK.
*    < 0: Error.
*/
int SYSVIEW_REC_ProcessIncoming(uint8_t inputChar) {
    uint8_t Byte;
    if (segger_rtt_addr != 0) {
      Byte = inputChar;
      printf("_State = %d,inputChar = %d\r\n",_State,inputChar);
      switch (_State) {
        case RECORDING:
          if(inputChar == SEGGER_SYSVIEW_COMMAND_ID_STOP){
              _State = WAIT_FOR_HOST_HELLO1_S;
          }
          write_system_and_receive_usb(inputChar);
          break;
        case SEND_CLIENT_HELLO4_VERSION2: // fall through!
        case SEND_CLIENT_HELLO3_VERSION1: // fall through!
        case SEND_CLIENT_HELLO2_V:        // fall through!
        case SEND_CLIENT_HELLO1_S:        // fall through from above!
          break;
        case WAIT_FOR_HOST_HELLO4_VERSION2:
          _State = SEND_CLIENT_HELLO1_S;          // we don't care about the clients version
          break;
        case WAIT_FOR_HOST_HELLO3_VERSION1:
          _State = WAIT_FOR_HOST_HELLO4_VERSION2; // we don't care about the clients version
          break;
        case WAIT_FOR_HOST_HELLO2_V:
          if ('V' == Byte) {
            _State = WAIT_FOR_HOST_HELLO3_VERSION1;
          } else {
            _State = WAIT_FOR_HOST_HELLO1_S;
          }
          break;
        case WAIT_FOR_HOST_HELLO1_S:
          if ('S' == Byte) {
            _State = WAIT_FOR_HOST_HELLO2_V;
          } else {
            _State = WAIT_FOR_HOST_HELLO1_S;
          }
          break;
      }
      enqueue(&s_tByteQueue,inputChar);
      fsm_rt_t tFsm = call_fsm( check_string, &s_fsmCheckStr);
      if(fsm_rt_cpl == tFsm) {
          get_all_peeked(&s_tByteQueue);
          uint32_t rtt_addr = segger_rtt_addr;
          segger_rtt_addr = 0;
          return rtt_addr;
      }else if(fsm_rt_user_req_drop == tFsm) {
          dequeue(&s_tByteQueue,&inputChar);
      }else {
          reset_peek(&s_tByteQueue);
      }     
    }
  return segger_rtt_addr;
}

void SystemView_init(uint32_t wAddr,uint32_t wSize)
{
    uint8_t buffer[16];
    segger_rtt_addr = 0;

    if (!swd_init_debug()) {
        pika_platform_printf("swd init error\r\n");
        return;
    }
    queue_init(&s_tByteQueue, s_chByteBuf, sizeof(s_chByteBuf));
    s_tGetByte.pTarget = &s_tByteQueue;
    s_tGetByte.fnGetByte = receive_usb_get_byte;
    init_fsm(check_string, &s_fsmCheckStr, args("SystemView.stop()", &s_tGetByte));
    clock_cpu_delay_ms(10);
    for (uint32_t rtt_addr = wAddr; rtt_addr < wAddr + wSize; rtt_addr += 4) {
        if (swd_read_memory(rtt_addr, buffer, 16)) {
            //for(uint8_t i = 0 ;i < 16;i++){
            //    pika_platform_printf("%02x ",buffer[i]);
            //}
            //pika_platform_printf("\r\n");
            if (memcmp(buffer, RTT_SIGNATURE, 10) == 0) {
                segger_rtt_addr =  rtt_addr;// 找到了 RTT 控制块
                return;
            }
        }
    }
    pika_platform_printf("no find _SEGGER_RTT addr\r\n"); 
}

void SystemView_start(PikaObj *self, PikaTuple* val)
{

    int val_num = pikaTuple_getSize(val);
    if(val_num == 2){
      Arg* arg = pikaTuple_getArg(val, 0);
      RTT_wAddr = (int)arg_getInt(arg);
      arg = pikaTuple_getArg(val, 1);
      RTT_wSize = (int)arg_getInt(arg);
      RTT_wChannel = 1;
    }
    if(val_num == 3){
      Arg* arg = pikaTuple_getArg(val, 0);
      RTT_wAddr = (int)arg_getInt(arg);
      arg = pikaTuple_getArg(val, 1);
      RTT_wSize = (int)arg_getInt(arg);
      arg = pikaTuple_getArg(val, 2);
      RTT_wChannel = (int)arg_getInt(arg);
    }
    pika_platform_printf("Addr = 0x%x,wSize = %d,Channel = %d\n",RTT_wAddr,RTT_wSize,RTT_wChannel);
    SystemView_init(RTT_wAddr,RTT_wSize);
    if(segger_rtt_addr != 0){
          swd_read_memory(segger_rtt_addr, (uint8_t *)&_SEGGER_RTT, sizeof(_SEGGER_RTT));
          pika_platform_printf("Find %s addr 0x%x\r\n",_SEGGER_RTT.acID,segger_rtt_addr);
          for(uint8_t i = 0;i<_SEGGER_RTT.MaxNumUpBuffers;i++){
              pika_platform_printf("UpBuffer Channel %d Size: %d Mode: %d\r\n",i,_SEGGER_RTT.aUp[i].SizeOfBuffer,_SEGGER_RTT.aUp[i].Flags);
          }
          for(uint8_t i = 0;i<_SEGGER_RTT.MaxNumDownBuffers;i++){
              pika_platform_printf("DownBuffer Channel %d Size: %d Mode: %d\r\n",i,_SEGGER_RTT.aDown[i].SizeOfBuffer,_SEGGER_RTT.aDown[i].Flags);
          }
    }
}

void SystemView_stop(PikaObj *self, PikaTuple* val)
{

    segger_rtt_addr = 0;

}

