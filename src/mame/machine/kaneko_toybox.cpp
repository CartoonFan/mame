// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia, Sebastien Volpe
/* Kaneko 'Toybox' protection

 the following chips have been seen

TBSOP01 is a NEC uPD78324 series MCU with 32K internal rom & 1024 bytes of ram
TBSOP02 is likely the same NEC uPD78324 series MCU as the TBS0P01

I'm guessing the only actual difference between them is the decryption table used
although the Jackie Chan info below contradicts that..

Currently none of the MCUs' internal roms are dumped so simulation is used


94  Bonk's Adventure             TOYBOX?            TBSOP01
94  Blood Warrior                TOYBOX?            TBS0P01 452 9339PK001
94  Great 1000 Miles Rally       TOYBOX                                                  "MM0525-TOYBOX199","USMM0713-TB1994 "
94  Great 1000 Miles Rally EV/US TOYBOX
95  Great 1000 Miles Rally 2     TOYBOX      KANEKO TBSOP02 454 9451MK002 (74 pin PQFP)  "USMM0713-TB1994 "
95  Jackie Chan                  TOYBOX                                                  "USMM0713-TB1994 "
95  Gals Panic 3                 TOYBOX?            TBSOP01

 todo:

 bonk:
    Where does the hardcoded EEPROM default data come from (there is a command to restore defaults directly, not from RAM)
    Where does the data for the additional tables come from, a transfer mode none of the other games use is used. (related to src[offs+6] and src[offs+7] params? )


MCU parameters:
---------------

mcu_command = kaneko16_mcu_ram[0x0010/2];    // command nb
mcu_offset  = kaneko16_mcu_ram[0x0012/2]/2;  // offset in shared RAM where MCU will write
mcu_subcmd  = kaneko16_mcu_ram[0x0014/2];    // sub-command parameter, happens only for command #4


    the only MCU commands found in program code are:
    - 0x04: protection: provide data (see below) and code <<<---!!!
    - 0x03: read DSW
    - 0x02: load game settings \ stored in ATMEL AT93C46 chip,
    - 0x42: save game settings / 128 bytes serial EEPROM
    - 0x43: restore eeprom defaults (from internal ROM or data ROM?)

*/



#include "emu.h"
#include "kaneko_toybox.h"

/***************************************************************************

    TOYBOX MCU data for Bonk's Adventure

***************************************************************************/

// notes based on test programs results, verified on PCB:
// - bonkadv_mcu_4_30 is the only cmd that outputs an odd (655) number of bytes
// - bonkadv_mcu_4_33: the 32 'zeroed' bytes every 64 bytes are effectively written by the MCU

// MCU executed command: 4300 0100 - factory settings
// this command is issued whenever the nvram settings are corrupted
// the MCU writes directly to NVRAM
static const uint16_t bonkadv_mcu_43[] = {
	0x8BE0,0x8E71,0x0102,0x0102,0x0300,0x010C,0x0202,0x0202,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0030,0x0020,0x4F6B,0x0305,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0082,0x6F82,0x628C,0xB490,0x6CAB
};

// MCU executed command: 0400 0E00 0034 - code
/*
200E00: 7071    moveq   #$71, D0
200E02: 7273    moveq   #$73, D1
200E04: 7475    moveq   #$75, D2
200E06: 7677    moveq   #$77, D3
200E08: 7879    moveq   #$79, D4
200E0A: 7A7B    moveq   #$7b, D5
200E0C: 7C7D    moveq   #$7d, D6
200E0E: 7E7F    moveq   #$7f, D7
200E10: 4E75    rts
*/
static const uint16_t bonkadv_mcu_4_34[] = {
	0x7071,0x7273,0x7475,0x7677,0x7879,0x7a7b,0x7c7d,0x7e7f,
	0x4e75
};
// MCU executed command: 0400 0180 0032 - 128 bytes at $200180
static const uint16_t bonkadv_mcu_4_32[] = {
	0x00cc,0xcc0c,0xc0c0,0xc080,0x0484,0xb6a6,0x0404,0x80c0,
	0x80b1,0xb1a1,0xa1b2,0xa2b3,0xb3a3,0xa3b1,0xb1b1,0xb1c0,
	0xc0a1,0xa1a1,0xa1b2,0xb2a2,0xa290,0x9090,0xb9b9,0xa9a9,
	0xbaaa,0xbbbb,0xabab,0xb9b9,0xb9b9,0xc8c8,0xa9a9,0xa9a9,
	0xbaba,0xaaaa,0x9898,0x98b9,0xb9a9,0xa9ba,0xaabb,0xbbab,
	0xabb9,0xb9b9,0xb9c8,0xc8a9,0xa9a9,0xa9ba,0xbaaa,0xaa98,
	0x9898,0xb1b1,0xa1a1,0xb2a2,0xb3b3,0xa3a3,0xb1b1,0xb1b1,
	0xc0c0,0xa1a1,0xa1a1,0xb2b2,0xa2a2,0x9090,0x9000,0x0000
};
// MCU executed command: 0400 0280 0031 - 112 bytes at $200280
static const uint16_t bonkadv_mcu_4_31[] = {
	0x1013,0x1411,0x1216,0x1519,0x1a17,0x1824,0x2322,0x211f,
	0x201e,0x1d1c,0x1b27,0x2825,0x2629,0x2a2b,0x2e2f,0x2c2d,
	0x3130,0x3435,0x3233,0x3f3e,0x3d3c,0x3a3b,0x3938,0x3736,
	0x4243,0x4041,0x4445,0x4649,0x4a47,0x484c,0x4b4f,0x504d,
	0x4e5a,0x5958,0x5755,0x5654,0x5352,0x515d,0x5e5b,0x5c5f,
	0x6061,0x6465,0x6263,0x6766,0x6a6b,0x6869,0x7574,0x7372,
	0x7071,0x6f6e,0x6d6c,0x7879,0x7677,0x7a7b,0x7c7d,0x7e7f
};
// MCU executed command: 0400 0E50 0030 - 655 bytes at $200E50 (656 here)
static const uint16_t bonkadv_mcu_4_30[] = {
	0x8c00,0x9700,0xa000,0xa700,0xb200,0xb700,0xc000,0xcc00,
	0xd600,0xdf00,0xef00,0xf500,0xfd00,0x0501,0x0b01,0x1801,
	0x1e01,0x2901,0x3101,0x3701,0x4101,0x4901,0x0b01,0x5d01,
	0xa700,0xb200,0x6c01,0x8101,0xb200,0x8101,0x8101,0x8101,
	0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,
	0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,
	0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x8101,0x2e02,
	0x3602,0x3d02,0x4102,0x4f02,0x5502,0x5502,0x5f02,0x6402,
	0x6d02,0x7302,0x7802,0x7d02,0x8302,0x8a02,0x0165,0x712b,
	0x57fe,0x57a2,0xff00,0x0003,0x7c3a,0x436c,0x66ff,0x0000,
	0x026c,0x6d43,0xff00,0x0000,0x74a5,0x8373,0x3599,0x9fff,

	0x0000,0x074a,0xff00,0x0002,0x8788,0x3331,0x32ff,0x0000,
	0x0276,0x8c71,0x2d8c,0xfe12,0x8cff,0x0000,0x0171,0x0d29,
	0xfe0d,0x29ff,0x0000,0x0559,0x5a5b,0x8a8b,0xff00,0x0003,
	0x0d0e,0x576a,0x641e,0xfe0d,0x0efe,0x64a3,0xff00,0x0006,
	0x0262,0xff00,0x0007,0x3e3f,0x7a7b,0xff00,0x0007,0x7584,
	0x4c53,0xff00,0x0001,0x6645,0xff00,0x0007,0x0d18,0x2984,
	0x5384,0xfe0d,0x29ff,0x0000,0x0457,0x5cff,0x0000,0x0751,
	0x5202,0x45fe,0x4575,0xff00,0x0000,0x191a,0x1b6e,0xff00,
	0x0006,0x3472,0xff00,0x0000,0x6569,0x090f,0x8b23,0xff00,
	0x0004,0x3b3c,0x3d79,0xff00,0x0080,0x770a,0x8176,0x2a65,
	0x64fe,0x7776,0xfe2a,0x76fe,0x650a,0xff00,0x0000,0x1011,
	0x1226,0x2728,0xa655,0x940f,0x68ff,0x0000,0x8045,0x6477,
	0x0a81,0x762a,0x65fe,0x7776,0xfe2a,0x76fe,0x650a,0xff00,
	0x0000,0x166b,0xff00,0x0007,0x1e30,0x39ff,0x0000,0x0345,
	0x3a77,0x7843,0x4647,0xff00,0x0000,0x7335,0x8399,0x9fff,
	0x0000,0x0237,0x38ff,0x0000,0x0044,0x303b,0xff00,0x0000,

	0x7d7e,0x7fa1,0x0aff,0x0000,0x0070,0xff00,0x0000,0x3e3f,
	0x7a7b,0x6180,0x89a4,0xff00,0x0006,0x86a5,0x4f42,0xff00,
	0x0000,0x1011,0x1255,0x0f68,0xff00,0x0002,0x8788,0x4033,
	0xff00,0x0005,0x650f,0xff00,0x0008,0x9596,0x9ba7,0xff00,
	0x0000,0x4849,0x82ff,0x0000,0x022e,0x2f30,0xff00,0x0001,
	0x7778,0x5826,0x2728,0xa6ff,0x0000,0x0813,0x1402,0x51ff,
	0x0000,0x072e,0x5c5d,0xff00,0x0006,0x57ff,0x0000,0x0230,
	0x391e,0x65fe,0x6530,0xff00,0x0000,0x4bff,0x0000,0x008e,
	0x8f90,0x91ff,0x0000,0x0092,0x9d17,0xff00,0x0000,0xff00,
	0x0000,0x8d9b,0xa77d,0x7e7f,0xa10a,0x9596,0xff00,0x0001,
	0x1993,0xff00,0x0000,0x67ff,0x0000,0x0067,0xff00,0x0000,
	0x27ff,0x0000,0x0390,0x8e23,0x978b,0xff00,0x0000,0x1d9e,
	0xff00,0x0007,0x3bff,0x0000,0x001d,0xff00,0x0000,0x8e90,
	0xff00,0x0005,0x2495,0x96ff,0x0000,0x009a,0xff00,0x0000
};
// MCU executed command: 0400 0400 0033 - 2560 bytes at $200400
static const uint16_t bonkadv_mcu_4_33[] = {
	0x00a4,0x0001,0x00a5,0x005a,0x00a6,0x0074,0x00a7,0x009b,
	0x00a8,0x00d0,0x00a9,0x00fe,0x00aa,0x015d,0x00ab,0x01b0,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0001,0x0002,0x0019,0x005b,0x0022,0x0075,0x002d,0x009c,
	0x0041,0x00d1,0x004e,0x00ff,0x0060,0x015f,0x0072,0x01b1,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0002,0x0008,0x001a,0x0061,0x0023,0x007b,0x002e,0x00a2,
	0x0042,0x00d7,0x004f,0x0107,0x0061,0x0165,0x0073,0x01b7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x000a,0x0000,0x0063,0x0000,0x007d,0x0000,0x00a4,
	0x0000,0x00d9,0x0000,0x0109,0x0000,0x0167,0x0000,0x01b9,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0003,0x000b,0x001b,0x0064,0x0024,0x007e,0x002f,0x00a5,
	0x0043,0x00da,0x0050,0x010a,0x0062,0x0168,0x0074,0x01ba,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x000c,0x0000,0x0065,0x0000,0x007f,0x0000,0x00a6,
	0x0000,0x00db,0x0000,0x010b,0x0000,0x0169,0x0000,0x01bb,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0004,0x0008,0x001c,0x0061,0x0025,0x007b,0x0030,0x00a2,
	0x0044,0x00d7,0x0051,0x0107,0x0063,0x0165,0x0075,0x01b7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0005,0x000d,0x001d,0x0066,0x0025,0x007b,0x0031,0x00a7,
	0x0045,0x00dc,0x0051,0x0107,0x0063,0x0165,0x0075,0x01b7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0017,0x0000,0x0069,0x0000,0x0089,0x0000,0x00af,
	0x0000,0x00df,0x0000,0x0115,0x0000,0x0173,0x0000,0x01c5,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0009,0x0015,0x001e,0x0067,0x0029,0x0087,0x0034,0x00ad,
	0x0046,0x00dd,0x0055,0x0113,0x0067,0x0171,0x0079,0x01c3,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0006,0x000e,0x0006,0x000e,0x0026,0x0080,0x0006,0x000e,
	0x0000,0x0000,0x0052,0x010c,0x0064,0x016a,0x0076,0x01bc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0007,0x0010,0x0007,0x0010,0x0027,0x0082,0x0032,0x00a8,
	0x0000,0x0000,0x0053,0x010e,0x0065,0x016c,0x0077,0x01be,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x0013,0x0000,0x0013,0x0000,0x0085,0x0000,0x00ab,
	0x0000,0x0000,0x0000,0x0111,0x0000,0x016f,0x0000,0x01c1,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0008,0x0014,0x0008,0x0014,0x0028,0x0086,0x0033,0x00ac,
	0x0000,0x0000,0x0054,0x0112,0x0066,0x0170,0x0078,0x01c2,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0018,0x0000,0x006a,0x0000,0x008a,0x0000,0x00b0,
	0x0000,0x00e0,0x0000,0x0116,0x0000,0x0174,0x0000,0x01c6,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x000c,0x0000,0x0065,0x0000,0x007f,0x0000,0x00a6,
	0x0000,0x00db,0x0000,0x010b,0x0000,0x0169,0x0000,0x01bb,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x000a,0x0019,0x001f,0x006b,0x002a,0x008b,0x0035,0x00b1,
	0x0047,0x00e1,0x0056,0x0117,0x0068,0x0175,0x007a,0x01c7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x000b,0x0019,0x0020,0x006b,0x002b,0x008b,0x0036,0x00b1,
	0x0048,0x00e1,0x0057,0x0117,0x0069,0x0175,0x007a,0x01c7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0015,0x003e,0x0015,0x003e,0x0099,0x0095,0x0015,0x003e,
	0x0000,0x0000,0x009e,0x013a,0x006d,0x018f,0x007e,0x01dc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x000c,0x001d,0x000c,0x001d,0x000c,0x001d,0x000c,0x001d,
	0x0049,0x00e5,0x0058,0x011b,0x006a,0x0179,0x007b,0x01c9,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0000,0x001f,0x0000,0x001f,0x0000,0x001f,0x0000,0x001f,
	0x0000,0x00e7,0x0000,0x011d,0x0000,0x017b,0x0000,0x01cb,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x000d,0x0020,0x0021,0x006f,0x002c,0x008f,0x0037,0x00b5,
	0x004a,0x00e8,0x0059,0x011e,0x006b,0x017c,0x007c,0x01cc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0022,0x0000,0x0071,0x0000,0x0091,0x0000,0x00b7,
	0x0000,0x00ea,0x0000,0x0120,0x0000,0x017e,0x0000,0x01ce,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0014,0x003b,0x0014,0x003b,0x005d,0x0092,0x0014,0x003b,
	0x0014,0x003b,0x009d,0x0137,0x00a0,0x018c,0x00a1,0x01d9,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0012,0x0036,0x0012,0x0036,0x0012,0x0036,0x0012,0x0036,
	0x008a,0x00e5,0x00be,0x013c,0x006c,0x0184,0x007d,0x01d1,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0013,0x0038,0x0013,0x0038,0x0013,0x0038,0x0013,0x0038,
	0x008a,0x00e5,0x00bf,0x013d,0x006c,0x0184,0x007d,0x01d1,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0038,0x00ba,0x0038,0x00ba,0x0038,0x00ba,0x0038,0x00ba,
	0x0038,0x00ba,0x00a2,0x0128,0x006e,0x019c,0x007f,0x01e7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x005c,0x014d,0x005c,0x014d,0x005c,0x014d,0x005c,0x014d,
	0x0000,0x0000,0x009c,0x0124,0x0071,0x01a5,0x0082,0x01f0,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x005b,0x014d,0x005b,0x014d,0x005b,0x014d,0x005b,0x014d,
	0x0000,0x0000,0x009b,0x0124,0x0070,0x01a1,0x0081,0x01ec,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0044,0x0000,0x0044,0x0000,0x0044,0x0000,0x0044,
	0x0000,0x00db,0x0000,0x010b,0x0000,0x0169,0x0000,0x01bb,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0018,0x0045,0x0018,0x0045,0x0018,0x0045,0x0018,0x0045,
	0x0044,0x00d7,0x0051,0x0107,0x0063,0x0165,0x0075,0x01b7,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x008b,0x000e,0x008b,0x000e,0x008c,0x0080,0x008b,0x000e,
	0x0000,0x0000,0x008d,0x010c,0x008e,0x016a,0x008f,0x01bc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x0090,0x0010,0x0090,0x0010,0x0091,0x0082,0x0092,0x00a8,
	0x0000,0x0000,0x0093,0x010e,0x0094,0x016c,0x0095,0x01be,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0096,0x003e,0x0096,0x003e,0x009a,0x0095,0x0096,0x003e,
	0x0000,0x0000,0x009f,0x013a,0x0097,0x018f,0x0098,0x01dc,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0049,0x0000,0x0069,0x0000,0x0098,0x0000,0x00af,
	0x0000,0x00df,0x0000,0x0145,0x0000,0x0173,0x0000,0x01c5,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x005e,0x0048,0x005f,0x0068,0x006f,0x0097,0x0080,0x00ae,
	0x00ac,0x00de,0x00ad,0x0144,0x00ae,0x0172,0x00af,0x01c4,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	0x004b,0x00ed,0x004b,0x00ed,0x004b,0x00ed,0x004b,0x00ed,
	0x004b,0x00ed,0x00b0,0x012b,0x00b6,0x0186,0x00ba,0x01d3,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0087,0x00f2,0x0087,0x00f2,0x0087,0x00f2,0x0087,0x00f2,
	0x0087,0x00f2,0x00b3,0x0130,0x00b8,0x018b,0x00bc,0x01d8,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x00c1,0x0050,0x00c1,0x0050,0x00c1,0x0050,0x00c1,0x0050,
	0x00c1,0x0050,0x00c3,0x0148,0x00c5,0x019a,0x00c7,0x01e5,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x00c2,0x0050,0x00c2,0x0050,0x00c2,0x0050,0x00c2,0x0050,
	0x00c2,0x0050,0x00c4,0x0148,0x00c6,0x019a,0x00c8,0x01e5,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

/* decryption table */
static const uint8_t decryption_table[0x100] = {
	0x7b,0x82,0xf0,0xbc,0x7f,0x1d,0xa2,0xc5,0x2a,0xfa,0x55,0xee,0x1a,0xd0,0x59,0x76,
	0x5e,0x75,0x79,0x16,0xa5,0xf6,0x84,0xed,0x0f,0x2e,0xf2,0x36,0x61,0xac,0xcd,0xab,
	0x01,0x3b,0x01,0x87,0x73,0xab,0xce,0x5d,0xd4,0x1d,0x68,0x2a,0x35,0xea,0x13,0x27,
	0x00,0xaa,0x46,0x36,0x6e,0x65,0x80,0x7e,0x19,0xe2,0x96,0xab,0xac,0xa5,0x6c,0x63,
	0x4a,0x6f,0x87,0xf6,0x6a,0xac,0x38,0xe2,0x1f,0x87,0xf9,0xaa,0xf5,0x41,0x60,0xa6,
	0x42,0xb9,0x30,0xf2,0xc3,0x1c,0x4e,0x4b,0x08,0x10,0x42,0x32,0xbf,0xb2,0xc5,0x0f,
	0x7a,0xab,0x97,0xf6,0xe7,0xb3,0x46,0xf8,0xec,0x2b,0x7d,0x5f,0xb1,0x10,0x03,0xe4,
	0x0f,0x22,0xdf,0x8d,0x10,0x66,0xa7,0x7e,0x96,0xbd,0x5a,0xaf,0xaa,0x43,0xdf,0x10,
	0x7c,0x04,0xe2,0x9d,0x66,0xd7,0xf0,0x02,0x58,0x8a,0x55,0x17,0x16,0xe2,0xe2,0x52,
	0xaf,0xd9,0xf9,0x0d,0x59,0x70,0x86,0x3c,0x05,0xd1,0x52,0xa7,0xf0,0xbf,0x17,0xd0,
	0x23,0x15,0xfe,0x23,0xf2,0x80,0x60,0x6f,0x95,0x89,0x67,0x65,0xc9,0x0e,0xfc,0x16,
	0xd6,0x8a,0x9f,0x25,0x2c,0x0f,0x2d,0xe4,0x51,0xb2,0xa8,0x18,0x3a,0x5d,0x66,0xa0,
	0x9f,0xb0,0x58,0xea,0x78,0x72,0x08,0x6a,0x90,0xb6,0xa4,0xf5,0x08,0x19,0x60,0x4e,
	0x92,0xbd,0xf1,0x05,0x67,0x4f,0x24,0x99,0x69,0x1d,0x0c,0x6d,0xe7,0x74,0x88,0x22,
	0x2d,0x15,0x7a,0xa2,0x37,0xa9,0xa0,0xb0,0x2c,0xfb,0x27,0xe5,0x4f,0xb6,0xcd,0x75,
	0xdc,0x39,0xce,0x6f,0x1f,0xfe,0xcc,0xb5,0xe6,0xda,0xd8,0xee,0x85,0xee,0x2f,0x04,
};

/* alt decryption table (gtmr2) */
static const uint8_t decryption_table_alt[0x100] = {
	0x26,0x17,0xb9,0xcf,0x1a,0xf5,0x14,0x1e,0x0c,0x35,0xb3,0x66,0xa0,0x17,0xe9,0xe4,
	0x90,0xf6,0xd5,0x35,0xac,0x95,0x49,0x43,0x64,0x0c,0x03,0x75,0x4d,0xda,0xb6,0xdf,
	0x06,0xcf,0x83,0x9e,0x35,0x2c,0x71,0x2a,0xab,0xcc,0x65,0xd4,0x1f,0xb0,0x88,0x3c,
	0xb7,0x87,0x35,0xc0,0x41,0x65,0x9f,0xa0,0xd5,0x8c,0x3e,0x06,0x53,0xdb,0x45,0x64,
	0x09,0x1e,0xc5,0x8d,0x50,0x24,0xe2,0x4a,0x9b,0x99,0x77,0x25,0x43,0xa9,0x1d,0xac,
	0x99,0x31,0x75,0xb5,0x53,0xab,0xad,0x5a,0x42,0x14,0xa1,0x52,0xac,0xec,0x5f,0xf8,
	0x8c,0x78,0x05,0x47,0xea,0xb8,0xde,0x69,0x98,0x2d,0x8f,0x9d,0xfc,0x05,0xea,0xee,
	0x77,0xbb,0xa9,0x31,0x01,0x00,0xea,0xd8,0x9c,0x43,0xb5,0x2f,0x4e,0xb5,0x1b,0xd2,
	0x01,0x4b,0xc4,0xf8,0x76,0x92,0x59,0x4f,0x20,0x52,0xd9,0x7f,0xa9,0x19,0xe9,0x7c,
	0x8d,0x3b,0xec,0xe0,0x60,0x08,0x2e,0xbd,0x27,0x8b,0xb2,0xfc,0x29,0xd8,0x39,0x8a,
	0x4f,0x2f,0x6b,0x04,0x10,0xbd,0xa1,0x04,0xde,0xc0,0xd5,0x0f,0x04,0x86,0xd6,0xd8,
	0xfd,0xb1,0x3c,0x4c,0xd1,0xc4,0xf1,0x5b,0xf5,0x8b,0xe3,0xc4,0x89,0x3c,0x39,0x86,
	0xd2,0x92,0xc9,0xe5,0x2c,0x4f,0xe2,0x2f,0x2d,0xc5,0x35,0x09,0x94,0x47,0x3c,0x04,
	0x40,0x8b,0x57,0x08,0xf6,0x74,0xe9,0xb8,0x36,0x4d,0xc5,0x26,0x13,0x3d,0x75,0xa0,
	0xa8,0x29,0x09,0x8c,0x87,0xf7,0x13,0xaf,0x4c,0x38,0x0b,0x8a,0x7f,0x2c,0x62,0x27,
	0x47,0xaa,0xda,0x07,0x92,0x8d,0xfd,0x1f,0xee,0x48,0x1a,0x53,0x3b,0x98,0x6a,0x72,
};


DEFINE_DEVICE_TYPE(KANEKO_TOYBOX, kaneko_toybox_device, "kaneko_toybox", "Kaneko Toybox MCU")

kaneko_toybox_device::kaneko_toybox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KANEKO_TOYBOX, tag, owner, clock)
	, m_eeprom(*this, finder_base::DUMMY_TAG)
	, m_dsw1(*this, finder_base::DUMMY_TAG)
	, m_mcuram(*this, finder_base::DUMMY_TAG)
	, m_mcudata(*this, finder_base::DUMMY_TAG)
	, m_gametype(GAME_NORMAL)
	, m_tabletype(TABLE_NORMAL)
{
	m_mcu_com[0] = m_mcu_com[1] = m_mcu_com[2] = m_mcu_com[3] = 0;
}

void kaneko_toybox_device::device_start()
{
	memset(m_mcu_com, 0, 4 * sizeof( uint16_t) );
	decrypt_rom();

	save_item(NAME(m_mcu_com));
}

void kaneko_toybox_device::device_reset()
{
	mcu_init();
}


#define MCU_RESPONSE(d) memcpy(&m_mcuram[mcu_offset], d, sizeof(d))


// I use a byteswapped MCU data rom to make the transfers to the 68k side easier
//  not sure if it's all 100% endian safe
void kaneko_toybox_device::decrypt_rom()
{
	for (int i = 0; i < 0x020000; i++)
	{
		if (m_tabletype == TABLE_NORMAL) m_mcudata[i] = m_mcudata[i] + decryption_table[(i ^ 1) & 0xff];
		else m_mcudata[i] = m_mcudata[i] + decryption_table_alt[(i ^ 1) & 0xff];
	}
}



void kaneko_toybox_device::handle_04_subcommand(uint8_t mcu_subcmd, uint16_t *mcu_ram)
{
	uint8_t* src = (uint8_t *)&m_mcudata[0x10000];
	uint8_t* dst = (uint8_t *)mcu_ram;
	int offs = (mcu_subcmd & 0x3f) * 8;

	//uint16_t unused = src[offs + 0] | (src[offs + 1] << 8);
	uint16_t romstart = src[offs + 2] | (src[offs + 3] << 8);
	uint16_t romlength = src[offs + 4] | (src[offs + 5] << 8);
	uint16_t ramdest = mcu_ram[0x0012 / 2];
	//uint16_t extra = src[offs + 6] | (src[offs + 7] << 8); // BONK .. important :-(

	//printf("romstart %04x length %04x\n", romstart, romlength);

	for (int x = 0; x < romlength; x++)
	{
		dst[BYTE_XOR_LE(ramdest + x)] = src[(romstart + x)];
	}
}


void kaneko_toybox_device::mcu_init()
{
	memset(m_mcu_com, 0, 4 * sizeof( uint16_t) );
}

void kaneko_toybox_device::mcu_com_w(offs_t offset, uint16_t data, uint16_t mem_mask, int _n_)
{
	COMBINE_DATA(&m_mcu_com[_n_]);
	if (m_mcu_com[0] != 0xFFFF)  return;
	if (m_mcu_com[1] != 0xFFFF)  return;
	if (m_mcu_com[2] != 0xFFFF)  return;
	if (m_mcu_com[3] != 0xFFFF)  return;

	memset(m_mcu_com, 0, 4 * sizeof( uint16_t ) );
	mcu_run();
}

void kaneko_toybox_device::mcu_com0_w(offs_t offset, uint16_t data, uint16_t mem_mask){ mcu_com_w(offset, data, mem_mask, 0); }
void kaneko_toybox_device::mcu_com1_w(offs_t offset, uint16_t data, uint16_t mem_mask){ mcu_com_w(offset, data, mem_mask, 1); }
void kaneko_toybox_device::mcu_com2_w(offs_t offset, uint16_t data, uint16_t mem_mask){ mcu_com_w(offset, data, mem_mask, 2); }
void kaneko_toybox_device::mcu_com3_w(offs_t offset, uint16_t data, uint16_t mem_mask){ mcu_com_w(offset, data, mem_mask, 3); }

/*
    bonkadv and bloodwar test bit 0
*/
uint16_t kaneko_toybox_device::mcu_status_r()
{
	logerror("%s : read MCU status\n", machine().describe_context());
	return 0; // most games test bit 0 for failure
}



void kaneko_toybox_device::mcu_run()
{
	uint16_t mcu_command  =   m_mcuram[0x0010/2];
	uint16_t mcu_offset   =   m_mcuram[0x0012/2] / 2;
	uint16_t mcu_data     =   m_mcuram[0x0014/2];

	//printf("command %04x\n",mcu_command);

	switch (mcu_command >> 8)
	{
		case 0x02:  // Read from NVRAM
		{
			uint8_t* nvdat = (uint8_t*)&m_mcuram[mcu_offset];

			for (int i = 0; i < 0x80; i += 2)
			{
				uint16_t dat = m_eeprom->internal_read(i / 2);
				nvdat[i]     = (dat & 0xff00) >> 8;
				nvdat[i + 1] = (dat & 0x00ff);
			}

			logerror("%s : MCU executed command: %04X %04X (load NVRAM settings)\n", machine().describe_context(), mcu_command, mcu_offset*2);

		}
		break;

		case 0x42:  // Write to NVRAM
		{
			uint8_t* nvdat = (uint8_t*)&m_mcuram[mcu_offset];
			for (int i = 0; i < 0x80; i += 2)
			{
				uint16_t dat = (nvdat[i] << 8) | (nvdat[i + 1]);
				m_eeprom->internal_write(i / 2, dat);
			}

			logerror("%s : MCU executed command: %04X %04X (save NVRAM settings)\n", machine().describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x43:  // Initialize NVRAM - MCU writes Default Data Set directly to NVRAM (from internal ROM, or from the data ROM?)
		{
			// only bonk seems to do this?
			if (m_gametype == GAME_BONK)
			{
				//memcpy(m_nvram_save, bonkadv_mcu_43, sizeof(bonkadv_mcu_43));
				uint8_t* nvdat = (uint8_t*)&bonkadv_mcu_43[0];
				for (int i = 0; i < 0x80; i += 2)
				{
					uint16_t dat = (nvdat[i] << 8) | (nvdat[i + 1]);
					m_eeprom->internal_write(i / 2, dat);
				}

				logerror("%s : MCU executed command: %04X %04X (restore default NVRAM settings)\n", machine().describe_context(), mcu_command, mcu_offset*2);
			}
		}
		break;

		case 0x03:  // DSW
		{
			m_mcuram[mcu_offset] = m_dsw1->read();
			logerror("%s : MCU executed command: %04X %04X (read DSW)\n", machine().describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x04:  // Protection
		{
			logerror("%s : MCU executed command: %04X %04X %04X\n", machine().describe_context(), mcu_command, mcu_offset*2, mcu_data);

			if (m_gametype == GAME_BONK)
			{
				// bonk still needs these hacks
				switch(mcu_data)
				{
					// static, in this order, at boot/reset - these aren't understood, different params in Mcu data rom, data can't be found
					case 0x34: MCU_RESPONSE(bonkadv_mcu_4_34); break;
					case 0x30: MCU_RESPONSE(bonkadv_mcu_4_30); break;
					case 0x31: MCU_RESPONSE(bonkadv_mcu_4_31); break;
					case 0x32: MCU_RESPONSE(bonkadv_mcu_4_32); break;
					case 0x33: MCU_RESPONSE(bonkadv_mcu_4_33); break;

					// dynamic, per-level (29), in level order
					default:
						handle_04_subcommand(mcu_data, m_mcuram);
						break;

				}
			}
			else
			{
				handle_04_subcommand(mcu_data, m_mcuram);
			}

		}
		break;

		default:
			logerror("%s : MCU executed command: %04X %04X %04X (UNKNOWN COMMAND)\n", machine().describe_context(), mcu_command, mcu_offset*2, mcu_data);
		break;
	}
}
