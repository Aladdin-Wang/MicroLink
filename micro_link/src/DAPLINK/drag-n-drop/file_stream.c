#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "virtual_fs.h"
#include "file_stream.h"
#include "util.h"
#include "validation.h"
#include "flash_decoder.h"
#include "cdc_acm_msc_dap.h"
#include "intelhex.h"
#include "ymodem_send_file.h"
#include "target_family.h"
#include "ota_api.h"
#include "core_cm3.h"
#undef this
#define this        (*ptThis)

typedef enum {
    STREAM_STATE_CLOSED,
    STREAM_STATE_OPEN,
    STREAM_STATE_END,
    STREAM_STATE_ERROR
} stream_state_t;

typedef bool (*stream_detect_cb_t)(const uint8_t *data, uint32_t size);
typedef error_t (*stream_open_cb_t)(void *state);
typedef error_t (*stream_write_cb_t)(void *state, const uint8_t *data, uint32_t size);
typedef error_t (*stream_close_cb_t)(void *state);

typedef struct {
    stream_detect_cb_t detect;
    stream_open_cb_t open;
    stream_write_cb_t write;
    stream_close_cb_t close;
} stream_t;

typedef struct {
    uint8_t vector_buf[FLASH_DECODER_MIN_SIZE];
    uint8_t buf_pos;
    uint32_t flash_addr;
} bin_state_t;

typedef struct {
    int parsing_complete;
    uint8_t bin_buffer[256];
} hex_state_t;

typedef union {
    bin_state_t bin;
    hex_state_t hex;
} shared_state_t;

static bool detect_bin(const uint8_t *data, uint32_t size);
static error_t open_bin(void *state);
static error_t write_bin(void *state, const uint8_t *data, uint32_t size);
static error_t close_bin(void *state);

static bool detect_hex(const uint8_t *data, uint32_t size);
static error_t open_hex(void *state);
static error_t write_hex(void *state, const uint8_t *data, uint32_t size);
static error_t close_hex(void *state);

static bool detect_rbl(const uint8_t *data, uint32_t size);
static error_t open_rbl(void *state);
static error_t write_rbl(void *state, const uint8_t *data, uint32_t size);
static error_t close_rbl(void *state);

static const stream_t stream[] = {
    {detect_bin, open_bin, write_bin, close_bin},   // STREAM_TYPE_BIN
    {detect_hex, open_hex, write_hex, close_hex},   // STREAM_TYPE_HEX
    {detect_rbl, open_rbl, write_rbl, close_rbl},   // STREAM_TYPE_RBL
};

static shared_state_t shared_state;
static stream_state_t state = STREAM_STATE_CLOSED;
static stream_t *current_stream = 0;

stream_type_t stream_start_identify(const uint8_t *data, uint32_t size)
{
    stream_type_t i;

    for (i = STREAM_TYPE_START; i < STREAM_TYPE_COUNT; i++) {
        if (stream[i].detect(data, size)) {
            return i;
        }
    }

    return STREAM_TYPE_NONE;
}

// Identify the file type from its extension
stream_type_t stream_type_from_name(const vfs_filename_t filename)
{
    // 8.3 file names must be in upper case
    if (0 == strncmp("BIN", &filename[8], 3)) {
        return STREAM_TYPE_BIN;
    } else if (0 == strncmp("HEX", &filename[8], 3)) {
        return STREAM_TYPE_HEX;
    } else if (0 == strncmp("RBL", &filename[8], 3)) {
        return STREAM_TYPE_RBL;
    } else {
        return STREAM_TYPE_NONE;
    }
}
extern uint8_t swd_write_word(uint32_t addr, uint32_t val);
extern uint8_t swd_read_word(uint32_t addr, uint32_t *val);
extern uint8_t swd_read_memory(uint32_t address, uint8_t *data, uint32_t size);
extern uint8_t swd_write_memory(uint32_t address, uint8_t *data, uint32_t size);
extern uint8_t swd_init_debug(void);
extern void led_usb_in_activity(void);
extern void led_usb_out_activity(void);
/**
 * @brief Unlock FLASH control register
 *
 * This function unlocks the FLASH control register to allow writing option bytes.
 */
void unlock_flash_control_register(void) {
    // Write the unlock key to the FLASH key register (FLASH_KEYR)
   // swd_write_memory(0x40022004, 0x45670123,4); // FLASH_KEYR
   // swd_write_memory(0x40022004, 0xCDEF89AB,4);
}

/**
 * @brief Lock FLASH control register
 *
 * This function locks the FLASH control register to prevent further modifications.
 */
void lock_flash_control_register(void) {
    // Write the lock key to the FLASH control register (FLASH_CR)
    uint32_t flash_cr;
    swd_read_memory(0x40022010, &flash_cr,4); // FLASH_CR
    flash_cr |= 0x80; // Set the LOCK bit
    swd_write_memory(0x40022010, &flash_cr,4);
}

/**
 * @brief Erase option bytes
 *
 * This function erases the option bytes to allow writing new values.
 */
void erase_option_bytes(void) {
    uint32_t flash_cr;
    
    // Enable option bytes erase
    swd_read_memory(0x40022010, &flash_cr,4); // FLASH_CR
    flash_cr |= 0x20; // Set the OPTER bit
    swd_write_memory(0x40022010, &flash_cr,4);
    
    // Start erase operation
    flash_cr |= 0x40; // Set the STRT bit
    swd_write_memory(0x40022010, &flash_cr,4);
    
    // Wait for the operation to complete
    uint32_t flash_sr;
    do {
        swd_read_memory(0x4002200C, &flash_sr,4); // FLASH_SR
    } while (flash_sr & 0x01); // Check the BSY bit
}

/**
 * @brief Disable FLASH read protection on STM32
 *
 * This function reads the option bytes, clears the read protection (RDP) bit,
 * writes the modified option bytes back, and triggers a system reset to apply the changes.
 */
void disable_flash_read_protection(void) {
    uint32_t option_bytes = 0;
    swd_init_debug();
    unlock_flash_control_register();
    erase_option_bytes();
    // 读取当前选项字节
    if (!swd_read_memory(0x1FFFF800, (uint8_t *)&option_bytes,sizeof(option_bytes))) {
        printf("Failed to read option bytes\n");
        return;
    }

    // 修改选项字节，清除读保护位
    option_bytes &= ~0xFF;  // 清除RDP位

    // 写回选项字节
    if (!swd_write_memory(0x1FFFF800, (uint8_t *)&option_bytes,sizeof(option_bytes))) {
        printf("Failed to write option bytes\n");
        return;
    }
    printf("Disable FLASH read protection.\n");
    // 触发系统复位以应用更改
    swd_write_word((uint32_t)&SCB->AIRCR, ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk));   
    lock_flash_control_register(); 
}
/**
 * @brief Enable FLASH read protection on STM32
 *
 * This function reads the option bytes, sets the read protection (RDP) bit,
 * writes the modified option bytes back, and triggers a system reset to apply the changes.
 */
void enable_flash_read_protection(void) {
    uint32_t option_bytes = 0;
    swd_init_debug();
    unlock_flash_control_register();
    erase_option_bytes();
    // 读取当前选项字节
    if (!swd_read_memory(0x1FFFF800, (uint8_t *)&option_bytes,sizeof(option_bytes))) {
        printf("Failed to read option bytes\n");
        return;
    }

    // 修改选项字节，设置读保护位
    option_bytes |= 0xCC;  // 设置RDP位

    // 写回选项字节
    if (!swd_write_memory(0x1FFFF800, (uint8_t *)&option_bytes,sizeof(option_bytes))) {
        printf("Failed to write option bytes\n");
        return;
    }
    printf("Enable FLASH read protection.\n");
    // 触发系统复位以应用更改
    swd_write_word((uint32_t)&SCB->AIRCR, ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk)); 
    lock_flash_control_register();
}

error_t stream_open(stream_type_t stream_type)
{
    error_t status;
    uint32_t val = 0;
    // Stream must not be open already
    if (state != STREAM_STATE_CLOSED) {
        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Stream must be of a supported type
    if (stream_type >= STREAM_TYPE_COUNT) {
        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Initialize all variables
    memset(&shared_state, 0, sizeof(shared_state));
    state = STREAM_STATE_OPEN;
    current_stream = (stream_t *)&stream[stream_type];
    // Initialize the specified stream
    status = current_stream->open(&shared_state);

    if (ERROR_SUCCESS != status) {
        state = STREAM_STATE_ERROR;
    }

    return status;
}

error_t stream_write(const uint8_t *data, uint32_t size)
{
    error_t status;

    // Stream must be open already
    if (state != STREAM_STATE_OPEN) {
        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Write to stream
    status = current_stream->write(&shared_state, data, size);
    led_usb_in_activity();
    led_usb_out_activity();
    if (ERROR_SUCCESS_DONE == status) {
        state = STREAM_STATE_END;
    } else if ((ERROR_SUCCESS_DONE_OR_CONTINUE == status) || (ERROR_SUCCESS == status)) {
        // Stream should remain in the open state
        util_assert(STREAM_STATE_OPEN == state);
    } else {
        state = STREAM_STATE_ERROR;
    }

    return status;
}

error_t stream_close(void)
{
    error_t status;

    // Stream must not be closed already
    if (STREAM_STATE_CLOSED == state) {
        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Close stream
    status = current_stream->close(&shared_state);

    state = STREAM_STATE_CLOSED;
    return status;
}

bool vfs_user_file_change_handler_hook(const vfs_filename_t filename, vfs_file_change_t change,
        vfs_file_t file, vfs_file_t new_file_data)
{
    strcpy(tYmodemSend.chFileName,filename);
    tYmodemSend.wFileSize = vfs_file_get_size(new_file_data);
    return false;
}

/* Binary file processing */
static bool detect_bin(const uint8_t *data, uint32_t size)
{
    return validate_bin_nvic(data);
}

static error_t open_bin(void *state)
{
    error_t status = 0;
    if(0 != get_chip_id()){
        status = flash_decoder_open();
    }else{
        ymodem_send_init(&tYmodemSend);
    }
    return status;
}

static error_t write_bin(void *state, const uint8_t *data, uint32_t size)
{
    error_t status;
    if(false == bYmomdemIsinit){
      bin_state_t *bin_state = (bin_state_t *)state;
      if (bin_state->buf_pos < FLASH_DECODER_MIN_SIZE) {
          flash_decoder_type_t flash_type;
          uint32_t size_left;
          uint32_t copy_size;
          uint32_t start_addr;
          const flash_intf_t *flash_intf;
          // Buffer Data
          size_left = FLASH_DECODER_MIN_SIZE - bin_state->buf_pos;
          copy_size = MIN(size_left, size);
          memcpy(bin_state->vector_buf + bin_state->buf_pos, data, copy_size);
          bin_state->buf_pos += copy_size;

          if (bin_state->buf_pos < FLASH_DECODER_MIN_SIZE) {
              // Not enough data to determine type
              return ERROR_SUCCESS;
          }

          data += copy_size;
          size -= copy_size;
          // Determine type
          flash_type = flash_decoder_detect_type(bin_state->vector_buf, bin_state->buf_pos, 0, false);

          if (FLASH_DECODER_TYPE_UNKNOWN == flash_type) {
              return ERROR_FD_UNSUPPORTED_UPDATE;
          }

          // Determine flash addresss
          status = flash_decoder_get_flash(flash_type, 0, false, &start_addr, &flash_intf);

          if (ERROR_SUCCESS != status) {
              return status;
          }

          bin_state->flash_addr = start_addr;
          // Pass on data to the decoder
          status = flash_decoder_write(bin_state->flash_addr, bin_state->vector_buf, bin_state->buf_pos);

          if (ERROR_SUCCESS != status) {
              return status;
          }

          bin_state->flash_addr += bin_state->buf_pos;
      }

      // Write data
      status = flash_decoder_write(bin_state->flash_addr, data, size);

      if (ERROR_SUCCESS != status) {
          return status;
      }

      bin_state->flash_addr += size;
    }else{
        ymomdem_usb_in_queue(data,size);
    }
    return ERROR_SUCCESS_DONE_OR_CONTINUE;
}

static error_t close_bin(void *state)
{
    error_t status = 0;
    if(false == bYmomdemIsinit){
       status = flash_decoder_close();
       swd_init_debug();
       swd_write_word((uint32_t)&SCB->AIRCR, ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk)); 
    }else{
       ymodem_send_uninit(&tYmodemSend);
    } 
    return status;
}

/* Hex file processing */

static bool detect_hex(const uint8_t *data, uint32_t size)
{
    return validate_hexfile(data);
}

static error_t open_hex(void *state)
{
    error_t status;
    get_chip_id();
    hex_state_t *hex_state = (hex_state_t *)state;
    memset(hex_state, 0, sizeof(*hex_state));
    reset_hex_parser();
    hex_state->parsing_complete = false;
    status = flash_decoder_open();

    return status;
}

static error_t write_hex(void *state, const uint8_t *data, uint32_t size)
{
    error_t status = ERROR_SUCCESS;
    hex_state_t *hex_state = (hex_state_t *)state;
    hexfile_parse_status_t parse_status = HEX_PARSE_UNINIT;
    uint32_t bin_start_address = 0; // Decoded from the hex file, the binary buffer data starts at this address
    uint32_t bin_buf_written = 0;   // The amount of data in the binary buffer starting at address above
    uint32_t block_amt_parsed = 0;  // amount of data parsed in the block on the last call

    while (1) {
        // try to decode a block of hex data into bin data
        parse_status = parse_hex_blob(data, size, &block_amt_parsed, hex_state->bin_buffer, sizeof(hex_state->bin_buffer), &bin_start_address, &bin_buf_written);

        // the entire block of hex was decoded. This is a simple state
        if (HEX_PARSE_OK == parse_status) {
            if (bin_buf_written > 0) {
                status = flash_decoder_write(bin_start_address, hex_state->bin_buffer, bin_buf_written);
            }

            break;
        } else if (HEX_PARSE_UNALIGNED == parse_status) {
            if (bin_buf_written > 0) {
                status = flash_decoder_write(bin_start_address, hex_state->bin_buffer, bin_buf_written);

                if (ERROR_SUCCESS != status) {
                    break;
                }
            }

            // incrememntal offset to finish the block
            size -= block_amt_parsed;
            data += block_amt_parsed;
        } else if (HEX_PARSE_EOF == parse_status) {
            if (bin_buf_written > 0) {
                status = flash_decoder_write(bin_start_address, hex_state->bin_buffer, bin_buf_written);
            }

            if (ERROR_SUCCESS == status) {
                status = ERROR_SUCCESS_DONE;
            }

            break;
        } else if (HEX_PARSE_CKSUM_FAIL == parse_status) {
            status = ERROR_HEX_CKSUM;
            break;
        } else if ((HEX_PARSE_UNINIT == parse_status) || (HEX_PARSE_FAILURE == parse_status)) {
            util_assert(HEX_PARSE_UNINIT != parse_status);
            status = ERROR_HEX_PARSER;
            break;
        }
    }

    return status;
}

static error_t close_hex(void *state)
{
    error_t status;
    status = flash_decoder_close();
    swd_init_debug();
    swd_write_word((uint32_t)&SCB->AIRCR, ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk));   

    return status;
}

/* RBL file processing */
static bool detect_rbl(const uint8_t *data, uint32_t size)
{
    return validate_rblfile(data);
}

static error_t open_rbl(void *state)
{
    error_t status = 0;
    
    return status;
}

static error_t write_rbl(void *state, const uint8_t *data, uint32_t size)
{
    static uint32_t offset = 0;
    ota_board_flash_write(DOWNLOAD_PART_ADDR + offset,data,size);
    offset = offset + size;
    return ERROR_SUCCESS_DONE_OR_CONTINUE;
}
extern uint32_t uid_word_encrypt[];
static error_t close_rbl(void *state)
{
    error_t status = 0;
    uint32_t wData = 0;
    board_flash_erase((APP_PART_ADDR + APP_PART_SIZE - MARK_SIZE));
    board_flash_write((APP_PART_ADDR + APP_PART_SIZE - MARK_SIZE), (const uint8_t *)&wData, sizeof(wData));
    return status;
}








