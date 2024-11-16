
static const uint32_t STM32F10x_1024_prog_blob[] = 
{
    0XE00ABE00,0X062D780D,0X24084068,0XD3000040,0X1E644058,0X1C49D1FA,0X2A001E52,0X4770D1F2,
    0X4603B510,0X04C00CD8,0X444C4C80,0X20006020,0X60204C7F,0X6060487F,0X6060487F,0X6460487D,
    0X6460487D,0X69C04620,0X0F04F010,0XF245D108,0X4C7A5055,0X20066020,0XF6406060,0X60A070FF,
    0XBD102000,0X48724601,0XF0406900,0X4A700080,0X46106110,0XF0406D00,0X65100080,0X47702000,
    0X6900486B,0X0004F040,0X61084969,0X69004608,0X0040F040,0XE0036108,0X20AAF64A,0X60084967,
    0X68C04863,0X0F01F010,0X4861D1F6,0XF0206900,0X495F0004,0X46086108,0XF0406D00,0X65080004,
    0X6D004608,0X0040F040,0XE0036508,0X20AAF64A,0X6008495A,0X6CC04856,0X0F01F010,0X4854D1F6,
    0XF0206D00,0X49520004,0X20006508,0X46014770,0X4448484E,0XF5006800,0X42812000,0X484CD21C,
    0XF0406900,0X4A4A0002,0X46106110,0X69006141,0X0040F040,0XE0036110,0X20AAF64A,0X60104A47,
    0X68C04843,0X0F01F010,0X4841D1F6,0XF0206900,0X4A3F0002,0XE01B6110,0X6D00483D,0X0002F040,
    0X65104A3B,0X65414610,0XF0406D00,0X65100040,0XF64AE003,0X4A3920AA,0X48356010,0XF0106CC0,
    0XD1F60F01,0X6D004832,0X0002F020,0X65104A30,0X47702000,0X4603B510,0XF0201C48,0X482B0101,
    0X68004448,0X2000F500,0XD2264283,0X4828E022,0XF0406900,0X4C260001,0X88106120,0XBF008018,
    0X68C04823,0X0F01F010,0X4821D1FA,0XF0206900,0X4C1F0001,0X46206120,0XF01068C0,0XD0060F14,
    0X68C04620,0X0014F040,0X200160E0,0X1C9BBD10,0X1E891C92,0XD1DA2900,0XE022E025,0X6D004814,
    0X0001F040,0X65204C12,0X80188810,0X4810BF00,0XF0106CC0,0XD1FA0F01,0X6D00480D,0X0001F020,
    0X65204C0B,0X6CC04620,0X0F14F010,0X4620D006,0XF0406CC0,0X64E00014,0XE7D72001,0X1C921C9B,
    0X29001E89,0X2000D1DA,0X0000E7D0,0X00000004,0X40022000,0X45670123,0XCDEF89AB,0X40003000,
    0X00000000,0X00000000,
};

static const uint32_t STM32F10x_1024_flash_dev[] = 
{
    0X54530101,0X4632334D,0X20783031,0X642D4C58,0X69736E65,0X46207974,0X6873616C,0X00000000,
    0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,
    0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,
    0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,
    0X00010000,0X08000000,0X00100000,0X00000400,0X00000000,0X000000FF,0X00000064,0X000001F4,
    0X00000800,0X00000000,0XFFFFFFFF,0XFFFFFFFF,
};

static const program_target_t STM32F10x_1024_flash =
{
    (RAM_BASE + 0X0021),  // Init
    (RAM_BASE + 0X0065),  // UnInit
    (RAM_BASE + 0X0081),  // EraseChip
    (RAM_BASE + 0X00EF),  // EraseSector
    (RAM_BASE + 0X0175),  // ProgramPage
    0x0,                    // Verify,
    // BKPT : start of blob + 1
    // RSB  : address to access global/static data
    // RSP  : stack pointer
    {
        (RAM_BASE + 1),
        (RAM_BASE + 0x244),
        (RAM_BASE + 0X800),
    },
    (RAM_BASE + 0XA00),                      // mem buffer location
    RAM_BASE,                      // location to write prog_blob in target RAM
    sizeof(STM32F10x_1024_prog_blob),    // prog_blob size
    STM32F10x_1024_prog_blob,     // address of prog_blob
    0x00000400,                      // ram_to_flash_bytes_to_be_written
    0
};