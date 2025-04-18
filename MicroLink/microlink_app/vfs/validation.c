/**
 * @file    validation.c
 * @brief   Implementation of validation.h
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2019, ARM Limited, All Rights Reserved
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
#include <stdio.h>
#include <string.h>
#include "validation.h"

typedef struct rbl_part_head_t {
    char chType[4];
    uint16_t hwAlgo;
    uint8_t chTimeStamp[6];
    char chAppPartName[16];
    char chDownloadVersion[24];
    char NoUse[24];
    uint32_t wPkgCrc;
    uint32_t wInvalid;
    uint32_t wRawSize;
    uint32_t wPkgSize;
    uint32_t wHeadCrc;
}rbl_part_head_t;
static struct rbl_part_head_t s_tPartHeat;

static inline uint32_t test_range(const uint32_t test, const uint32_t min, const uint32_t max)
{
    return ((test < min) || (test > max)) ? 0 : 1;
}

uint8_t validate_bin_nvic(const uint8_t *buf)
{
    return validate_bin_nvic_base(buf);
}

extern uint32_t get_bin_start_address(void);

uint8_t validate_bin_nvic_base(const uint8_t *buf)
{
    uint32_t i = 4, nvic_val = 0;
    uint8_t in_range = 0;
    // test the initial SP value
    //memcpy(&nvic_val, buf + 0, sizeof(nvic_val));
    ////printf("nvic_val = 0x%x\n",nvic_val);
    //if (1 == test_range(nvic_val, 0x1FFF8000, 0x24080000)) {
    //    in_range = 1;
    //}

    //if (in_range == 0) {
    //    return 0;
    //}
    uint32_t nvic_val_base = (get_bin_start_address() & 0XFF000000);
    // Reset_Handler
    // NMI_Handler
    // HardFault_Handler
    for (; i <= 12; i += 4) {
        in_range = 0;
        memcpy(&nvic_val, buf + i, sizeof(nvic_val));
        if (1 == test_range(nvic_val, nvic_val_base, nvic_val_base + 0x200000)) {
            in_range = 1;
        }
        if (in_range == 0) {
            return 0;
        }
    }
    return 1;
}

uint8_t validate_hexfile(const uint8_t *buf)
{
      // look here for known hex records
      // add hex identifier b[0] == ':' && b[8] == {'0', '2', '3', '4', '5'}
      return ((buf[0] == ':') && ((buf[8] == '0') || (buf[8] == '2') || (buf[8] == '3') || (buf[8] == '4') || (buf[8] == '5'))) ? 1 : 0;
}

uint8_t validate_rblfile(const uint8_t *buf)
{
    memcpy((uint8_t *)&s_tPartHeat, buf + 0, sizeof(rbl_part_head_t));
    return (strcmp(s_tPartHeat.chType, "RBL") == 0 && strncmp(s_tPartHeat.chAppPartName, "MicroLink", 9) == 0 );
}


