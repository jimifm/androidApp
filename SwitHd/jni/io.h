///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <io.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2010/08/10
//   @fileversion: CAT6023_SRC_1.15
//******************************************/

#ifndef _IO_h_
#define _IO_h_
//#include "Mcu.h"
//#include "Reg_c51.h"
#include "typedef.h"//add by zhanghong
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <linux/i2c.h>
#include "i2c-dev.h"


BYTE HDMIRX_ReadI2C_Byte(BYTE RegAddr);
SYS_STATUS HDMIRX_WriteI2C_Byte(BYTE RegAddr,BYTE d);

SYS_STATUS HDMIRX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N);

SYS_STATUS HDMIRX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N);
#define HDMIRX_OrReg_Byte(reg,ormask) HDMIRX_WriteI2C_Byte(reg,(HDMIRX_ReadI2C_Byte(reg) | (ormask)))
#define HDMIRX_AndReg_Byte(reg,andmask) HDMIRX_WriteI2C_Byte(reg,(HDMIRX_ReadI2C_Byte(reg) & (andmask)))
#define HDMIRX_SetReg_Byte(reg,andmask,ormask) HDMIRX_WriteI2C_Byte(reg,((HDMIRX_ReadI2C_Byte(reg) & (andmask))|(ormask)))

#ifdef _EDIDI2C_

SYS_STATUS HDMI_WriteEDID_Byte(BYTE RegAddr,BYTE d);
BOOL EDID_WRITE_BYTE( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data );
#ifdef _ENPARSE_
BYTE I2C_EDID_READ( void );
void I2C_EDID_SEND_ACK( BOOL bit_value );
BOOL EDID_READ_BYTE( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data );
BYTE HDMI_ReadEDID_Byte(BYTE RegAddr);
#endif
#endif

//#define ErrorF iprintf

extern    BYTE    EDID_ROM_DEV1;
#ifdef RX_KEY_SWITCH
BYTE GetswitchRx();
#endif

#ifdef 	_xxxxx_
void HotPlug(BOOL Enable);
#endif

void delay1ms(USHORT ms);
void HoldSystem(void);

int i2cbus_fd;
#endif
