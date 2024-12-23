/**
 * @file    util.c
 * @brief   Implementation of util.h
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>

#include "util.h"


//remove dependency from vfs_manager
__attribute__((weak)) void vfs_mngr_fs_remount(void) {}

uint32_t util_write_hex8(char *str, uint8_t value)
{
    static const char nybble_chars[] = "0123456789abcdef";
    *(str + 0) = nybble_chars[(value >> 4) & 0x0F ];
    *(str + 1) = nybble_chars[(value >> 0) & 0x0F ];
    return 2;
}

uint32_t util_write_hex16(char *str, uint16_t value)
{
    uint32_t pos = 0;
    pos += util_write_hex8(str + pos, (value >> 8) & 0xFF);
    pos += util_write_hex8(str + pos, (value >> 0) & 0xFF);
    return pos;
}

uint32_t util_write_hex32(char *str, uint32_t value)
{
    uint32_t pos = 0;
    pos += util_write_hex8(str + pos, (value >> 0x18) & 0xFF);
    pos += util_write_hex8(str + pos, (value >> 0x10) & 0xFF);
    pos += util_write_hex8(str + pos, (value >> 0x08) & 0xFF);
    pos += util_write_hex8(str + pos, (value >> 0x00) & 0xFF);
    return pos;
}

uint32_t util_write_uint32(char *str, uint32_t value)
{
    uint32_t temp_val;
    uint64_t digits;
    uint32_t i;
    // Count the number of digits
    digits = 0;
    temp_val = value;

    while (temp_val > 0) {
        temp_val /= 10;
        digits += 1;
    }

    if (digits <= 0) {
        digits = 1;
    }

    // Write the number
    for (i = 0; i < digits; i++) {
        str[digits - i - 1] = '0' + (value % 10);
        value /= 10;
    }

    return digits;
}

uint32_t util_write_uint32_zp(char *str, uint32_t value, uint16_t total_size)
{
    uint32_t size;
    // Get the size of value
    size = util_write_uint32(str, value);

    if (size >= total_size) {
        return size;
    }

    // Zero fill
    memset(str, '0', total_size);
    // Write value
    util_write_uint32(str + (total_size - size), value);
    return total_size;
}

uint32_t util_write_string(char *str, const char *data)
{
    uint32_t pos = 0;

    while (0 != data[pos]) {
        str[pos] = data[pos];
        pos++;
    }

    return pos;
}

uint32_t util_write_string_in_region(uint8_t *buf, uint32_t size, uint32_t start, uint32_t pos, const char *input) {
    return util_write_in_region(buf, size, start, pos, input, strlen(input));
}

uint32_t util_write_in_region(uint8_t *buf, uint32_t size, uint32_t start, uint32_t pos, const char *input, uint32_t length) {
    if (buf != NULL) {
        // Check if there is something to copy
        if (((pos + length) >= start) && (pos <= (start + size))) {
            uint32_t i_off = 0;
            uint32_t o_off = 0;
            uint32_t l = length;
            if (pos < start) {
                i_off = start - pos;
                l -= i_off;
            } else {
                o_off = pos - start;
            }
            if ((pos + length) > (start + size)) {
                l -= (pos + length) - (start + size);
            }
            memcpy(buf + o_off, input + i_off, l);
        }
    }

    return length;
}

void _util_assert(bool expression, const char *filename, uint16_t line)
{

}

void util_assert_clear()
{

}
