
static const uint32_t STM32F10x_512_prog_blob[] = 
{
    0XE00ABE00,0X062D780D,0X24084068,0XD3000040,0X1E644058,0X1C49D1FA,0X2A001E52,0X4770D1F2,
    0X4603B510,0X4C442000,0X48446020,0X48446060,0X46206060,0XF01069C0,0XD1080F04,0X5055F245,
    0X60204C40,0X60602006,0X70FFF640,0X200060A0,0X4601BD10,0X69004838,0X0080F040,0X61104A36,
    0X47702000,0X69004834,0X0004F040,0X61084932,0X69004608,0X0040F040,0XE0036108,0X20AAF64A,
    0X60084930,0X68C0482C,0X0F01F010,0X482AD1F6,0XF0206900,0X49280004,0X20006108,0X46014770,
    0X69004825,0X0002F040,0X61104A23,0X61414610,0XF0406900,0X61100040,0XF64AE003,0X4A2120AA,
    0X481D6010,0XF01068C0,0XD1F60F01,0X6900481A,0X0002F020,0X61104A18,0X47702000,0X4603B510,
    0XF0201C48,0XE0220101,0X69004813,0X0001F040,0X61204C11,0X80188810,0X480FBF00,0XF01068C0,
    0XD1FA0F01,0X6900480C,0X0001F020,0X61204C0A,0X68C04620,0X0F14F010,0X4620D006,0XF04068C0,
    0X60E00014,0XBD102001,0X1C921C9B,0X29001E89,0X2000D1DA,0X0000E7F7,0X40022000,0X45670123,
    0XCDEF89AB,0X40003000,0X00000000,
};

static const uint32_t STM32F10x_512_flash_dev[] = 
{
    0X54530101,0X4632334D,0X20783031,0X68676948,0X6E65642D,0X79746973,0X616C4620,0X00006873,
    0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,
    0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,
    0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,0X00000000,
    0X00010000,0X08000000,0X00080000,0X00000400,0X00000000,0X000000FF,0X00000064,0X000001F4,
    0X00000800,0X00000000,0XFFFFFFFF,0XFFFFFFFF,
};

static uint32_t STM32F10x_512_rw_data[] = 
{
    0X00000000,
};

static program_target_t STM32F10x_512_flash =
{
    (RAM_BASE + 0X0021),  // Init
    (RAM_BASE + 0X0053),  // UnInit
    (RAM_BASE + 0X0065),  // EraseChip
    (RAM_BASE + 0X009F),  // EraseSector
    (RAM_BASE + 0X00DD),  // ProgramPage
    0x0,                    // Verify,
    // BKPT : start of blob + 1
    // RSB  : address to access global/static data
    // RSP  : stack pointer
    {
        (RAM_BASE + 1),
        (RAM_BASE + 0x148),
        (RAM_BASE + 0X800),
    },
    (RAM_BASE + 0XA00),                      // mem buffer location
    RAM_BASE,                      // location to write prog_blob in target RAM
    sizeof(STM32F10x_512_prog_blob),    // prog_blob size
    STM32F10x_512_prog_blob,     // address of prog_blob
    0x00000400,                      // ram_to_flash_bytes_to_be_written
};
