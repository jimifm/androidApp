///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IO.c>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2010/08/10
//   @fileversion: CAT6023_SRC_1.15
//******************************************/

//#include "mcu.h"
#include "io.h"
#include "cat6023.h"
#ifdef SUPPORT_HDMITX
#include "hdmitx.h"
#endif

/*
#define CLOCK           27000000
#define MachineCycle    6

#define One_Sec         1000L
*/
/*
#ifdef 	_xxxxx_
void HotPlug(BOOL Enable)
{
    RX_HPD=Enable;
    RX_HPD1=Enable;
}
#endif
void set_8051_scl( BOOL bit_value )
{
     SCL_PORT=bit_value;

}

void set_8051_sda( BOOL bit_value,BYTE device )
{
     switch( device ) {
         case 0:
             TX0_SDA_PORT=bit_value;
            break;

#ifdef _2PORT_
         case 1:
             TX1_SDA_PORT=bit_value;
            break;
#endif

#ifdef _3PORT_
         case 1:
             TX1_SDA_PORT=bit_value;
            break;
         case 2:
             TX2_SDA_PORT=bit_value;
            break;
#endif

#ifdef _4PORT_
         case 1:
             TX1_SDA_PORT=bit_value;
            break;
         case 2:
             TX2_SDA_PORT=bit_value;
            break;
         case 3:
             TX3_SDA_PORT=bit_value;
            break;
#endif

#ifdef _8PORT_
         case 1:
             TX1_SDA_PORT=bit_value;
            break;
         case 2:
             TX2_SDA_PORT=bit_value;
            break;
         case 3:
             TX3_SDA_PORT=bit_value;
            break;
         case 4:
             TX4_SDA_PORT=bit_value;
            break;
         case 5:
             TX5_SDA_PORT=bit_value;
            break;
         case 6:
             TX6_SDA_PORT=bit_value;
            break;
         case 7:
             TX7_SDA_PORT=bit_value;
            break;
#endif

         default:
             RX_SDA_PORT=bit_value;
     }

}

BOOL get_8051_sda( BYTE device )
{
     switch( device ) {
         case 0:
             TX0_SDA_PORT=1;
             return TX0_SDA_PORT;
            break;

#ifdef _2PORT_
         case 1:
             TX1_SDA_PORT=1;
             return TX1_SDA_PORT;
            break;
#endif

#ifdef _3PORT_
         case 1:
             TX1_SDA_PORT=1;
             return TX1_SDA_PORT;
            break;
         case 2:
             TX2_SDA_PORT=1;
             return TX2_SDA_PORT;
            break;
#endif

#ifdef _4PORT_
         case 1:
             TX1_SDA_PORT=1;
             return TX1_SDA_PORT;
            break;
         case 2:
             TX2_SDA_PORT=1;
             return TX2_SDA_PORT;
            break;
         case 3:
             TX3_SDA_PORT=1;
             return TX3_SDA_PORT;
            break;
#endif

#ifdef _8PORT_
         case 1:
             TX1_SDA_PORT=1;
             return TX1_SDA_PORT;
            break;
         case 2:
             TX2_SDA_PORT=1;
             return TX2_SDA_PORT;
            break;
         case 3:
             TX3_SDA_PORT=1;
             return TX3_SDA_PORT;
            break;
         case 4:
             TX4_SDA_PORT=1;
             return TX4_SDA_PORT;
            break;
         case 5:
             TX5_SDA_PORT=1;
             return TX5_SDA_PORT;
            break;
         case 6:
             TX6_SDA_PORT=1;
             return TX6_SDA_PORT;
            break;
         case 7:
             TX7_SDA_PORT=1;
             return TX7_SDA_PORT;
            break;
#endif

         default:
             RX_SDA_PORT=1;
             return RX_SDA_PORT;
     }
}

void i2c_8051_start( BYTE device )
{
    set_8051_sda( HIGH,device );
    set_8051_scl( HIGH );
    set_8051_sda( LOW,device );
    set_8051_scl( LOW );
}

void i2c_8051_write( BYTE byte_data,BYTE device )
{
 BYTE data bit_cnt,tmp;
 BOOL data bit_value;

     for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
         tmp=(byte_data << bit_cnt) & 0x80;
         bit_value=tmp && 0x80;

         set_8051_sda( bit_value,device );
         set_8051_scl( HIGH );
         set_8051_scl( LOW );
     }
}

BOOL i2c_8051_wait_ack( BYTE device )
{
 BOOL data ack_bit_value;

    set_8051_sda( HIGH,device );
    set_8051_scl( HIGH );
    ack_bit_value=get_8051_sda( device );
    set_8051_scl( LOW );

    return ack_bit_value;
}

BYTE i2c_8051_read( BYTE device )
{
 BYTE data bit_cnt,byte_data;
 BOOL data bit_value;

     byte_data=0;
     for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
         set_8051_scl( HIGH );

         bit_value=get_8051_sda( device );

         byte_data=(byte_data << 1) | bit_value;

         set_8051_scl( LOW );
     }

     return byte_data;
}

void i2c_8051_send_ack( BOOL bit_value,BYTE device )
{
     set_8051_sda( bit_value,device );
     set_8051_scl( HIGH );
     set_8051_scl( LOW );
     set_8051_sda( HIGH,device );
}

void i2c_8051_end( BYTE device )
{
     set_8051_sda( LOW,device );
     set_8051_scl( HIGH );
     set_8051_sda( HIGH,device );
}

BOOL i2c_write_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device )
{
 BYTE data i;

     i2c_8051_start(device);

     i2c_8051_write(address&0xFE,device);
     if( i2c_8051_wait_ack(device)==1 )    {
         i2c_8051_end(device);
     return 0;
      }

     i2c_8051_write(offset,device);
     if( i2c_8051_wait_ack(device)==1 )    {
         i2c_8051_end(device);
     return 0;
     }

     for(i=0; i<byteno-1; i++) {
          i2c_8051_write(*p_data,device);
          if( i2c_8051_wait_ack(device)==1 ) {
              i2c_8051_end(device);
         return 0;
         }
         p_data++;
     }

     i2c_8051_write(*p_data,device);
     if( i2c_8051_wait_ack(device)==1 )    {
          i2c_8051_end(device);
     return 0;
     }
     else {
          i2c_8051_end(device);
     return 1;
     }
}

BOOL i2c_read_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device )
{
 BYTE data i;

     i2c_8051_start(device);

     i2c_8051_write(address&0xFE,device);
     if( i2c_8051_wait_ack(device)==1 ) {
         i2c_8051_end(device);
         return 0;
     }

     i2c_8051_write(offset,device);
     if( i2c_8051_wait_ack(device)==1 ) {
         i2c_8051_end(device);
         return 0;
     }

     i2c_8051_start(device);

     i2c_8051_write(address|0x01,device);
     if( i2c_8051_wait_ack(device)==1 ) {
         i2c_8051_end(device);
         return 0;
     }

     for(i=0; i<byteno-1; i++) {
         *p_data=i2c_8051_read(device);
         i2c_8051_send_ack(LOW,device);

         p_data++;
     }

     *p_data=i2c_8051_read(device);
     i2c_8051_send_ack(HIGH,device);
     i2c_8051_end(device);

    return 1;
}

#if 0

#endif

#ifdef	 SUPPORT_HDMITX
BYTE HDMITX_ReadI2C_Byte(BYTE RegAddr)
{
 BYTE data p_data;

 i2c_read_byte(TX0ADR,RegAddr,1,&p_data,TX0DEV);

 return p_data;
}

SYS_STATUS HDMITX_WriteI2C_Byte(BYTE RegAddr,BYTE d)
{
 BOOL data flag;

 flag=i2c_write_byte(TX0ADR,RegAddr,1,&d,TX0DEV);

 return !flag;
}

SYS_STATUS HDMITX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
 BOOL data flag;

 flag=i2c_read_byte(TX0ADR,RegAddr,N,pData,TX0DEV);

 return !flag;
}

SYS_STATUS HDMITX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
 BOOL data flag;

 flag=i2c_write_byte(TX0ADR,RegAddr,N,pData,TX0DEV);

 return !flag;
}

#endif
*/
BYTE HDMIRX_ReadI2C_Byte(BYTE RegAddr)
//Dongzejun:从CAT6023读取1个字节
{
	int ret = 0;
	struct i2c_rdwr_ioctl_data work_queue;
	BYTE rdata;
	
	work_queue.nmsgs = 2; 
	work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct
		i2c_msg));
	if (!work_queue.msgs) {
		printf("Memory alloc error\n");
		//close(i2cbus_fd);
		return -1;
	}
	
	ioctl(i2cbus_fd, I2C_TIMEOUT, 2); 
	ioctl(i2cbus_fd, I2C_RETRIES, 1); 
	
	(work_queue.msgs[0]).len = 1;
	(work_queue.msgs[0]).addr = RXADR;
	(work_queue.msgs[0]).flags = 0;
	(work_queue.msgs[0]).buf = &RegAddr;
		
	(work_queue.msgs[1]).len = 1;
	(work_queue.msgs[1]).flags = I2C_M_RD;
	(work_queue.msgs[1]).addr = RXADR;
	(work_queue.msgs[1]).buf = &rdata;
	
	ret = ioctl(i2cbus_fd, I2C_RDWR, (unsigned long) &work_queue);
	if (ret < 0)
	{		
		printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
	//goto err;
	}
	else
		printf("[read ] reg:%02x val:%02x\n", RegAddr, rdata);	

	free(work_queue.msgs);
 	return rdata;

}

SYS_STATUS HDMIRX_WriteI2C_Byte(BYTE RegAddr,BYTE d)
//Dongzejun:对CAT6023写1 个字节数据
{
	int ret = 0;
	struct i2c_rdwr_ioctl_data work_queue;
	BYTE msg[2] = "";
	work_queue.nmsgs = 1; 
	work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct
		i2c_msg));
	if (!work_queue.msgs) {
		printf("Memory alloc error\n");
		//close(i2cbus_fd);
		return -1;
	}

	msg[0] = RegAddr;
	msg[1] = d;
	ioctl(i2cbus_fd, I2C_TIMEOUT, 2); 
	ioctl(i2cbus_fd, I2C_RETRIES, 1); 
	
	(work_queue.msgs[0]).len = 2;
	(work_queue.msgs[0]).flags = 0;
	(work_queue.msgs[0]).addr = RXADR;
	(work_queue.msgs[0]).buf = msg;
	
	ret = ioctl(i2cbus_fd, I2C_RDWR, (unsigned long) &work_queue);
	if (ret < 0)
	{		
		printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
		//goto err;
	}
	else
		printf("[write] reg:%02x val:%02x\n", RegAddr, d);
	
	free(work_queue.msgs);
 	return ret;

}

SYS_STATUS HDMIRX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	int ret = 0;
	struct i2c_rdwr_ioctl_data work_queue;
	//unsigned char data[2] = "";
	//data[0] = (RegAddr & 0xff00) >> 8;
	//data[1] = (RegAddr & 0x00ff);
	//unsigned char data = 0;
	
	work_queue.nmsgs = 2; 
	work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct
		i2c_msg));
	if (!work_queue.msgs) {
		printf("Memory alloc error\n");
		//close(i2cbus_fd);
		return -1;
	}
	
	ioctl(i2cbus_fd, I2C_TIMEOUT, 2); 
	ioctl(i2cbus_fd, I2C_RETRIES, 1); 
	
	(work_queue.msgs[0]).len = 1;
	(work_queue.msgs[0]).addr = RXADR;
	(work_queue.msgs[0]).flags = 0;
	(work_queue.msgs[0]).buf = &RegAddr;
		
	(work_queue.msgs[1]).len = N;
	(work_queue.msgs[1]).flags = I2C_M_RD;
	(work_queue.msgs[1]).addr = RXADR;
	(work_queue.msgs[1]).buf = pData;
	
	ret = ioctl(i2cbus_fd, I2C_RDWR, (unsigned long) &work_queue);
	if (ret < 0)
	{		
		printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
		//free(work_queue.msgs);
		//return -1;
	}
	else
	{
		int i ;
		for(i = 0; i < N; i++)
			printf("[read N] reg:%02x val:%02x\n", RegAddr + i, *(pData + i));	
	}
		
	free(work_queue.msgs);
 	return ret;
}

SYS_STATUS HDMIRX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	int ret = 0;
	struct i2c_rdwr_ioctl_data work_queue;
	PBYTE pmsg =  (PBYTE)malloc((N + 1) * sizeof(BYTE));
	if (!pmsg) {
		printf("Memory alloc error\n");
		//close(i2cbus_fd);
		return -1;
	}
	work_queue.nmsgs = 1; 
	work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct
		i2c_msg));
	if (!work_queue.msgs) {
		printf("Memory alloc error\n");
		//close(i2cbus_fd);
		return -1;
	}

	pmsg[0] = RegAddr;
	memcpy(pmsg + 1, pData, N);
	ioctl(i2cbus_fd, I2C_TIMEOUT, 2); 
	ioctl(i2cbus_fd, I2C_RETRIES, 1); 
	
	(work_queue.msgs[0]).len = 1 + N;
	(work_queue.msgs[0]).flags = 0;
	(work_queue.msgs[0]).addr = RXADR;
	(work_queue.msgs[0]).buf = pmsg;
	
	ret = ioctl(i2cbus_fd, I2C_RDWR, (unsigned long) &work_queue);
	if (ret < 0)
	{		
		printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
		//free(pmsg);
		//free(work_queue.msgs);
		//return -1;
	}
	//else
	//	printf("[write] reg:%02x val:%02x\n", RegAddr, d);
	
	free(pmsg);
	free(work_queue.msgs);
 	return ret;
}

#ifdef _EDIDI2C_

void SET_EDID_SCL( BOOL bit_value )
{
    if(EDID_ROM_DEV1==TRUE)
        EDID1_SCL=bit_value;
    else
        EDID_SCL=bit_value;

}

void SET_EDID_SDA( BOOL bit_value)
{
    if(EDID_ROM_DEV1==TRUE)
        EDID1_SDA=bit_value;
    else
        EDID_SDA=bit_value;

}

BOOL GET_EDID_SDA()
{
    BOOL Ack;
    if(EDID_ROM_DEV1==TRUE)
        Ack=EDID1_SDA;
    else
        Ack=EDID_SDA;

    return Ack;

}

void I2C_EDID_START()
{
    SET_EDID_SDA( HIGH );
    SET_EDID_SCL( HIGH );
    SET_EDID_SDA( LOW );
    SET_EDID_SCL( LOW );
}

void I2C_EDID_WRITE( BYTE byte_data )
{
    BYTE data bit_cnt,tmp;
    BOOL data bit_value;

    for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
        tmp=(byte_data << bit_cnt) & 0x80;
        bit_value=tmp && 0x80;

        SET_EDID_SDA( bit_value );
        SET_EDID_SCL( HIGH );
        SET_EDID_SCL( LOW );
    }
}

BOOL I2C_EDID_WAIT_ACK( void )
{
    BOOL data ack_bit_value;

    SET_EDID_SDA( HIGH );
    SET_EDID_SCL( HIGH );
    ack_bit_value=GET_EDID_SDA();
    SET_EDID_SCL( LOW );

    return ack_bit_value;
}

void I2C_EDID_END( void )
{
    SET_EDID_SDA( LOW );
    SET_EDID_SCL( HIGH );
    SET_EDID_SDA( HIGH );
}

BOOL EDID_WRITE_BYTE( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data )
{
    BYTE data i;

    I2C_EDID_START();

    I2C_EDID_WRITE(address&0xFE);
    if( I2C_EDID_WAIT_ACK()==1 )
        {
            I2C_EDID_END();
            return 0;
        }

    I2C_EDID_WRITE(offset);
    if( I2C_EDID_WAIT_ACK()==1 )
        {
            I2C_EDID_END();
            return 0;
        }

    for(i=0; i<byteno-1; i++)
        {
            I2C_EDID_WRITE(*p_data);
            if( I2C_EDID_WAIT_ACK()==1 )
                {
                    I2C_EDID_END();
                    return 0;
                }
            p_data++;
        }

    I2C_EDID_WRITE(*p_data);
    if( I2C_EDID_WAIT_ACK()==1 )
        {
            I2C_EDID_END();
            return 0;
        }
    else
        {
            I2C_EDID_END();
            return 1;
        }

}

#ifdef _EDIDPARSE_
SYS_STATUS HDMI_WriteEDID_Byte(BYTE RegAddr,BYTE d)
{
    BOOL  flag;
    BYTE  rddata,ErrorCnt=0;
    do {
        flag=EDID_WRITE_BYTE(EDID_ADR,RegAddr,1,&d);
        delay1ms(10);
        rddata=HDMI_ReadEDID_Byte(RegAddr);
        #ifdef Printf

        #endif
        if(ErrorCnt)delay1ms(500);
        ErrorCnt++;

        if(ErrorCnt>5)break;
    }
    while( (flag==FALSE) || (rddata!=d));

#ifndef RX_SINGAL_PORT
    EDID_ROM_DEV1=TRUE;
    ErrorCnt=0;
    do {
        flag=EDID_WRITE_BYTE(EDID_ADR,RegAddr,1,&d);
        delay1ms(10);
        rddata=HDMI_ReadEDID_Byte(RegAddr);
        #ifdef Printf

        #endif
        if(ErrorCnt)delay1ms(500);
        ErrorCnt++;

        if(ErrorCnt>5)break;
    }
    while( (flag==FALSE) || (rddata!=d));

    EDID_ROM_DEV1=FALSE;
#endif
    return !flag;

}
#else
#if 0
SYS_STATUS HDMI_WriteEDID_Byte(BYTE RegAddr,BYTE d)
{
    BYTE flag=0;
    EDID_WRITE_BYTE(EDID_ADR,RegAddr,1,&d);
#ifndef RX_SINGAL_PORT
    EDID_ROM_DEV1=TRUE;
    EDID_WRITE_BYTE(EDID_ADR,RegAddr,1,&d);
    EDID_ROM_DEV1=FALSE;
#endif
    return !flag;

}
#endif
#endif

/*
SYS_STATUS HDMI_WriteEDID_ByteN(SHORT RegAddr,BYTE *pData,int N)
{
    BOOL data flag;

    flag=EDID_WRITE_BYTE(EDID_ADR,RegAddr,N,pData);

    return !flag;
}
*/

BYTE I2C_EDID_READ( void )
{
    BYTE data bit_cnt,byte_data;
    BOOL data bit_value;

    byte_data=0;
    for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
        SET_EDID_SCL( HIGH );

        bit_value=GET_EDID_SDA();

        byte_data=(byte_data << 1) | bit_value;

        SET_EDID_SCL( LOW );
    }

    return byte_data;
}

void I2C_EDID_SEND_ACK( BOOL bit_value )
{
    SET_EDID_SDA( bit_value );
    SET_EDID_SCL( HIGH );
    SET_EDID_SCL( LOW );
    SET_EDID_SDA( HIGH );
}

BOOL EDID_READ_BYTE( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data )
{
    BYTE data i;

    I2C_EDID_START();

    I2C_EDID_WRITE(address&0xFE);
    if( I2C_EDID_WAIT_ACK()==1 ) {
        I2C_EDID_END();
        return 0;
    }

    I2C_EDID_WRITE(offset);
    if( I2C_EDID_WAIT_ACK()==1 ) {
        I2C_EDID_END();
        return 0;
    }

    I2C_EDID_START();

    I2C_EDID_WRITE(address|0x01);
    if( I2C_EDID_WAIT_ACK()==1 ) {
        I2C_EDID_END();
        return 0;
    }

    for(i=0; i<byteno-1; i++) {
        *p_data=I2C_EDID_READ();
        I2C_EDID_SEND_ACK(LOW);

        p_data++;
    }

    *p_data=I2C_EDID_READ();
    I2C_EDID_SEND_ACK(HIGH);
    I2C_EDID_END();

    return 1;
}

BYTE HDMI_ReadEDID_Byte(BYTE RegAddr)
{
    BYTE data p_data;

    EDID_READ_BYTE(EDID_ADR,RegAddr,1,&p_data);
    return p_data;
}

/*
SYS_STATUS HDMI_ReadEDID_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
    BOOL data flag;

    flag=EDID_READ_BYTE(EDID_ADR,RegAddr,N,pData);

    return !flag;
}
*/

#endif

#ifdef RX_KEY_SWITCH

#define POWER_KEY_RELEASE  1
#define POWER_KEY_PRESS       0
BYTE GetswitchRx(){
      BYTE KEY_in;
      KEY_in=RX_SWITCH_KEY;
      switch(KEY_in){
               case POWER_KEY_RELEASE :
                    return 0;

            case POWER_KEY_PRESS:
                 delay1ms(1);
                 KEY_in=RX_SWITCH_KEY;
                 if (KEY_in==POWER_KEY_RELEASE){
                     return 0;
                 }else{
                     delay1ms(20);
                 }

       }             return 1;

}

#endif

void delay1ms(USHORT ms)
{
	usleep(ms * 1000);
#ifdef H_DEYAY
	struct timeval tv;
	tv.tv_sec = ms /1000;
	tv.tv_usec = (ms%1000) * 1000;
	select(0, NULL, NULL, NULL, &tv);
#endif
}
#if 0
void HoldSystem(void)
{
	P1_5=1;

	while((!P1_5));

}
#endif
