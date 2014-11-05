///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <cat6023.c>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2010/08/10
//   @fileversion: CAT6023_SRC_1.15
//******************************************/

#define OUTPUT_24BIT_RGB444

/*********************************************************************************
 * CAT6023 HDMI RX sample code                                                   *
 *********************************************************************************/
#include "cat6023.h"

#ifndef DEBUG_PORT_ENABLE
#define DEBUG_PORT_ENABLE 0
#else
#pragma message("DEBUG_PORT_ENABLE defined\n")
#endif

#define SetSPDIFMUTE(x) SetMUTE(~(1<<O_TRI_SPDIF),(x)?(1<<O_TRI_SPDIF):0)
#define SetI2S3MUTE(x) SetMUTE(~(1<<O_TRI_I2S3),  (x)?(1<<O_TRI_I2S3):0)
#define SetI2S2MUTE(x) SetMUTE(~(1<<O_TRI_I2S2),  (x)?(1<<O_TRI_I2S2):0)
#define SetI2S1MUTE(x) SetMUTE(~(1<<O_TRI_I2S1),  (x)?(1<<O_TRI_I2S1):0)
#define SetI2S0MUTE(x) SetMUTE(~(1<<O_TRI_I2S0),  (x)?(1<<O_TRI_I2S0):0)
//Dongzejun:¾²Òô²Ù×÷
#define SetALLMute() SetMUTE(B_VDO_MUTE_DISABLE,(B_VDO_MUTE_DISABLE|B_TRI_ALL))

#define SwitchHDMIRXBank(x) HDMIRX_WriteI2C_Byte(REG_RX_BANK, (x)&1)

char _CODE * VStateStr[] = {
    "VSTATE_PwrOff",
    "VSTATE_SyncWait ",
    "VSTATE_SWReset",
    "VSTATE_SyncChecking",
    "VSTATE_HDCPSet",
    "VSTATE_HDCP_Reset",
    "VSTATE_ModeDetecting",
    "VSTATE_VideoOn",
    "VSTATE_Reserved"
} ;

char _CODE *AStateStr[] = {
    "ASTATE_AudioOff",
    "ASTATE_RequestAudio",
    "ASTATE_ResetAudio",
    "ASTATE_WaitForReady",
    "ASTATE_AudioOn",
    "ASTATE_Reserved"
};
#define OUTPUT_SYNC_EMBEDDED 1//add by zhanghong 10.08
#if defined(OUTPUT_CCIR656)
    #pragma message("OUTPUT_CCIR656 defined")  				
    #define HDMIRX_OUTPUT_MAPPING   (B_OUTPUT_16BIT)				//Dongzejun:"OUTPUT_CCIR656 defined"
    #define HDMIRX_OUTPUT_TYPE (B_SYNC_EMBEDDED|B_CCIR565)												//Dongzejun:"OUTPUT_CCIR656 defined"
    #define HDMIRX_OUTPUT_COLORMODE (B_OUTPUT_YUV422<<O_OUTPUT_COLOR_MODE)	//Dongzejun:"OUTPUT_CCIR656 defined"
#elif defined(OUTPUT_SYNC_EMBEDDED)
    #pragma message("OUTPUT_SYNC_EMBEDDED defined")
    #define HDMIRX_OUTPUT_MAPPING   (B_OUTPUT_16BIT)			//Dongzejun:"OUTPUT_SYNC_EMBEDDED defined"
    #define HDMIRX_OUTPUT_TYPE (B_SYNC_EMBEDDED)				//Dongzejun:"OUTPUT_SYNC_EMBEDDED defined"
    #define HDMIRX_OUTPUT_COLORMODE (B_OUTPUT_YUV422<<O_OUTPUT_COLOR_MODE)  	//Dongzejun:"OUTPUT_SYNC_EMBEDDED defined"
#elif defined(OUTPUT_16BIT_YUV422)
    #pragma message("OUTPUT_16BIT_YUV422 defined")
    #define HDMIRX_OUTPUT_MAPPING   (B_OUTPUT_16BIT)			//Dongzejun:"OUTPUT_16BIT_YUV422 defined"
    #define HDMIRX_OUTPUT_TYPE 0														//Dongzejun:"OUTPUT_16BIT_YUV422 defined"
    #define HDMIRX_OUTPUT_COLORMODE (B_OUTPUT_YUV422<<O_OUTPUT_COLOR_MODE)		//Dongzejun:"OUTPUT_16BIT_YUV422 defined"
#elif defined(OUTPUT_24BIT_YUV422)
    #pragma message("OUTPUT_24BIT_YUV422 defined")
    #define HDMIRX_OUTPUT_MAPPING   0			//Dongzejun:"OUTPUT_24BIT_YUV422 defined"
    #define HDMIRX_OUTPUT_TYPE  0					//Dongzejun:"OUTPUT_24BIT_YUV422 defined"
    #define HDMIRX_OUTPUT_COLORMODE (B_OUTPUT_YUV422<<O_OUTPUT_COLOR_MODE)		//Dongzejun:"OUTPUT_24BIT_YUV422 defined"
#elif defined(OUTPUT_24BIT_YUV444)
    #pragma message("OUTPUT_24BIT_YUV444 defined")
    #define HDMIRX_OUTPUT_MAPPING   0			//Dongzejun:"OUTPUT_24BIT_YUV444 defined"
    #define HDMIRX_OUTPUT_TYPE  0					//Dongzejun:"OUTPUT_24BIT_YUV444 defined"
    #define HDMIRX_OUTPUT_COLORMODE (B_OUTPUT_YUV444<<O_OUTPUT_COLOR_MODE)	//Dongzejun:"OUTPUT_24BIT_YUV444 defined"
#else
    #pragma message("OUTPUT_24BIT_RGB444 defined")
    #define HDMIRX_OUTPUT_MAPPING   0		//Dongzejun:Reg0x1B//Dongzejun:"OUTPUT_24BIT_RGB444 defined"
    #define HDMIRX_OUTPUT_TYPE  0				//Dongzejun:"OUTPUT_24BIT_RGB444 defined"
    #define HDMIRX_OUTPUT_COLORMODE (B_OUTPUT_RGB24<<O_OUTPUT_COLOR_MODE)		//Dongzejun:"OUTPUT_24BIT_RGB444 defined"
#endif

#define DEFAULT_START_FIXED_AUD_SAMPLEFREQ AUDFS_192KHz		//Dongzejun:²ÉÑùÆµÂÊ³õÊ¼»¯ÉèÖÃ£¬Òª¸ü¸ÄµÄ

BYTE	ucDVISCDToffCNT=0;	//Dongzejun:DVIÄ£Ê½ÏÂSCDTÑÓÊ±¼ÆÊý¼ì²â
_IDATA Video_State_Type VState = VSTATE_PwrOff ;
_IDATA Audio_State_Type AState = ASTATE_AudioOff ;	//Dongzejun:

static _IDATA USHORT VideoCountingTimer = 0 ;	//Dongzejun:Video×´Ì¬¼ÆÊýÆ÷
static _IDATA USHORT AudioCountingTimer = 0 ;
static _IDATA USHORT MuteResumingTimer = 0 ;
static BOOL MuteAutoOff = FALSE ;
static _IDATA BYTE bGetSyncFailCount = 0 ;

//Dongzejun:Êä³öÊÓÆµ¸ñÊ½ÉèÖÃ
static BYTE _IDATA bOutputVideoMode = F_MODE_EN_UDFILT | F_MODE_YUV422  ;	
//bit1:0 ->Êä³öÉ«²Ê¿Õ¼ä
//bit6:	-> UP /Down 444<-->422



static BOOL EnaSWCDRRest = FALSE ;	//Dongzejun:´Ë±äÁ¿³¤Îª0£¬Î´Ê¹ÓÃ

//Dongzejun:
BYTE _XDATA bDisableAutoAVMute = 0 ;	//Dongzejun:´ËÈ«¾Ö±äÁ¿Î´ÓÃ

BYTE _XDATA bHDCPMode = 0 ;

#define LOOP_MSEC 32
#define MS_TimeOut(x) (((x)+LOOP_MSEC-1)/LOOP_MSEC)

#define VSTATE_MISS_SYNC_COUNT MS_TimeOut(8000)	// 8Ãë
#define VSATE_CONFIRM_SCDT_COUNT MS_TimeOut(150)	//È·ÈÏSCDT×î´ó¼ÆÊýÆ÷
#define AUDIO_READY_TIMEOUT MS_TimeOut(200)
#define AUDIO_CLEARERROR_TIMEOUT MS_TimeOut(1000)
#define MUTE_RESUMING_TIMEOUT MS_TimeOut(2500)
#define HDCP_WAITING_TIMEOUT MS_TimeOut(3000)
#define CDRRESET_TIMEOUT MS_TimeOut(3000)
#define VSTATE_SWRESET_COUNT MS_TimeOut(500)
#define FORCE_SWRESET_TIMEOUT  MS_TimeOut(15000)
#define VIDEO_TIMER_CHECK_COUNT MS_TimeOut(250)

#define SCDT_LOST_TIMEOUT  15
static _XDATA USHORT SWResetTimeOut = FORCE_SWRESET_TIMEOUT;	//Dongzejun:¸´Î»¼ÆÊý

static _XDATA BYTE ucHDMIAudioErrorCount = 0 ;

	//Dongzejun:²ÉÑùÆµÂÊ³õÊ¼»¯ÉèÖÃ£¬Òª¸ü¸ÄµÄ
static _XDATA BYTE ucAudioSampleClock = DEFAULT_START_FIXED_AUD_SAMPLEFREQ ;


BOOL bIntPOL = FALSE ;	//Dongzejun:ÖÐ¶Ï¼«ÐÔÉèÖÃ
static BOOL NewAVIInfoFrameF = FALSE ;	//Dongzejun:½ÓÊÕÒ»¸öÐÂµÄAVI¡¡Ö¡=1
static BOOL MuteByPKG = OFF ;		//Dongzejun:Í¨¹ýPKGÉèÖÃ¾²Òô£¨ ÐÅÏ¢°ü £© 0:´ò¿ªÒôÆµ£»1:½ûÒô

//Dongzejun:
static _XDATA BYTE bInputVideoMode ;	//Dongzejun:×¢Òâ´Ë±äÁ¿¶¨Òå
// bit1 /bit0  :Êä³öÉ«²Ê¿Õ¼ä
// bit4: 1->F_MODE_ITU709 0->F_MODE_ITU601
// bit5:F_MODE_16_235  1->16-235    0->0-255

static _XDATA BYTE prevAVIDB1 = 0 ;
static _XDATA BYTE prevAVIDB2 = 0 ;

static _XDATA USHORT currHTotal ;
static _XDATA BYTE currXcnt ;
static BOOL currScanMode ;
static BOOL bGetSyncInfo() ;

static _XDATA AUDIO_CAPS AudioCaps ;

_XDATA VTiming s_CurrentVM ;

BYTE SCDTErrorCnt = 0;	//Dongzejun:Reg0x13[3]ÖÐ¶ÏÖÐ¼ÆÊý ÔÚÈí¼þ¸´Î»º¯Êý"SWReset_HDMIRX()"ÖÐÇå0

#ifdef USE_MODE_TABLE
static VTiming _CODE s_VMTable[] = {
    {640,480,800,525,25175L,0x89,16,96,48,10,2,33,PROG,Vneg,Hneg},
    {720,480,858,525,27000L,0x80,16,62,60,9,6,30,PROG,Vneg,Hneg},
    {1280,720,1650,750,74000L,0x2E,110,40,220,5,5,20,PROG,Vpos,Hpos},
    {1920,540,2200,562,74000L,0x2E,88,44,148,2,5,15,INTERLACE,Vpos,Hpos},
    {720,240,858,262,13500L,0xFF,19,62,57,4,3,15,INTERLACE,Vneg,Hneg},
    {720,240,858,262,13500L,0xFF,19,62,57,4,3,15,PROG,Vneg,Hneg},
    {1440,240,1716,262,27000L,0x80,38,124,114,5,3,15,INTERLACE,Vneg,Hneg},
    {1440,240,1716,263,27000L,0x80,38,124,114,5,3,15,PROG,Vneg,Hneg},
    {2880,240,3432,262,54000L,0x40,76,248,288,4,3,15,INTERLACE,Vneg,Hneg},
    {2880,240,3432,262,54000L,0x40,76,248,288,4,3,15,PROG,Vneg,Hneg},
    {2880,240,3432,263,54000L,0x40,76,248,288,5,3,15,PROG,Vneg,Hneg},
    {1440,480,1716,525,54000L,0x40,32,124,120,9,6,30,PROG,Vneg,Hneg},
    {1920,1080,2200,1125,148352L,0x17,88,44,148,4,5,36,PROG,Vpos,Hpos},
    {720,576,864,625,27000L,0x80,12,64,68,5,5,36,PROG,Vneg,Hneg},
    {1280,720,1980,750,74000L,0x2E,440,40,220,5,5,20,PROG,Vpos,Hpos},
    {1920,540,2640,562,74000L,0x2E,528,44,148,2,5,15,INTERLACE,Vpos,Hpos},
    {1440/2,288,1728/2,312,13500L,0xFF,24/2,126/2,138/2,2,3,19,INTERLACE,Vneg,Hneg},
    {1440,288,1728,312,27000L,0x80,24,126,138,2,3,19,INTERLACE,Vneg,Hneg},
    {1440/2,288,1728/2,312,13500L,0xFF,24/2,126/2,138/2,2,3,19,PROG,Vneg,Hneg},
    {1440,288,1728,313,27000L,0x80,24,126,138,3,3,19,PROG,Vneg,Hneg},
    {1440,288,1728,314,27000L,0x80,24,126,138,4,3,19,PROG,Vneg,Hneg},
    {2880,288,3456,312,54000L,0x40,48,252,276,2,3,19,INTERLACE,Vneg,Hneg},
    {2880,288,3456,312,54000L,0x40,48,252,276,2,3,19,PROG,Vneg,Hneg},
    {2880,288,3456,313,54000L,0x40,48,252,276,3,3,19,PROG,Vneg,Hneg},
    {2880,288,3456,314,54000L,0x40,48,252,276,4,3,19,PROG,Vneg,Hneg},
    {1440,576,1728,625,54000L,0x40,24,128,136,5,5,39,PROG,Vpos,Hneg},
    {1920,1080,2640,1125,148000L,0x17,528,44,148,4,5,36,PROG,Vpos,Hpos},
    {1920,1080,2750,1125,74000L,0x2E,638,44,148,4,5,36,PROG,Vpos,Hpos},
    {1920,1080,2640,1125,74000L,0x2E,528,44,148,4,5,36,PROG,Vpos,Hpos},
    {1920,1080,2200,1125,74000L,0x2E,88,44,148,4,5,36,PROG,Vpos,Hpos},

    {640,350,832,445,31500L,0x6D,32,64,96,32,3,60,PROG,Vneg,Hpos},
    {640,400,832,445,31500L,0x6D,32,64,96,1,3,41,PROG,Vneg,Hneg},
    {832,624,1152,667,57283L,0x3C,32,64,224,1,3,39,PROG,Vneg,Hneg},
    {720,350,900,449,28322L,0x7A,18,108,54,59,2,38,PROG,Vneg,Hneg},
    {720,400,900,449,28322L,0x7A,18,108,54,13,2,34,PROG,Vpos,Hneg},
    {720,400,936,446,35500L,0x61,36,72,108,1,3,42,PROG,Vpos,Hneg},
    {640,480,800,525,25175L,0x89,16,96,48,10,2,33,PROG,Vneg,Hneg},
    {640,480,832,520,31500L,0x6D,24,40,128,9,3,28,PROG,Vneg,Hneg},
    {640,480,840,500,31500L,0x6D,16,64,120,1,3,16,PROG,Vneg,Hneg},
    {640,480,832,509,36000L,0x60,56,56,80,1,3,25,PROG,Vneg,Hneg},
    {800,600,1024,625,36000L,0x60,24,72,128,1,2,22,PROG,Vpos,Hpos},
    {800,600,1056,628,40000L,0x56,40,128,88,1,4,23,PROG,Vpos,Hpos},
    {800,600,1040,666,50000L,0x45,56,120,64,37,6,23,PROG,Vpos,Hpos},
    {800,600,1056,625,49500L,0x45,16,80,160,1,3,21,PROG,Vpos,Hpos},
    {800,600,1048,631,56250L,0x3D,32,64,152,1,3,27,PROG,Vpos,Hpos},
    {848,480,1088,517,33750L,0x66,16,112,112,6,8,23,PROG,Vpos,Hpos},
    {1024,384,1264,408,44900L,0x4d,8,176,56,0,4,20,INTERLACE,Vpos,Hpos},
    {1024,768,1344,806,65000L,0x35,24,136,160,3,6,29,PROG,Vneg,Hneg},
    {1024,768,1328,806,75000L,0x2E,24,136,144,3,6,29,PROG,Vneg,Hneg},
    {1024,768,1312,800,78750L,0x2B,16,96,176,1,3,28,PROG,Vpos,Hpos},
    {1024,768,1376,808,94500L,0x24,48,96,208,1,3,36,PROG,Vpos,Hpos},
    {1152,864,1600,900,108000L,0x20,64,128,256,1,3,32,PROG,Vpos,Hpos},
    {1280,768,1440,790,68250L,0x32,48,32,80,3,7,12,PROG,Vneg,Hpos},
    {1280,768,1664,798,79500L,0x2B,64,128,192,3,7,20,PROG,Vpos,Hneg},
    {1280,768,1696,805,102250L,0x21,80,128,208,3,7,27,PROG,Vpos,Hneg},
    {1280,768,1712,809,117500L,0x1D,80,136,216,3,7,31,PROG,Vpos,Hneg},
    {1280,960,1800,1000,108000L,0x20,96,112,312,1,3,36,PROG,Vpos,Hpos},
    {1280,960,1728,1011,148500L,0x17,64,160,224,1,3,47,PROG,Vpos,Hpos},
    {1280,1024,1688,1066,108000L,0x20,48,112,248,1,3,38,PROG,Vpos,Hpos},
    {1280,1024,1688,1066,135000L,0x19,16,144,248,1,3,38,PROG,Vpos,Hpos},
    {1280,1024,1728,1072,157500L,0x15,64,160,224,1,3,44,PROG,Vpos,Hpos},
    {1360,768,1792,795,85500L,0x28,64,112,256,3,6,18,PROG,Vpos,Hpos},
    {1400,1050,1560,1080,101000L,0x22,48,32,80,3,4,23,PROG,Vneg,Hpos},
    {1400,1050,1864,1089,121750L,0x1C,88,144,232,3,4,32,PROG,Vpos,Hneg},
    {1400,1050,1896,1099,156000L,0x16,104,144,248,3,4,42,PROG,Vpos,Hneg},
    {1400,1050,1912,1105,179500L,0x13,104,152,256,3,4,48,PROG,Vpos,Hneg},
    {1440,900,1600,926,88750L,0x26,48,32,80,3,6,17,PROG,Vneg,Hpos},
    {1440,900,1904,934,106500L,0x20,80,152,232,3,6,25,PROG,Vpos,Hneg},
    {1440,900,1936,942,136750L,0x19,96,152,248,3,6,33,PROG,Vpos,Hneg},
    {1440,900,1952,948,157000L,0x16,104,152,256,3,6,39,PROG,Vpos,Hneg},
    {1600,1200,2160,1250,162000L,0x15,64,192,304,1,3,46,PROG,Vpos,Hpos},
    {1600,1200,2160,1250,175500L,0x13,64,192,304,1,3,46,PROG,Vpos,Hpos},
    {1600,1200,2160,1250,189000L,0x12,64,192,304,1,3,46,PROG,Vpos,Hpos},
    {1600,1200,2160,1250,202500L,0x11,64,192,304,1,3,46,PROG,Vpos,Hpos},
    {1600,1200,2160,1250,229500L,0x0F,64,192,304,1,3,46,PROG,Vpos,Hpos},
    {1680,1050,1840,1080,119000L,0x1D,48,32,80,3,6,21,PROG,Vneg,Hpos},
    {1680,1050,2240,1089,146250L,0x17,104,176,280,3,6,30,PROG,Vpos,Hneg},
    {1680,1050,2272,1099,187000L,0x12,120,176,296,3,6,40,PROG,Vpos,Hneg},
    {1680,1050,2288,1105,214750L,0x10,128,176,304,3,6,46,PROG,Vpos,Hneg},
    {1792,1344,2448,1394,204750L,0x10,128,200,328,1,3,46,PROG,Vpos,Hneg},
    {1792,1344,2456,1417,261000L,0x0D,96,216,352,1,3,69,PROG,Vpos,Hneg},
    {1856,1392,2528,1439,218250L,0x0F,96,224,352,1,3,43,PROG,Vpos,Hneg},
    {1856,1392,2560,1500,288000L,0x0C,128,224,352,1,3,104,PROG,Vpos,Hneg},
    {1920,1200,2080,1235,154000L,0x16,48,32,80,3,6,26,PROG,Vneg,Hpos},
    {1920,1200,2592,1245,193250L,0x11,136,200,336,3,6,36,PROG,Vpos,Hneg},
    {1920,1200,2608,1255,245250L,0x0E,136,208,344,3,6,46,PROG,Vpos,Hneg},
    {1920,1200,2624,1262,281250L,0x0C,144,208,352,3,6,53,PROG,Vpos,Hneg},
    {1920,1440,2600,1500,234000L,0x0E,128,208,344,1,3,56,PROG,Vpos,Hneg},
    {1920,1440,2640,1500,297000L,0x0B,144,224,352,1,3,56,PROG,Vpos,Hneg},
};

#define     SizeofVMTable    (sizeof(s_VMTable)/sizeof(VTiming))
#else
#define     SizeofVMTable    0
#endif

static BYTE _CODE bCSCOffset_16_235[] =
{
    0x00, 0x80, 0x00
};

static BYTE _CODE bCSCOffset_0_255[] =
{
    0x10, 0x80, 0x10
};

#ifdef OUTPUT_YUV
    _CODE BYTE bCSCMtx_RGB2YUV_ITU601_16_235[] =
    {
        0xB2,0x04,0x64,0x02,0xE9,0x00,
        0x93,0x3C,0x18,0x04,0x56,0x3F,
        0x49,0x3D,0x9F,0x3E,0x18,0x04
    } ;

    _CODE BYTE bCSCMtx_RGB2YUV_ITU601_0_255[] =
    {
        0x09,0x04,0x0E,0x02,0xC8,0x00,
        0x0E,0x3D,0x84,0x03,0x6E,0x3F,
        0xAC,0x3D,0xD0,0x3E,0x84,0x03
    } ;

    _CODE BYTE bCSCMtx_RGB2YUV_ITU709_16_235[] =
    {
        0xB8,0x05,0xB4,0x01,0x93,0x00,
        0x49,0x3C,0x18,0x04,0x9F,0x3F,
        0xD9,0x3C,0x10,0x3F,0x18,0x04
    } ;

    _CODE BYTE bCSCMtx_RGB2YUV_ITU709_0_255[] =
    {
        0xE5,0x04,0x78,0x01,0x81,0x00,
        0xCE,0x3C,0x84,0x03,0xAE,0x3F,
        0x49,0x3D,0x33,0x3F,0x84,0x03
    } ;
#endif

#ifdef OUTPUT_RGB

    _CODE BYTE bCSCMtx_YUV2RGB_ITU601_16_235[] =
    {
        0x00,0x08,0x6A,0x3A,0x4F,0x3D,
        0x00,0x08,0xF7,0x0A,0x00,0x00,
        0x00,0x08,0x00,0x00,0xDB,0x0D
    } ;

    _CODE BYTE bCSCMtx_YUV2RGB_ITU601_0_255[] =
    {
        0x4F,0x09,0x81,0x39,0xDF,0x3C,
        0x4F,0x09,0xC2,0x0C,0x00,0x00,
        0x4F,0x09,0x00,0x00,0x1E,0x10
    } ;

    _CODE BYTE bCSCMtx_YUV2RGB_ITU709_16_235[] =
    {
        0x00,0x08,0x53,0x3C,0x89,0x3E,
        0x00,0x08,0x51,0x0C,0x00,0x00,
        0x00,0x08,0x00,0x00,0x87,0x0E
    } ;

    _CODE BYTE bCSCMtx_YUV2RGB_ITU709_0_255[] =
    {
        0x4F,0x09,0xBA,0x3B,0x4B,0x3E,
        0x4F,0x09,0x56,0x0E,0x00,0x00,
        0x4F,0x09,0x00,0x00,0xE7,0x10
    } ;
#endif

static BYTE ucCurrentHDMIPort = 0 ;	//Dongzejun:ÉèÖÃÊ¹ÓÃµÄHDMI¶Ë¿ÚA»¹ÊÇB
static BOOL AcceptCDRReset = TRUE ;	//Dongzejun: ³õÊ¼ÉèÖÃÎª1

BOOL CheckHDMIRX() ;

void DumpHDMIRX() ;

void Interrupt_Handler() ;
void Timer_Handler() ;
void Video_Handler() ;

static void HWReset_HDMIRX() ;
static void SWReset_HDMIRX() ;
static void Terminator_Reset() ;
static void Terminator_Off() ;
static void Terminator_On() ;

void Check_RDROM() ;
void RDROM_Reset() ;
void SetDefaultRegisterValue() ;

static void LoadCustomizeDefaultSetting() ;
static void SetupAudio() ;

BOOL ReadRXIntPin() ;

static void ClearIntFlags(BYTE flag) ;
static void ClearHDCPIntFlags() ;
BOOL IsSCDT() ;
BOOL CheckPlg5VPwr() ;

BOOL CheckHDCPFail() ;
void SetMUTE(BYTE AndMask, BYTE OrMask) ;
void SetMCLKInOUt(BYTE MCLKSelect) ;
void SetIntMask1(BYTE AndMask,BYTE OrMask) ;
void SetIntMask2(BYTE AndMask,BYTE OrMask) ;
void SetIntMask3(BYTE AndMask,BYTE OrMask) ;
void SetIntMask4(BYTE AndMask,BYTE OrMask) ;
void SetGeneralPktType(BYTE type) ;
BOOL IsCAT6023HDMIMode() ;
static void EnableAudio() ;

#define SetForceHWMute() { SetHWMuteCTRL((~B_HW_FORCE_MUTE),(B_HW_FORCE_MUTE)) ; }
#define SetHWMuteClrMode() { SetHWMuteCTRL((~B_HW_AUDMUTE_CLR_MODE),(B_HW_AUDMUTE_CLR_MODE)) ;}  //dONGZEJUN:

//Dongzejun:
#define SetHWMuteClr() 				{ SetHWMuteCTRL((~B_HW_MUTE_CLR),(B_HW_MUTE_CLR)) ; }
#define SetHWMuteEnable() 		{ SetHWMuteCTRL((~B_HW_MUTE_EN),(B_HW_MUTE_EN)) ; }
#define ClearForceHWMute() 		{ SetHWMuteCTRL((~B_HW_FORCE_MUTE),0) ; }
#define ClearHWMuteClrMode() { SetHWMuteCTRL((~B_HW_AUDMUTE_CLR_MODE),0) ; }
#define ClearHWMuteClr() 			{ SetHWMuteCTRL((~B_HW_MUTE_CLR),0) ; }
#define ClearHWMuteEnable() 	{ SetHWMuteCTRL((~B_HW_MUTE_EN),0) ;}

void RXINT_5V_PwrOn() ;
void RXINT_5V_PwrOff() ;
void RXINT_SCDT_On() ;
void RXINT_SCDT_Off() ;
void RXINT_RXCKON() ;
void RXINT_VideoMode_Chg() ;
void RXINT_HDMIMode_Chg() ;
void RXINT_AVMute_Set() ;
void RXINT_AVMute_Clear() ;
void RXINT_SetNewAVIInfo() ;
void RXINT_ResetAudio() ;
void RXINT_ResetHDCP() ;
void TimerServiceISR() ;
static void VideoTimerHandler() ;
static void AudioTimerHandler() ;
static void MuteProcessTimerHandler() ;

void AssignVideoTimerTimeout(USHORT TimeOut) ;
void ResetVideoTimerTimeout() ;
void SwitchVideoState(Video_State_Type state) ;

void AssignAudioTimerTimeout(USHORT TimeOut) ;
void ResetAudioTimerTimeout() ;
void SwitchAudioState(Audio_State_Type state) ;
#define EnableMuteProcessTimer() { MuteResumingTimer = MuteByPKG?MUTE_RESUMING_TIMEOUT:0 ; }
#define DisableMuteProcessTimer() { MuteResumingTimer = 0 ; }

static void DumpSyncInfo(VTiming *pVTiming) ;

//Dongzejun:MuteAutoOff±äÁ¿Öµ£¬ÊÜµ½Reg0x14[7]/[0]Á½¸ö±êÖ¾Î»Ó°Ïì
#define StartAutoMuteOffTimer() { MuteAutoOff = ON ; }	//Dongzejun:=1
#define EndAutoMuteOffTimer() { MuteAutoOff = OFF ; }	//Dongzejun:=0
static void CDR_Reset() ;
static void Reset_SCDTOFF() ;

static void SetVideoInputFormatWithoutInfoFrame(BYTE bInMode) ;
static void SetColorimetryByMode(/* PSYNC_INFO pSyncInfo */) ;
void SetVideoInputFormatWithInfoFrame() ;
BOOL SetColorimetryByInfoFrame() ;
void SetColorSpaceConvert() ;

void HDCP_Reset() ;
void SetDVIVideoOutput() ;
void SetNewInfoVideoOutput() ;
void ResetAudio() ;
void SetHWMuteCTRL(BYTE AndMask, BYTE OrMask) ;
void SetAudioMute(BOOL bMute) ;
static void SetVideoMute(BOOL bMute) ;

#ifndef _MCU_
void delay1ms(USHORT ms) ;
//void ErrorF(char *fmt,...) ;
#endif

void Check_HDMInterrupt()
//Dongzejun:ÖÐ¶ÏÒý½Å±ä»¯£¬½«¼ì²éÖÐ¶Ï
{
	Interrupt_Handler() ;
}

BOOL CheckHDMIRX()
//Dongzejun:
{
    Timer_Handler() ;
    Video_Handler() ;

    if( VState == VSTATE_VideoOn && (!MuteByPKG))
    {
        return TRUE ;
    }

    return FALSE ;
}

void
SelectHDMIPort(BYTE ucPort)
{

    if(ucPort != CAT_HDMI_PORTA)
    {
        ucPort = CAT_HDMI_PORTB ;
    }

    if( ucPort != ucCurrentHDMIPort )
    {
        ucCurrentHDMIPort = ucPort ;
    }

    HDMIRX_PRINTF(("SelectHDMIPort ucPort = %d, ucCurrentHDMIPort = %d\n",ucPort, ucCurrentHDMIPort));

}

BYTE
GetCurrentHDMIPort()
{
	return ucCurrentHDMIPort ;
}

void InitCAT6023()
//Dongzejun:2011/03/01
{
    BYTE uc ;

    HWReset_HDMIRX() ;		//Dongzejun:Ó²¼þ¸´Î»º¯Êý£¬¿ÉÒÔÌí¼Ó


    HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL0, 0) ;	//Dongzejun:to power on all modules

    #if 1
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_REGRST) ;	//Dongzejun:1->Reset the chip
    delay1ms(1) ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_HDCP_CTRL) ;

    #endif
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_SWRST|B_CDRRST) ;
    delay1ms(1) ;

	if (ucCurrentHDMIPort==CAT_HDMI_PORTA)
	{
		uc = B_PORT_SEL_A|B_PWD_AFEALL|B_PWDC_ETC ;
	}
	else
	{
		uc = B_PORT_SEL_B|B_PWD_AFEALL|B_PWDC_ETC ;
	}
	
	HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL1, uc);		//Dongzejun:Ñ¡ÔñHDMI¡¡A
    HDMIRX_PRINTF(("InitCAT6023(): reg07 = %02X, ucCurrentHDMIPort = %d\n", (int)HDMIRX_ReadI2C_Byte(07), (int)ucCurrentHDMIPort));

    SetIntMask1(0,B_PWR5VON|B_SCDTON|B_PWR5VOFF|B_SCDTOFF) ;		//Dongzejun:Reg0x16 Ê¹ÄÜÕâÐ©ÖÐ¶Ï
    SetIntMask2(0,B_NEW_AVI_PKG|B_PKT_SET_MUTE|B_PKT_CLR_MUTE) ;	//Dongzejun:Reg0x17
    SetIntMask3(0,B_ECCERR|B_R_AUTH_DONE|B_R_AUTH_START) ;				//Dongzejun:Reg0x18
    SetIntMask4(0,B_M_RXCKON_DET) ;

    SetDefaultRegisterValue() ;	//Dongzejun:Load the default value
    LoadCustomizeDefaultSetting() ;

    SetALLMute() ;		//Dongzejun:¾²Òô²Ù×÷

    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, 0) ;	//Dongzejun:Reg0x05 normal operation
    bDisableAutoAVMute = FALSE ;

		
    uc = HDMIRX_ReadI2C_Byte(REG_RX_DEVREV) ;
	//HDCP Enable/Disable   ¿ÉÒÔ½ûÖ¹ÁË¡£
    if( uc == 0xA2 )
    {
        HDMIRX_WriteI2C_Byte(REG_RX_HDCP_CTRL, 0x09) ;	//Dongzejun:Reg0x11 ²»½â????
        HDMIRX_WriteI2C_Byte(REG_RX_HDCP_CTRL, 0x19) ;
    }
    else
    {
    	uc = B_EXTROM | B_HDCP_ROMDISWR | B_HDCP_EN ;
        HDMIRX_WriteI2C_Byte(REG_RX_HDCP_CTRL, uc) ;
    }
    delay1ms(200) ;

    RDROM_Reset() ;

    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_SWRST) ; //sw reset
    delay1ms(1) ;
    SetALLMute() ;
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, 0) ;

    Terminator_Reset() ;

    SwitchVideoState(VSTATE_PwrOff) ;	//Dongzejun:³õÊ¼ÉèÖÃVState=VSTATE_PwrOff;

#ifdef SUPPORT_REPEATER
	RxHDCPRepeaterCapabilityClear(B_ENABLE_FEATURE_1P1|B_ENABLE_FAST);
    if( bHDCPMode & HDCP_REPEATER )
    {
	    RxHDCPRepeaterCapabilitySet(B_ENABLE_REPEATER);
	    RxHDCPRepeaterCapabilityClear(B_KSV_READY);
	}
	else
	{
	    RxHDCPRepeaterCapabilityClear(B_KSV_READY|B_ENABLE_REPEATER);
        SetIntMask3(~(B_R_AUTH_DONE|B_R_AUTH_START),B_ECCERR) ;
	}
#else
	HDMIRX_WriteI2C_Byte(REG_RX_CDEPTH_CTRL,HDMIRX_ReadI2C_Byte(REG_RX_CDEPTH_CTRL)&0xF);
#endif
    AcceptCDRReset = TRUE;

}

void PowerDownHDMI()
{
	HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL1, B_PWD_AFEALL|B_PWDC_ETC|B_PWDC_SRV|B_EN_AUTOPWD) ;
	HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL0, B_PWD_ALL) ;
}

BOOL IsCAT6023Interlace()
{
    if(HDMIRX_ReadI2C_Byte(REG_RX_VID_MODE)&B_INTERLACE)
    {
        return TRUE ;
    }
    return FALSE ;
}

WORD getCAT6023HorzTotal()
{
    BYTE uc[2] ;
	WORD hTotal ;

	uc[0] = HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_L) ;
	uc[1] = HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_H) ;
	hTotal = (WORD)(uc [1] & M_HTOTAL_H) ;
	hTotal <<= 8 ;
	hTotal |= (WORD)uc[0] ;

	return hTotal ;
}

WORD getCAT6023HorzActive()
{
    BYTE uc[3] ;

	WORD hTotal, hActive ;

	uc[0] = HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_L) ;
	uc[1] = HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_H) ;
	uc[2] = HDMIRX_ReadI2C_Byte(REG_RX_VID_HACT_L) ;

	hTotal = (WORD)(uc [1] & M_HTOTAL_H) ;
	hTotal <<= 8 ;
	hTotal |= (WORD)uc[0] ;

	hActive = (WORD)(uc[1] >> O_HACT_H)& M_HACT_H ;
	hActive <<= 8 ;
	hActive |= (WORD)uc[2] ;

	if( (hActive | (1<<11)) < hTotal )
	{
		hActive |= 1<<11 ;
	}

	return hActive ;

}

WORD getCAT6023HorzFrontPorch()
{
    BYTE uc[2] ;
	WORD hFrontPorch ;

	uc[0] = HDMIRX_ReadI2C_Byte(REG_RX_VID_H_FT_PORCH_L) ;
	uc[1] = (HDMIRX_ReadI2C_Byte(REG_RX_VID_HSYNC_WID_H) >> O_H_FT_PORCH ) & M_H_FT_PORCH ;
	hFrontPorch = (WORD)uc[1] ;
	hFrontPorch <<= 8 ;
	hFrontPorch |= (WORD)uc[0] ;

	return hFrontPorch ;
}

WORD getCAT6023HorzSyncWidth()
{
    BYTE uc[2] ;
	WORD hSyncWidth ;

	uc[0] = HDMIRX_ReadI2C_Byte(REG_RX_VID_HSYNC_WID_L) ;
	uc[1] = HDMIRX_ReadI2C_Byte(REG_RX_VID_HSYNC_WID_H)  & M_HSYNC_WID_H ;

	hSyncWidth = (WORD)uc[1] ;
	hSyncWidth <<= 8 ;
	hSyncWidth |= (WORD)uc[0] ;

	return hSyncWidth ;
}

WORD getCAT6023HorzBackPorch()
{
	WORD hBackPorch ;

	hBackPorch = getCAT6023HorzTotal() - getCAT6023HorzActive() - getCAT6023HorzFrontPorch() - getCAT6023HorzSyncWidth() ;

	return hBackPorch ;
}

WORD getCAT6023VertTotal()
{
    BYTE uc[3] ;
	WORD vTotal, vActive ;
	uc[0] = HDMIRX_ReadI2C_Byte(REG_RX_VID_VTOTAL_L) ;
	uc[1] = HDMIRX_ReadI2C_Byte(REG_RX_VID_VTOTAL_H) ;
	uc[2] = HDMIRX_ReadI2C_Byte(REG_RX_VID_VACT_L) ;

	vTotal = (WORD)uc[1] & M_VTOTAL_H ;
	vTotal <<= 8 ;
	vTotal |= (WORD)uc[0] ;

	vActive = (WORD)(uc[1] >> O_VACT_H ) & M_VACT_H ;
	vActive |= (WORD)uc[2] ;

	if( vTotal > (vActive | (1<<10)))
	{
		vActive |= 1<<10 ;
	}

	#if 0
	if( vActive == 600 && vTotal == 634 )
	{
		vTotal = 666 ;
	}
	#endif

	return vTotal ;
}

WORD getCAT6023VertActive()
{
    BYTE uc[3] ;
	WORD vTotal, vActive ;
	uc[0] = HDMIRX_ReadI2C_Byte(REG_RX_VID_VTOTAL_L) ;
	uc[1] = HDMIRX_ReadI2C_Byte(REG_RX_VID_VTOTAL_H) ;
	uc[2] = HDMIRX_ReadI2C_Byte(REG_RX_VID_VACT_L) ;

	vTotal = (WORD)uc[1] & M_VTOTAL_H ;
	vTotal <<= 8 ;
	vTotal |= (WORD)uc[0] ;

	vActive = (WORD)(uc[1] >> O_VACT_H ) & M_VACT_H ;
	vActive <<= 8 ;
	vActive |= (WORD)uc[2] ;

	if( vTotal > (vActive | (1<<10)))
	{
		vActive |= 1<<10 ;
	}

	return vActive ;
}

WORD getCAT6023VertFrontPorch()
{
    WORD vFrontPorch ;

	vFrontPorch = (WORD)HDMIRX_ReadI2C_Byte(REG_RX_VID_V_FT_PORCH) & 0xF ;

	if( getCAT6023VertActive() == 600 && getCAT6023VertTotal() == 666 )
	{
		vFrontPorch |= 0x20 ;
	}

	return vFrontPorch ;

}

WORD getCAT6023VertSyncToDE()
{
    WORD vSync2DE ;

    vSync2DE = (WORD)HDMIRX_ReadI2C_Byte(REG_RX_VID_VSYNC2DE) ;
    return vSync2DE ;
}

WORD getCAT6023VertSyncWidth()
{
    WORD vSync2DE ;
    WORD vTotal, vActive, hActive  ;

    vSync2DE = getCAT6023VertSyncToDE() ;
    vTotal = getCAT6023VertTotal() ;
    vActive = getCAT6023VertActive() ;
    hActive = getCAT6023HorzActive() ;
#ifndef CAT6023_A1

    if( vActive < 300 )
    {
    	return 3 ;
    }

    if( hActive == 640 && hActive == 480 )
    {
    	if( HDMIRX_ReadI2C_Byte(REG_RX_VID_XTALCNT_128PEL) < 0x80 )
    	{
    		return 3 ;
    	}

    	return 2;
    }

    return 5 ;
#endif
}

WORD getCAT6023VertSyncBackPorch()
{
    WORD vBackPorch ;

    vBackPorch = getCAT6023VertSyncToDE() - getCAT6023VertSyncWidth() ;
    return vBackPorch ;
}

BYTE getCAT6023xCnt()
{
    return HDMIRX_ReadI2C_Byte(REG_RX_VID_XTALCNT_128PEL) ;
}

BOOL getCAT6023AudioInfo(BYTE *pbAudioSampleFreq, BYTE *pbValidCh)
{
    if(IsCAT6023HDMIMode())
    {
        if( pbAudioSampleFreq )
        {
            *pbAudioSampleFreq = HDMIRX_ReadI2C_Byte(REG_RX_FS) & M_Fs ;
        }

        if( pbValidCh )
        {
            *pbValidCh = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT) ;
            if( *pbValidCh & B_AUDIO_LAYOUT )
            {
                *pbValidCh &= M_AUDIO_CH ;
            }
            else
            {
                *pbValidCh = B_AUDIO_SRC_VALID_0 ;
            }
        }
        return TRUE ;
    }
    else
    {
        return FALSE ;
    }
}

#ifdef GET_PACKAGE

BOOL
GetAVIInfoFrame(BYTE *pData)
{

    if( pData == NULL )
    {
        return ER_FAIL ;
    }

    pData[0] = AVI_INFOFRAME_TYPE ;
    pData[1] = HDMIRX_ReadI2C_Byte(REG_RX_AVI_VER) ;
    pData[2] = AVI_INFOFRAME_LEN ;

    HDMIRX_ReadI2C_ByteN(REG_RX_AVI_DB1, pData+3,AVI_INFOFRAME_LEN) ;

    return ER_SUCCESS ;
}

BOOL
GetAudioInfoFrame(BYTE *pData)
{

    if( pData == NULL )
    {
        return FALSE ;
    }

    pData[0] = AUDIO_INFOFRAME_TYPE ;
    pData[1] = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_VER) ;
    pData[2] = AUDIO_INFOFRAME_LEN ;

    HDMIRX_ReadI2C_ByteN(REG_RX_AUDIO_DB1, pData+3,AUDIO_INFOFRAME_LEN) ;

    return TRUE ;
}

BOOL
GetMPEGInfoFrame(BYTE *pData)
{

    if( pData == NULL )
    {
        return FALSE ;
    }

    pData[0] = MPEG_INFOFRAME_TYPE ;
    pData[1] = HDMIRX_ReadI2C_Byte(REG_RX_MPEG_VER) ;
    pData[2] = MPEG_INFOFRAME_LEN ;

    HDMIRX_ReadI2C_ByteN(REG_RX_MPEG_DB1, pData+3,MPEG_INFOFRAME_LEN) ;

    return TRUE ;
}

BOOL
GetVENDORSPECInfoFrame(BYTE *pData)
{

    if( pData == NULL )
    {
        return FALSE ;
    }

    pData[0] = VENDORSPEC_INFOFRAME_TYPE ;
    pData[1] = HDMIRX_ReadI2C_Byte(REG_RX_VS_VER) ;
    pData[2] = VENDORSPEC_INFOFRAME_LEN ;

    HDMIRX_ReadI2C_ByteN(REG_RX_VS_DB1, pData+3,VENDORSPEC_INFOFRAME_LEN) ;

    return TRUE ;
}

BOOL
GetACPPacket(BYTE *pData)
{

    if( pData == NULL )
    {
        return FALSE ;
    }

    HDMIRX_ReadI2C_ByteN(REG_RX_ACP_TYPE, pData,ACP_PKT_LEN) ;

    return TRUE ;
}

#endif

void
getCAT6023Regs(BYTE *pData)
{
    int i, j ;

    SwitchHDMIRXBank(0) ;
    for( i = j = 0 ; i < 256 ; i++,j++ )
    {
        pData[j] = HDMIRX_ReadI2C_Byte((BYTE)(i&0xFF)) ;
    }
    SwitchHDMIRXBank(1) ;
    for( i = 0xA0 ; i <= 0xF2 ; i++, j++ )
    {
        pData[j] = HDMIRX_ReadI2C_Byte((BYTE)(i&0xFF)) ;
    }
    SwitchHDMIRXBank(0) ;
}

BYTE
getCAT6023OutputColorMode()
{
    return bOutputVideoMode & F_MODE_CLRMOD_MASK ;
}

BYTE
getCAT6023OutputColorDepth()
{
    BYTE uc ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_FS) & M_GCP_CD ;
    return uc >> O_GCP_CD ;
}

static void HWReset_HDMIRX()
//Dongzejun:Ó²¼þ¸´Î»º¯Êý£¬¿ÉÒÔÌí¼Ó
{

#ifdef _MCU_

#endif
}

static void Terminator_Off()
//Dongzejun:  Reg0x07ÉèÖÃ=0x0c
//Reg0x07:Bit2=1  bit3=1

{
    BYTE uc ;
	//Dongzejun:Reg0x07=0x0c turn off the terminator
    uc = HDMIRX_ReadI2C_Byte(REG_RX_PWD_CTRL1) | (B_PWD_AFEALL|B_PWDC_ETC);  
    HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL1, uc ) ;
    HDMIRX_PRINTF(("Terminator_Off, reg07 = %02x\n",(int)uc));
}

static void Terminator_On()
//Dongzejun:Reg0x07=0x00  turn on the terminator
//Reg0x07:Bit2=0  bit3=0
{
    BYTE uc ;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_PWD_CTRL1) & ~(B_PWD_AFEALL|B_PWDC_ETC);
    HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL1, uc) ;
    HDMIRX_PRINTF(("Terminator_On, reg07 = %02x\n",(int)uc));
}

static void Terminator_Reset()
//Dongzejun:¿ÉÒÔ²ÎÕÕIT6603µÄ×ö·¨
{
    Terminator_Off() ;
    delay1ms(500) ;		//Dongzejun:´Ë²¿·Ö³ÌÐòÑÓÊ±Ì«³¤£¬Òª¿¼ÂÇÈí¼þÑÓÊ±£¬»òÕß³õÊ¼»¯·ÖÁ½²¿·ÖÖ´ÐÐ
    Terminator_On() ;
}

void RDROM_Reset()
//Dongzejun:HDCP Rom reset¶ÔReg0x9B²Ù×÷¡£
{
    BYTE i ;
    BYTE uc ;

    HDMIRX_PRINTF(("RDROM_Reset()\n"));

    uc = HDMIRX_ReadI2C_Byte(REG_RX_RDROM_CLKCTRL) & ~(B_ROM_CLK_SEL_REG|B_ROM_CLK_VALUE) ;
    for(i=0 ;i < 16 ; i++ )
    {
        HDMIRX_WriteI2C_Byte(REG_RX_RDROM_CLKCTRL, B_ROM_CLK_SEL_REG|uc) ;
        HDMIRX_WriteI2C_Byte(REG_RX_RDROM_CLKCTRL, B_ROM_CLK_SEL_REG|B_ROM_CLK_VALUE|uc) ;
    }

    HDMIRX_WriteI2C_Byte(REG_RX_RDROM_CLKCTRL,uc) ;

}

void Check_RDROM()
//Dongzejun:¶ÔROM¡¡HDCP¸´Î»
{
    BYTE uc ;
    HDMIRX_PRINTF(("Check_HDCP_RDROM()\n"));

    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, 0) ;

    if( IsSCDT() )
    {
        int count ;

        {
            uc = HDMIRX_ReadI2C_Byte(REG_RX_RDROM_STATUS) ;	//Dognzejun:Reg0x92
            if( (uc & 0xF) != 0x9 )		//Dongzejun:ROM¡¡¿ÕÏÐÊ±¾Í²»ÓÃ¸´Î»ÁË
            {
                RDROM_Reset() ;
            }
            HDMIRX_PRINTF(("Check_HDCP_RDROM() done.\n"));
            return ;
        }
    }
}

static void SWReset_HDMIRX()
//Dongzejun:ÇÐ»»Reset×´Ì¬VSTATE_SWReset
{
    Check_RDROM() ;
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_SWRST) ;
    delay1ms(1) ;
    SetALLMute() ;
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, 0) ;

    Terminator_Off() ;
    SwitchVideoState(VSTATE_SWReset) ;		//

    SCDTErrorCnt = 0;

}

typedef struct _REGPAIR {
    BYTE ucAddr ;
    BYTE ucValue ;
} REGPAIR ;

//Dongzejun:ÉèÖÃÊÓÆµ¡¢ÒôÆµ¸ñÊ½
static REGPAIR _CODE acCustomizeValue[] =
{

    {REG_RX_VCLK_CTRL, 0x30},	//Reg0x1d

    {REG_RX_I2S_CTRL,0x61},			//Reg0x75 I2S¿ØÖÆÆ÷ ÎÒÃÇÉèÖÃÖµÎª0x60

    {REG_RX_PG_CTRL2,HDMIRX_OUTPUT_COLORMODE},	//Reg0x3d:Êä³öÉ«²Ê¿Õ¼ä¶¨Òå
    {REG_RX_VIDEO_MAP,HDMIRX_OUTPUT_MAPPING},			//Dongzejun:Reg0x1B
    {REG_RX_VIDEO_CTRL1,HDMIRX_OUTPUT_TYPE},			//Dongzejun:Reg0x1C
    {REG_RX_MCLK_CTRL, 0xC1},							//Dongzejun:Reg0x78
    {0xFF,0xFF}
} ;

static void LoadCustomizeDefaultSetting()
//Dongzejun:ÓÃ»§ÉèÖÃµÄÒ»Ð©±äÁ¿
{
    BYTE i, uc ;
    for( i = 0 ; acCustomizeValue[i].ucAddr != 0xFF ; i++ )
    {
        HDMIRX_WriteI2C_Byte(acCustomizeValue[i].ucAddr,acCustomizeValue[i].ucValue) ;
    }

    /*
    uc = HDMIRX_ReadI2C_Byte(REG_RX_PG_CTRL2) & ~(M_OUTPUT_COLOR_MASK<<O_OUTPUT_COLOR_MODE);
    switch(bOutputVideoMode&F_MODE_CLRMOD_MASK)
    {
    case F_MODE_YUV444:
        uc |= B_OUTPUT_YUV444 << O_OUTPUT_COLOR_MODE ;
        break ;
    case F_MODE_YUV422:
        uc |= B_OUTPUT_YUV422 << O_OUTPUT_COLOR_MODE ;
        break ;
    }
    HDMIRX_WriteI2C_Byte(REG_RX_PG_CTRL2, uc) ;
    */
    bOutputVideoMode&=~F_MODE_CLRMOD_MASK;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_PG_CTRL2) & (M_OUTPUT_COLOR_MASK<<O_OUTPUT_COLOR_MODE);

    switch(uc)
    {
    case (B_OUTPUT_YUV444 << O_OUTPUT_COLOR_MODE): 
			bOutputVideoMode|=F_MODE_YUV444; 
			break ;
    case (B_OUTPUT_YUV422 << O_OUTPUT_COLOR_MODE): 
			bOutputVideoMode|=F_MODE_YUV422; 
			break ;
    case 0: 
			bOutputVideoMode|=F_MODE_RGB444; 
			break ;
    default: 
			bOutputVideoMode|=F_MODE_RGB444; 
			break ;
    }
    bIntPOL = (HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_CTRL) & B_INTPOL)?LO_ACTIVE:HI_ACTIVE ;	//Dongzejun:ÖÐ¶Ï¼«ÐÔÉèÖÃ
}

static REGPAIR _CODE acDefaultValue[] =
{
    {0x0F,0x00},
    {REG_RX_VIO_CTRL,0xAE},		//Reg0x08

    {REG_RX_DESKEW_CTRL, 0x40},		//Reg0x3B

    {REG_RX_PLL_CTRL,0x03},		//Reg0x68

	{REG_RX_EQUAL_CTRL1,0x11},	//Reg0x6b

    {REG_RX_EQUAL_CTRL2, 0x00},	//Reg0x6c

    {0x93,0x43},
    {0x94,0x4F},
    {0x95,0x87},
    {0x96,0x33},

    {0xFF,0xFF}

} ;

void SetDefaultRegisterValue()
//Dongzejun:
{
    BYTE i ;

    for( i = 0 ; acDefaultValue[i].ucAddr != 0xFF ; i++ )
    {
        HDMIRX_WriteI2C_Byte(acDefaultValue[i].ucAddr, acDefaultValue[i].ucValue ) ;
    }

}

static void ClearIntFlags(BYTE flag)
//Dongzejun:Çå³ýÖÐ¶Ï Ð´Reg0x19 1->0
{
    BYTE uc ;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_CTRL) ;
    uc &= FLAG_CLEAR_INT_MASK ;
    uc |= flag ;
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL,uc) ;  //Dongzejun: Ð´1
    delay1ms(1);
    uc &= FLAG_CLEAR_INT_MASK ;
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL,uc) ;	//Dongzejun:Ð´0
    delay1ms(1);

}

static void ClearHDCPIntFlags()
//Dongzejun:Reg0x18		1->0 HDCPÈÏÖ¤ÖÐ¶ÏÇå³ý
{
    BYTE uc ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_CTRL1) ;
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL1, (BYTE)B_CLR_HDCP_INT|uc ) ;
    delay1ms(1);
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL1, uc&((BYTE)~B_CLR_HDCP_INT) ) ;
}

BOOL IsSCDT()
//Dongzejun: SCDTÒÑ¼ì²âµ½·µ»Ø1
//Ö÷ÒªÅÐ¶ÏReg0x10[2/1]SCDT/VCLK
{
    BYTE uc ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE) & (B_SCDT|B_VCLK_DET/*|B_PWR5V_DET*/) ;
    return (uc==(B_SCDT|B_VCLK_DET/*|B_PWR5V_DET*/))?TRUE:FALSE ;
}


BOOL CheckPlg5VPwr()
//Dongzejun:Èç¹ûHDMI¡¡PORTAÓÐ5VµçÔ´£¬Ôò·µ»Ø1
//ÅÐ¶ÏReg0x10[0]=1,PORTAÓÐµçÔ´
{
    BYTE uc ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE) ;

    if( ucCurrentHDMIPort == CAT_HDMI_PORTB )
    {
        return (uc&B_PWR5V_DET_PORTB)?TRUE:FALSE ;

    }

    return (uc&B_PWR5V_DET_PORTA)?TRUE:FALSE ;
}


void SetMUTE(BYTE AndMask, BYTE OrMask)
//Dongzejun:Reg0x89  P12
{
    BYTE uc ;

    if( AndMask )
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL) ;

    }
    uc &= AndMask ;
    uc |= OrMask ;
    #ifdef HDMI_REPEATER
    #pragma message("HDMI Repeating TTL to next stage, do not gatting the video sync.")
    uc &= 0x1F ;
    uc |= 0x80 ;
    #endif
    HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL,uc) ;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL) ;

}

/*///////////////////////////////////////////////////////////////
//¶ÔÕâ¸öº¯ÊýµÄÒ»Ð©Àí½â¡£
1¡¢µ±AndMask£½0Ê±£¬Ê¹ÓÃOrMask±äÁ¿¶ÔÏàÓ¦µÄ¼Ä´æÆ÷Î»½øÐÐ£¬ÖÃÎ»²Ù×÷¡£

2¡¢µ±AndMask!=0Ê±£¬OrMask¡¡Ö¸¶¨Î»Çå0»òÖÃÎ»¡£
//
///////////////////////////////////////////////////////////////////////*/
void SetIntMask1(BYTE AndMask,BYTE OrMask)
//Dongzejun:Reg0x16
{
    BYTE uc ;
    if( AndMask != 0 )
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_MASK1) ;
    }
    uc &= AndMask ;
    uc |= OrMask ;
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_MASK1, uc) ;
}

void SetIntMask2(BYTE AndMask,BYTE OrMask)
//Dongzejun:Reg0x17
{
    BYTE uc ;
    if( AndMask != 0 )
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_MASK2) ;
    }
    uc &= AndMask ;
    uc |= OrMask ;
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_MASK2, uc) ;
}

void SetIntMask3(BYTE AndMask,BYTE OrMask)
//Dongzejun:Reg0x18
{
    BYTE uc ;
    if( AndMask != 0 )
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_MASK3) ;
    }
    uc &= AndMask ;
    uc |= OrMask ;
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_MASK3, uc) ;
}

void SetIntMask4(BYTE AndMask,BYTE OrMask)
//Dongzejun:Reg0x8c
{
    BYTE uc ;
    if( AndMask != 0 )
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_MASK4) ;
    }
    uc &= AndMask ;
    uc |= OrMask ;
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_MASK4, uc) ;
}

#if 0
void
SetGeneralPktType(BYTE type)
{
    HDMIRX_WriteI2C_Byte(REG_RX_PKT_REC_TYPE,type) ;
}
#endif

BOOL IsCAT6023HDMIMode()
//Dongzejun:·µ»Ø1£­HDMI;0£­DVI
{
    BYTE uc ;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE) ;

    return (uc&B_HDMIRX_MODE)?TRUE:FALSE ;
}

void Interrupt_Handler()
//Dongzejun:ÖÐ¶Ï´¦Àí¡£
{
	BYTE int1data = 0 ;	//Dongzejun:Reg0x13 ÖÐ¶Ï1
	BYTE int2data = 0 ;	//Dongzejun:Reg0x14 Interrupt2
	BYTE int3data = 0 ;	//Reg0x15
	BYTE int4data = 0 ;	//Dongzejun:Reg0x8B ÖÐ¶Ï4
	BYTE sys_state ;		//Dongzejun:Reg0x10 ÏµÍ³×´Ì¬
	BYTE flag = FLAG_CLEAR_INT_ALL;

    if( VState == VSTATE_SWReset )
    {
    	return ;
    }

    sys_state = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE) ;	//Dongzejun:ÏµÍ³×´Ì¬

//==========================================================================
//Dongzejun:Interrupt 1/Interrupt 4

    int4data = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT4) ;
    int1data = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT1) ;
    if( int1data || (int4data&B_RXCKON_DET) )
    {
        HDMIRX_PRINTF3(("system state = %02X\n",(int)sys_state));
        HDMIRX_PRINTF3(("Interrupt 1 = %02X\n",(int)int1data));
        HDMIRX_PRINTF3(("Interrupt 4 = %02X\n",(int)int4data));
        ClearIntFlags(B_CLR_MODE_INT) ;	//Dongzejun:Çå³ýÖÐ¶Ï1

		if(!CheckPlg5VPwr())	//Dongzejun:HDMP¡¡PROTAÃ»ÓÐµçÔ´
		{
			if( VState != VSTATE_SWReset && VState != VSTATE_PwrOff )
			{
				SWReset_HDMIRX() ;		//Dongzejun:Ö»Òª5VµçÔ´Ò»¶ªÊ§£¬ÔòVState£½VSTATE_SWReset
				return ;
			}
		}

        if( int1data & B_PWR5VOFF )	//PORTA:¶Ë¿Ú5VµçÔ´¹Ø±Õ
        {
            HDMIRX_PRINTF(("5V Power Off interrupt\n"));
            RXINT_5V_PwrOff() ;	//Dongzejun:
        }

        if( VState == VSTATE_SWReset )  	//Dongzejun:PORTAµôµç½«±ä³É´Ë×´Ì¬
        {
        	return ;
        }

        if( int1data & B_SCDTOFF )		//Dongzejun:Î´¼ì²âµ½SCDT¡¡£¬Èç¹û³¬¹ý¼ÆÊ±»¹ÊÇSCDT¡¡OFF£¬½«ÒªÈ¥¸´Î»²Ù×÷
        {
            HDMIRX_PRINTF(("SCDT Off interrupt\n"));
            RXINT_SCDT_Off() ;	// ==>VSTATE_SyncWait
        }

        if( int1data & B_PWR5VON )	//Dongzejun:ÓÐ5VµçÑ¹
        {
            HDMIRX_PRINTF(("5V Power On interrupt\n"));
            RXINT_5V_PwrOn() ;
        }

        if( int1data & B_VIDMODE_CHG )	//Dongzejun:Video Mode¸ü¸Ä
        {
            HDMIRX_PRINTF(("Video mode change interrupt.\n:"));
            RXINT_VideoMode_Chg() ;
            if( VState == VSTATE_SWReset )
            {
            	return ;
            }
        }

        if( int1data & B_HDMIMODE_CHG )	// 1:HDMI¡¢DVIÄ£Ê½ÇÐ»»
        {
            HDMIRX_PRINTF(("HDMI Mode change interrupt.\n"));
            RXINT_HDMIMode_Chg() ;
        }

        if( int1data & B_SCDTON )		//Dongzejun:SCDTÓÐÐ§
        {
            HDMIRX_PRINTF(("SCDT On interrupt\n"));
            RXINT_SCDT_On() ;
        }

        if( int4data & B_RXCKON_DET )	//Dongzejun:Reg0x8b
        {
            HDMIRX_PRINTF(("RXCKON DET interrupt\n"));
            RXINT_RXCKON() ;
        }
    }

//==========================================================================
//Dongzejun:Interrupt 2
    int2data = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT2) ;
    if( int2data )
    {
        BYTE vid_stat = HDMIRX_ReadI2C_Byte(REG_RX_VID_INPUT_ST) ;
        HDMIRX_PRINTF2(("Interrupt 2 = %02X\n",(int)int2data));
        ClearIntFlags(B_CLR_PKT_INT|B_CLR_MUTECLR_INT|B_CLR_MUTESET_INT) ;

        if( int2data & B_PKT_SET_MUTE )	//Dongzejun:AV Mute
        {
            HDMIRX_PRINTF(("AVMute set interrupt.\n" ));
            RXINT_AVMute_Set() ;
        }

        if( int2data & B_NEW_AVI_PKG )	//Dongzejun:ÐÂµÄAVI°üÃ÷Ìì´Ó´Ë¿ªÊ¼2011/03/02
        {
            HDMIRX_PRINTF(("New AVI Info Frame Change interrupt\n"));
            RXINT_SetNewAVIInfo() ;
        }

        if( ( int2data & B_PKT_CLR_MUTE ))
        {
            HDMIRX_PRINTF(("AVMute clear interrupt.\n" ));
            RXINT_AVMute_Clear() ;
        }
    }

	//Dongzejun:Reg0x15
    int3data = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT3) ;
    if( int3data &(B_R_AUTH_DONE|B_R_AUTH_START))
    {
        ClearHDCPIntFlags() ;
    #ifdef SUPPORT_REPEATER
        if( bHDCPMode & HDCP_REPEATER )
        {
	        if( int3data & B_R_AUTH_START )
	        {
	            HDMIRX_PRINTF((" B_R_AUTH_START\n"));
	            SwitchRxHDCPState(RXHDCP_AuthStart) ;
	        }
	        if( int3data & B_R_AUTH_DONE )
	        {
	            HDMIRX_PRINTF(("B_R_AUTH_DONE \n"));
	            SwitchRxHDCPState(RXHDCP_AuthDone) ;
	        }
        }
    #endif
    }

	if( VState == VSTATE_VideoOn || VState == VSTATE_HDCP_Reset)
	{

	    if( int3data &(B_ECCERR|B_AUDFIFOERR|B_AUTOAUDMUTE) )
	    {
	        ClearIntFlags(B_CLR_AUDIO_INT|B_CLR_ECC_INT) ;
	        if( AState != ASTATE_AudioOff)
	        {
		        HDMIRX_PRINTF(("Interrupt 3 = %02X\n",(int)int3data));
		        if( int3data & (B_AUTOAUDMUTE|B_AUDFIFOERR))
		        {
		            HDMIRX_PRINTF(("Audio Error interupt\n"));
		            RXINT_ResetAudio() ;
		            SetIntMask3(~(B_AUTOAUDMUTE|B_AUDFIFOERR),0) ;
		        }
	        }

	        if( int3data & B_ECCERR )
	        {
	            HDMIRX_PRINTF(("ECC error interrupt\n"));
	            RXINT_ResetHDCP() ;
	        }
	    }
	}

    #ifdef DEBUG
    if( int1data | int2data )
    {
        int1data = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT1);
        int2data = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT2);
        int3data = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT3);
        sys_state = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE);
        HDMIRX_PRINTF2(("%02X %02X %02X %02X\n",
            (int)int1data,
            (int)int2data,
            (int)int3data,
            (int)sys_state)) ;
    }
    #endif

}

void RXINT_5V_PwrOn()
//Dongzejun:Reg0x13[0] =1¡¡5VµçÔ´Õý³£
{

    if( VState == VSTATE_PwrOff )	//Dongzejun:VSTATE_PwrOff==>VSTATE_SyncWait
    {

        if( CheckPlg5VPwr() )
        {
            SwitchVideoState(VSTATE_SyncWait) ;
        }
    }
}

void RXINT_5V_PwrOff()
//Dongzejun:
{
    BYTE sys_state ;

    sys_state = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE) ;

    SWReset_HDMIRX() ;
}

void RXINT_SCDT_On()
//Dongzejun:VSTATE_SyncWait==>VSTATE_SyncChecking
{
    if( VState == VSTATE_SyncWait )
    {
        if(IsSCDT())
        {
            SwitchVideoState(VSTATE_SyncChecking) ; // 
        }
    }
}

static void CDR_Reset()
//Dongzejun:¼ì²âµ½VCLKÊ±ÖÓÓÐÐ§£¬½«¶ÔÆä¸´Î»²Ù×÷¡£
{

    BYTE uc;
	HDMIRX_PRINTF(("CDR_Reset()\n"));

//Dongzejun:¹Ø±ÕÖÐ¶Ï
    SetIntMask4(0,0) ;
    SetIntMask1(0,0) ;
		
	// 1. Reg97[5] = '1'		Dongzejun:
    uc = HDMIRX_ReadI2C_Byte(0x97) ;
    HDMIRX_WriteI2C_Byte(0x97,uc|0x20);


// 2. Reg05[7][1] = '1' '1'			Dongzejun
    if( EnaSWCDRRest )		//´ËEnaSWCDRRest£½0
    {
        HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL,B_CDRRST| B_SWRST  ) ;
    }
    else
    {
        HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_CDRRST|B_VDORST  ) ;
    }

// 3. Reg73[3]  = '1'			Dongzejun
    uc = HDMIRX_ReadI2C_Byte(REG_RX_CDEPTH_CTRL) ;		//Dongzejun:Reg0x73
    HDMIRX_WriteI2C_Byte(REG_RX_CDEPTH_CTRL,uc |B_RSTCD);

// 4. Reg97[5] = '0'				Dongzejun
    uc = HDMIRX_ReadI2C_Byte(0x97) ;
    HDMIRX_WriteI2C_Byte(0x97,uc&(~0x20));

// 5. REg05[7][1] = '0''0'	Dongzejun
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, 0 ) ;

// 6. Reg73[3] = '0'				Dongzejun
    uc = HDMIRX_ReadI2C_Byte(REG_RX_CDEPTH_CTRL) ;
    HDMIRX_WriteI2C_Byte(REG_RX_CDEPTH_CTRL,uc &(~B_RSTCD));

#ifdef SUPPORT_REPEATER
    RxHDCPRepeaterCapabilityClear(B_KSV_READY);
#endif

//Dongzejun:¿ªÆôÖÐ¶Ï
    SetIntMask4(0,B_M_RXCKON_DET) ;
    SetIntMask1(0,B_PWR5VON|B_SCDTON|B_PWR5VOFF) ;
    ClearIntFlags(B_CLR_MODE_INT) ;

    AcceptCDRReset = FALSE ;
	ucDVISCDToffCNT=0;
}

void RXINT_SCDT_Off()
//Dongzejun:Reg0X13[3]Î´¼ì²âµ½SCDT
//VStateÇÐ»»µ½"VSTATE_SyncWait"
{

    if( VState != VSTATE_PwrOff )
    {
        HDMIRX_PRINTF(("GetSCDT OFF\n"));
        SwitchVideoState(VSTATE_SyncWait) ;	//Dongzejun:µÈ´ýÍ¬²½

        SCDTErrorCnt++;
    }
}

void RXINT_VideoMode_Chg()
//Dongzejun:Reg0x13[5]=1 VideoÄ£Ê½¸ü¸Ä
{
    BYTE sys_state ;

    HDMIRX_PRINTF(("RXINT_VideoMode_Chg\n"));

    sys_state = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE) ;

    if(CheckPlg5VPwr())		//Dongzejun:5VÓÐµçÔ´  ==>VSTATE_SyncWait
    {
        SwitchVideoState(VSTATE_SyncWait) ;
    }
    else				//Dongzejun:ÎÞ5VµçÔ´==>VSTATE_SWReset
    {
        SWReset_HDMIRX() ;
    }
}

void RXINT_HDMIMode_Chg()
//Dongzejun:HDMI/DVIÏà»¥ÇÐ»»£¬Èç¹ûVState == VSTATE_VideoOn£¬ÔòÒª×öÈçÏÂ´¦Àí.
{
    if(VState == VSTATE_VideoOn )
    {
        if( IsCAT6023HDMIMode() )		//Dongzejun:HDMIÄ£Ê½
        {
            HDMIRX_PRINTF(("HDMI Mode.\n"));
            SwitchAudioState(ASTATE_RequestAudio) ;

        }
        else
        {
            HDMIRX_PRINTF(("DVI Mode.\n"));
            SwitchAudioState(ASTATE_AudioOff) ;
            NewAVIInfoFrameF = FALSE ;

            SetDVIVideoOutput() ;

        }
    }
}

void RXINT_RXCKON()
//Dongzejun:CLK¡¡ON×´Ì¬
{
    if( AcceptCDRReset == TRUE )
    {
        if((HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE)&(B_VCLK_DET|B_RXCK_VALID)) == (B_VCLK_DET|B_RXCK_VALID))  //Dongzejun:CLKÓÐÐ§
        {
            CDR_Reset() ;
        }
    }
}

void RXINT_AVMute_Set()
//Dongzejun:Reg0x14 bit0 PktSetMuteÉèÖÃ¾²Òô¡£
{
    BYTE uc ;
    MuteByPKG = ON ;

    SetAudioMute(ON) ;
    SetVideoMute(ON) ;
    StartAutoMuteOffTimer() ;
    SetIntMask2(~(B_PKT_CLR_MUTE),(B_PKT_CLR_MUTE)) ;

    bDisableAutoAVMute = 0 ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL) ;
    uc &= ~B_VDO_MUTE_DISABLE ;
    HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc) ;
}

void RXINT_AVMute_Clear()
//Dongzejun:2011/03/03
{
    BYTE uc ;
    MuteByPKG = OFF ;
    bDisableAutoAVMute = 0 ;
    uc =  HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL) ;  //Dongzejun:ºÍº¯ÊýRXINT_AVMute_Set()ÉèÖÃÒ»Ö±
    uc &= ~B_VDO_MUTE_DISABLE ;
    HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc) ;

    EndAutoMuteOffTimer() ;		//Dongzejun:Reg0x14[7]=1Ê±£¬ÉèÖÃ

    if(VState == VSTATE_VideoOn )
    {
        SetVideoMute(OFF) ;

    }

    if(AState == ASTATE_AudioOn )
    {
        SetHWMuteClr() ;
        ClearHWMuteClr() ;

        SetAudioMute(OFF) ;
    }
    SetIntMask2(~(B_PKT_CLR_MUTE),0) ;
}

void RXINT_SetNewAVIInfo()
//Dongzejun:ÐÂµÄAVI°ü
{
    NewAVIInfoFrameF = TRUE ;

    if( VState == VSTATE_VideoOn )
    {
        SetNewInfoVideoOutput() ;
    }

    prevAVIDB1 = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB1) ;
    prevAVIDB2 = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB2) ;

}

void RXINT_ResetAudio()
//Dongzejun:ÒôÆµ¸´Î»²Ù×÷

{

    if(AState != ASTATE_AudioOff)
    {
        SetAudioMute(ON) ;
        SwitchAudioState(ASTATE_RequestAudio) ; //Dongzejun:ÇëÇóÒôÆµ´ò¿ª
    }
}

void RXINT_ResetHDCP()
//Dongzejun:
{
    BYTE uc ;

    if( VState == VSTATE_VideoOn )
    {
        ClearIntFlags(B_CLR_ECC_INT) ;
        delay1ms(1) ;
        uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT3) ;

        if( uc & B_ECCERR )
        {

    		SwitchVideoState(VSTATE_HDCP_Reset) ;

        }

    }
}

void Timer_Handler()
//Dongzejun:
{

	Interrupt_Handler() ;
    VideoTimerHandler() ;
    MuteProcessTimerHandler() ;
    AudioTimerHandler() ;
#ifdef SUPPORT_REPEATER
    RxHDCP_Handler() ;
#endif
}

static void VideoTimerHandler()
//Dongzejun:¶¨Ê±Æ÷VIDEO´¦Àí
{
	UCHAR uc ;

//Dongzejun µÈ´ýVSTATE_SyncWaitÊÇ·ñ³¬Ê±
	if (SCDTErrorCnt>= SCDT_LOST_TIMEOUT)	
    {
		SWReset_HDMIRX() ;
	}

	//===========================================1-VSTATE_SWReset=======================================================
	//Dongzejun:VSTATE_SWReset
	if( VState == VSTATE_SWReset )
	{
		if(VideoCountingTimer==0)
		{
			Terminator_On() ;
			SwitchVideoState(VSTATE_PwrOff) ;
			return ;
		}
		VideoCountingTimer-- ;

		return ;
	}
	
	//==========================================2->VSTATE_PwrOff========================================================
	//Dongzejun:VSTATE_PwrOff HDMIÉÏµç£¬Ôò=>VSTATE_SyncWait
	if( VState == VSTATE_PwrOff )
	{
	    if(CheckPlg5VPwr())
	    {
            SwitchVideoState(VSTATE_SyncWait) ;
            return ;
	    }
	}

	//==========================================3->VSTATE_SyncWait========================================================
	//Dongzejun:  VSTATE_SyncWait
	if( VState == VSTATE_SyncWait)
	{
        if( AcceptCDRReset == TRUE )
        {
        	//Dongzejun;µ±ÔÚ×´Ì¬VSTATE_SyncWaitÏÂ¼ì²âµ½ÓÐÐ§µÄVCLKÊ±ÖÓÐÅºÅ£¬½«È¥¸´Î»
            if((HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE)&(B_VCLK_DET|B_RXCK_VALID)) == (B_VCLK_DET|B_RXCK_VALID))
            {
            	//AcceptCDRReset = FALSE;		//Dongzejun:´ËÓï¾äÎÞÓÃ
            	EnaSWCDRRest = FALSE;
            	CDR_Reset() ;
            }
        }
	}

	
//ÔÚ´ËÌõ¼þÏÂÓÐµÄ×´Ì¬:    VSTATE_SyncChecking / VSTATE_HDCPSet/ VSTATE_HDCP_Reset/ VSTATE_ModeDetecting/VSTATE_VideoOn,

	if((VState != VSTATE_PwrOff)&&(VState != VSTATE_SyncWait)&&(VState != VSTATE_SWReset))
	{
	    if(!IsSCDT())		//Dongzejun:SCDTÎ´¼ì²âµ½£¬ÔòÒªÇÐ»»µ½==>VSTATE_SyncWait
	    {
            SwitchVideoState(VSTATE_SyncWait) ;
            return ;
	    }
	}
	else if ((VState != VSTATE_PwrOff)&&(VState != VSTATE_SWReset))
	{
	    if(!CheckPlg5VPwr())	//Dongzejun:´ËHDMIÏß5VµçÔ´¶ªÊ§£¬Ôò¸´Î»HDMI
	    {

            SWReset_HDMIRX() ;
            return ;
	    }
	}

	//==========================================4->VSTATE_SyncWait /VSTATE_SyncChecking===============================================
	//Dongzejun:
    if( VState == VSTATE_SyncWait || VState == VSTATE_SyncChecking )
    {
        SWResetTimeOut-- ;
        if( SWResetTimeOut == 0 )
        {
            SWReset_HDMIRX() ;
            return ;
        }
    }

//====================VSTATE_SyncWait 1============================
    if( VState == VSTATE_SyncWait )
    {

        if( VideoCountingTimer == 0 )
        {
            HDMIRX_PRINTF(("VsyncWaitResetTimer up, call SWReset_HDMIRX()\n",VideoCountingTimer));
            SWReset_HDMIRX() ;
            return ;

        }
        else
        {
			uc=HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE);
        	HDMIRX_PRINTF(("REG_RX_SYS_STATE = %X\r",(int)uc));
			uc &= (B_RXPLL_LOCK|B_RXCK_VALID|B_SCDT|B_VCLK_DET);

			//Dongzejun:ÒÑ»ñµÃÕýÈ·µÄVIDEO¡¡SIGNAL
			if(uc == (B_RXPLL_LOCK|B_RXCK_VALID|B_SCDT|B_VCLK_DET)	)
        	{
            	SwitchVideoState(VSTATE_SyncChecking) ;	//Dongzejun:VSTATE_SyncWait==>VSTATE_SyncChecking
                return ;
       		}
			else
			{
				uc=HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATE);
				uc &= (B_RXCK_VALID|B_VCLK_DET|B_HDMIRX_MODE);
				#if 1     //´Ë¶Î³ÌÐò£¬²¢ÎÞÊµ¼ÊµÄÒâË¼
				if(uc == (B_RXCK_VALID|B_VCLK_DET))	//Dongzejun: DVIÄ£Ê½
				{
					if(ucDVISCDToffCNT++>100)
					{
						HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_SWRST);
						printf("\n\nrst_ctrl (1)= %X \n",(int)HDMIRX_ReadI2C_Byte(REG_RX_RST_CTRL));
						delay1ms(1);
						HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, 0);
						printf("rst_ctrl (2)= %X \n\n",(int)HDMIRX_ReadI2C_Byte(REG_RX_RST_CTRL));
						ucDVISCDToffCNT=0;
					}
				}
				else
				{
					ucDVISCDToffCNT=0;
				}
				#endif
		}

        VideoCountingTimer-- ;
        }
    }

//==========================================3->VSTATE_SyncChecking===============================================
//===============VSTATE_SyncChecking 3==========================
    if( VState == VSTATE_SyncChecking )
    {

        if( VideoCountingTimer == 0)
        {
            SwitchVideoState(VSTATE_ModeDetecting) ;		//Dongzejun:Í£ÁôÔÚVSTATE_SyncChecking³¬¹ý150ms£¬½«ÇÐ»»µ½VSTATE_ModeDetecting×´Ì¬
            return ;
        }
        else
        {
            VideoCountingTimer-- ;
        }
    }

//================VSTATE_HDCP_Reset 5================================
    if( VState == VSTATE_HDCP_Reset )
    {

        if( --VideoCountingTimer == 0)
        {
        	HDMIRX_PRINTF(("HDCP timer reach, reset !!\n"));

            SWReset_HDMIRX() ;
            return ;
        }
        else
        {
            HDMIRX_PRINTF(("VideoTimerHandler[VSTATE_HDCP_Reset](%d)\n",VideoCountingTimer));
            do {
	        	ClearIntFlags(B_CLR_ECC_INT) ;
	        	delay1ms(1) ;
	        	uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT3) ;
	        	if(uc & B_ECCERR)
	        	{
	                break ;
	        	}
	        	delay1ms(1) ;
	        	ClearIntFlags(B_CLR_ECC_INT) ;
	        	delay1ms(1) ;
	        	uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT3) ;
	        	if(!(uc & B_ECCERR))
	        	{
	                SwitchVideoState(VSTATE_VideoOn) ;
	                return ;
	        	}
	        }while(0) ;
        }
    }


//==========================================7->VSTATE_VideoOn===============================================
    if( VState == VSTATE_VideoOn )
    {
		char diff ;
		unsigned short HTotal ;
		unsigned char xCnt ;
		BOOL bVidModeChange = FALSE ;
		BOOL ScanMode ;

		HTotal = (unsigned short)HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_L) ;
		HTotal |= (unsigned short)(HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_H)&M_HTOTAL_H) << 8 ;
		if(ABS((int)HTotal -(int)currHTotal)>4)
		{
			bVidModeChange = TRUE ;		//Dongzejun:ÊÓÆµÐÅºÅÒÑ¸Ä±ä
			HDMIRX_PRINTF(("HTotal changed.\n"));
		}

		if(!bVidModeChange)
		{
			xCnt = (unsigned char)HDMIRX_ReadI2C_Byte(REG_RX_VID_XTALCNT_128PEL) ;

			diff = (char)currXcnt - (char)xCnt ;

			if( xCnt > 0x80 )
			{
				if( ABS(diff) > 6 )
				{
					HDMIRX_PRINTF(("Xcnt changed. %02x -> %02x ",(int)xCnt,(int)currXcnt));
					HDMIRX_PRINTF(("diff = %d\r\n",(int)diff));
					bVidModeChange = TRUE ;
				}
			}
			else if ( xCnt > 0x40 )
			{
				if( ABS(diff) > 4 )
				{
					HDMIRX_PRINTF(("Xcnt changed. %02x -> %02x ",(int)xCnt,(int)currXcnt));
					HDMIRX_PRINTF(("diff = %d\r\n",(int)diff));
					bVidModeChange = TRUE ;
				}
			}
			else if ( xCnt > 0x20 )
			{
				if( ABS(diff) > 2 )
				{
					HDMIRX_PRINTF(("Xcnt changed. %02x -> %02x ",(int)xCnt,(int)currXcnt));
					HDMIRX_PRINTF(("diff = %d\n\r",(int)diff));
					bVidModeChange = TRUE ;
				}
			}
			else
			{
				if( ABS(diff) > 1 )
				{
					HDMIRX_PRINTF(("Xcnt changed. %02x -> %02x ",(int)xCnt,(int)currXcnt));
					HDMIRX_PRINTF(("diff = %d\r\n",(int)diff));
					bVidModeChange = TRUE ;
				}
			}
		}

        if(s_CurrentVM.VActive < 300)
        {
    		if( !bVidModeChange )
    		{
    			ScanMode = (HDMIRX_ReadI2C_Byte(REG_RX_VID_MODE)&B_INTERLACE)?INTERLACE:PROG ;
    			if( ScanMode != currScanMode )
    			{
    				HDMIRX_PRINTF(("ScanMode change.\r\n"));
    				bVidModeChange = TRUE ;
    			}
    		}
        }

		if(bVidModeChange)
		{

			SwitchVideoState(VSTATE_SyncWait) ;
			return ;
		}
        else
        {
            unsigned char currAVI_DB1, currAVI_DB2 ;

            currAVI_DB1 = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB1) ;
            currAVI_DB2 = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB2) ;

            if( IsCAT6023HDMIMode()){
                if( (currAVI_DB1 != prevAVIDB1)||(currAVI_DB2 != prevAVIDB2)){
                    RXINT_SetNewAVIInfo() ;
                }
            }
            prevAVIDB1 = currAVI_DB1 ;
            prevAVIDB2 = currAVI_DB2 ;
        }

		if(VideoCountingTimer>0)
		{
            VideoCountingTimer-- ;
            if(VideoCountingTimer == 0)
            {
                AcceptCDRReset = TRUE ;
            }
		}

    }
}

static void SetupAudio()
//Dongzejun:ÓÐµãÒÉÎÊ
{
    BYTE uc ;
    BYTE RxAudioCtrl ;
    getCAT6023InputAudio(&AudioCaps) ;

    if(AudioCaps.AudioFlag & B_CAP_AUDIO_ON)  //Dongzejun:ÒôÆµÊÇ´ò¿ªµÄreg8a[7]:audio on
    {

        uc=HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL) & 0xF8;
        uc |=0x1;
        HDMIRX_WriteI2C_Byte(REG_RX_MCLK_CTRL,uc);	//Dongzejun: default set 256Fs

        uc=HDMIRX_ReadI2C_Byte(REG_RX_FS_SET)&0xCF;
        uc |= 0x70 ;
        HDMIRX_WriteI2C_Byte(REG_RX_FS_SET,uc);
				

        if(AudioCaps.AudioFlag& B_CAP_HBR_AUDIO)		//Dongzejun:Audio Type is High Bit Rate
        {

            Switch_HDMIRX_Bank(0) ;

            #ifdef _HBR_I2S_
            uc = HDMIRX_ReadI2C_Byte(REG_RX_HWAMP_CTRL) ;
            uc &= ~(1<<4) ;		//Dongzejun: 0 HBR Output by i2s
            HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL, uc) ;
            #else
            HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL,(HDMIRX_ReadI2C_Byte(REG_RX_HWAMP_CTRL)|0x10)); //Dongzejun: 0 HBR Output by SPDIF
            #endif

        #if 1
            uc=HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL) & 0xF8;
            uc |=0x0;		//Dongzejun:// 128xFs
            HDMIRX_WriteI2C_Byte(REG_RX_MCLK_CTRL,uc);
        #endif

            SetHWMuteClrMode();
            ResetAudio();
        }
        else if(AudioCaps.AudioFlag& B_CAP_DSD_AUDIO ) //Dongzejun:Audio Type is DSD
        {

            SetHWMuteClrMode();
            ResetAudio();
        }
        else    //Dongzejun:LPCM .....
        {
            uc = HDMIRX_ReadI2C_Byte(REG_RX_HWAMP_CTRL) ;
            HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL,uc &(~0x10));		//Dongzejun:HBR output by i2s

            ucHDMIAudioErrorCount++;
            RxAudioCtrl=HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CTRL);

            if(ucHDMIAudioErrorCount > 10)
            {
                ucHDMIAudioErrorCount=0;

                if ( RxAudioCtrl & B_FORCE_FS )
                {
                    RxAudioCtrl &= ~B_FORCE_FS;
                }

                else
                {
                    // Force Sample FS setting progress:
                    // a. if find Audio Error in a period timers,
                    // assum the FS message is wrong,then try to force FS setting.
                    // force sequence : 48KHz -> 44.1KHz -> 32KHz -> 96KHz ->  192KHz ->
                    // (88.2KHz -> 176.4KHz )
                    // -> 48KHz
                    //Dongzejun:ÉèÖÃ²ÉÑùÆµÂÊµÄ²½Öè£¬µ÷ÊÔÊ±¿ÉÒÔ×¢ÒâÒ»ÏÂ£¬´Ë³ÌÐò¶Î¡£
                    switch(ucAudioSampleClock)
                    {
                    case AUDFS_192KHz: ucAudioSampleClock=AUDFS_48KHz;break ;

                    case AUDFS_48KHz: ucAudioSampleClock=AUDFS_44p1KHz;break ;
                    case AUDFS_44p1KHz: ucAudioSampleClock=AUDFS_32KHz;break ;
                    case AUDFS_32KHz: ucAudioSampleClock=AUDFS_96KHz;break ;

                #ifndef SUPPORT_FORCE_88p2_176p4

                    case AUDFS_96KHz: ucAudioSampleClock=AUDFS_192KHz;break ;

                #else

                    case AUDFS_88p2KHz: ucAudioSampleClock=AUDFS_176p4KHz;break ;
                    case AUDFS_96KHz: ucAudioSampleClock=AUDFS_88p2KHz;break ;
                    case AUDFS_176p4KHz: ucAudioSampleClock=AUDFS_192KHz;break ;

                #endif

                    default: ucAudioSampleClock=AUDFS_48KHz;break;
                    }
                    HDMIRX_PRINTF(("===[Audio FS Error ]===\n"));
                    RxAudioCtrl |=B_FORCE_FS;
                }

            }
            RxAudioCtrl |= B_EN_I2S_NLPCM ;
            HDMIRX_WriteI2C_Byte(REG_RX_AUDIO_CTRL,RxAudioCtrl);

            uc=HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL);
            if(RxAudioCtrl & B_FORCE_FS)
            {

                uc|=B_CTSINI_EN;
            }
            else
            {
                uc &= ~B_CTSINI_EN;
            }
            HDMIRX_WriteI2C_Byte(REG_RX_MCLK_CTRL, uc) ;

            SetHWMuteClrMode();

            ResetAudio();

            if( RxAudioCtrl & B_FORCE_FS)
            {

                uc=HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL) ;
                uc &= 0xF0 ;
                uc |= ucAudioSampleClock & 0xF ;
                HDMIRX_WriteI2C_Byte(REG_RX_FS_SET,uc);
                HDMIRX_WriteI2C_Byte(REG_RX_FS_SET,uc);
                HDMIRX_WriteI2C_Byte(REG_RX_FS_SET,uc);
                HDMIRX_WriteI2C_Byte(REG_RX_FS_SET,uc);
            }
            SetIntMask3(~(B_AUTOAUDMUTE|B_AUDFIFOERR),(B_AUTOAUDMUTE|B_AUDFIFOERR)) ;
        }
        /*
        else
        {
            uc = HDMIRX_ReadI2C_Byte(REG_RX_HWAMP_CTRL) ;
            HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL,uc &(~0x10));
            SetHWMuteClrMode();
            ResetAudio();

        }
        */
        ClearIntFlags(B_CLR_AUDIO_INT);
        SetIntMask3(~(B_AUTOAUDMUTE|B_AUDFIFOERR),(B_AUTOAUDMUTE|B_AUDFIFOERR));
        SwitchAudioState(ASTATE_WaitForReady);

    }
    else
    {
        ucHDMIAudioErrorCount = 0 ;

        ucAudioSampleClock=DEFAULT_START_FIXED_AUD_SAMPLEFREQ ;

        uc=HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CTRL);
        uc &= ~B_FORCE_FS ;
        HDMIRX_WriteI2C_Byte(REG_RX_AUDIO_CTRL, uc);
        uc = HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL) ;
        uc &= ~B_CTSINI_EN;
        HDMIRX_WriteI2C_Byte(REG_RX_MCLK_CTRL, uc) ;
        HDMIRX_PRINTF2(("Audio Off, clear Audio Error Count.\n"));
    }
}


//Dongzejun:´ò¿ªÒôÆµ¹¦ÄÜ?
static void EnableAudio()
{

    SetupAudio() ;

    delay1ms(5) ;

    if(AudioCaps.AudioFlag & B_CAP_AUDIO_ON)
    {
        if( HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT3) & (B_AUTOAUDMUTE|B_AUDFIFOERR) )
        {
            SwitchAudioState(ASTATE_RequestAudio) ;
        }
        else
        {
            SwitchAudioState(ASTATE_AudioOn) ;
        }
    }
    else
    {
        SwitchAudioState(ASTATE_RequestAudio) ;
    }

}

void AudioTimerHandler()
{
    BYTE uc;
    AUDIO_CAPS CurAudioCaps ;

    switch(AState)
    {
    case ASTATE_RequestAudio:
        SetupAudio() ;
        break;

    case ASTATE_WaitForReady:
        if(AudioCountingTimer==0)
        {
            SwitchAudioState(ASTATE_AudioOn);
        }
        else
        {
            AudioCountingTimer --;
        }

        break;

    case ASTATE_AudioOn:
        getCAT6023InputAudio(&CurAudioCaps) ;

        if(AudioCaps.AudioFlag != CurAudioCaps.AudioFlag
           /* || AudioCaps.AudSrcEnable != CurAudioCaps.AudSrcEnable
           || AudioCaps.SampleFreq != CurAudioCaps.SampleFreq */ )
        {

            ucHDMIAudioErrorCount=0;

            ucAudioSampleClock = DEFAULT_START_FIXED_AUD_SAMPLEFREQ;

            uc=HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CTRL);
            uc &= ~B_FORCE_FS ;
            HDMIRX_WriteI2C_Byte(REG_RX_AUDIO_CTRL, uc);
            uc = HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL) ;
            uc &= ~B_CTSINI_EN;
            HDMIRX_WriteI2C_Byte(REG_RX_MCLK_CTRL, uc) ;

            HDMIRX_PRINTF(("Audio change, clear Audio Error Count.\n"));

            SetAudioMute(ON);
            SwitchAudioState(ASTATE_RequestAudio);

        }

		if ( AudioCountingTimer != 0 )
        {
            AudioCountingTimer -- ;
            if ( AudioCountingTimer == 0 )
            {
                ucHDMIAudioErrorCount=0 ;		//Dongzejun:Çå³ý´íÎó¼ÆÊý
    			HDMIRX_PRINTF(("Audio On, clear Audio Error Count.\n"));
            }
        }

        break;
    }
}

BYTE    CAT6023FsGet()
{
    BYTE RxFS ;
    RxFS=HDMIRX_ReadI2C_Byte(REG_RX_FS) & 0x0F;
    return    RxFS;
}

BOOL    CAT6023HDAudioGet()
{
    BOOL bRxHBR ;
    bRxHBR=(HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT)&(1<<6))?TRUE:FALSE;
    return    bRxHBR;
}

BOOL    CAT6023MultiPCM()
{
    BOOL bRxMultiCh ;
    bRxMultiCh=(HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT) & (1<<4))?TRUE:FALSE;
    return    bRxMultiCh;
}

BYTE    CAT6023AudioChannelNum()
{
    BYTE RxChEn ;
    RxChEn=HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT) & M_AUDIO_CH;

    return     RxChEn;
}

void    CAT6023HBRMclkSet(BYTE cFs)
{
    BYTE    uc;
    uc=HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL) & 0xF8;

    switch(cFs)
    {
    case    9:
        uc |=0x0;
        break;
    default:
        uc |=0x1;
        break;

    }
    HDMIRX_WriteI2C_Byte(REG_RX_MCLK_CTRL,uc);

}

void getCAT6023InputAudio(AUDIO_CAPS *pAudioCaps)
//Dongzejun:»ñÈ¡HDMIÊäÈëÒôÆµÖµ¡¡
{
    BYTE uc ;

    if( !pAudioCaps )
    {
        return ;
    }
    Switch_HDMIRX_Bank(0) ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_FS) ;
    pAudioCaps->SampleFreq=uc&M_Fs ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT) ;
    pAudioCaps->AudioFlag = uc & 0xF0 ;

	//Dongzejun:ÒÔÏÂÕâÐ©³ÌÐò¶ÎÓÐÒâÒåÂð¿
    pAudioCaps->AudSrcEnable=uc&M_AUDIO_CH ;
    delay1ms(1) ;
    pAudioCaps->AudSrcEnable|=HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT)&M_AUDIO_CH ;
    delay1ms(1) ;
    pAudioCaps->AudSrcEnable|=HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT)&M_AUDIO_CH ;
    delay1ms(1) ;
    pAudioCaps->AudSrcEnable|=HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT)&M_AUDIO_CH ;

    if( (uc & (B_HBRAUDIO|B_DSDAUDIO)) == 0)  //Dongzejun:²»ÊÇHBRºÍDSDÒôÆµ¸ñÊ½
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT0) ;

        if( (uc & B_AUD_NLPCM ) == 0 )
        {
            pAudioCaps->AudioFlag |= B_CAP_LPCM; //Dongzejun:LPCM
        }
    }

}

void getCAT6023InputChStat(AUDIO_CAPS *pAudioCaps)
{
    BYTE uc ;

    if( !pAudioCaps )
    {
        return ;
    }

    uc = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT0) ;
    pAudioCaps->ChStat[0] = uc;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT1) ;
    pAudioCaps->ChStat[1] = uc;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT2) ;
    pAudioCaps->ChStat[2] = uc;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT3) ;

    pAudioCaps->ChStat[3] = uc & M_CLK_ACCURANCE;
    pAudioCaps->ChStat[3] <<= 4 ;
    pAudioCaps->ChStat[3] |= ((BYTE)pAudioCaps->SampleFreq)&0xF ;

    pAudioCaps->ChStat[4] = (~((BYTE)pAudioCaps->SampleFreq))&0xF ;
    pAudioCaps->ChStat[4] <<= 4 ;
    pAudioCaps->ChStat[4] |= (uc & M_SW_LEN )>>O_SW_LEN;

}

static void MuteProcessTimerHandler()
//Dongzejun:
{
    BYTE uc ;
    BOOL TurnOffMute = FALSE ;

    if( MuteByPKG == ON )   //Dongzejun:¾²Òô
    {

        if( (MuteResumingTimer > 0)&&(AState == ASTATE_AudioOn))
        {
            MuteResumingTimer -- ;
            uc = HDMIRX_ReadI2C_Byte(REG_RX_VID_INPUT_ST) ;
            HDMIRX_PRINTF(("MuteResumingTimer = %d uc = %02X\n",MuteResumingTimer , (int)uc));

            if(!(uc&B_AVMUTE))
            {
                TurnOffMute = TRUE ;
                MuteByPKG = OFF ;
            }
            else if((MuteResumingTimer == 0))
            {
                bDisableAutoAVMute = B_VDO_MUTE_DISABLE ;

                uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL) ;
                uc |= B_VDO_MUTE_DISABLE ;
                HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc) ;

                TurnOffMute = TRUE ;
                MuteByPKG = OFF ;
            }
        }

        if ( MuteAutoOff )
        {
            uc = HDMIRX_ReadI2C_Byte(REG_RX_VID_INPUT_ST) ;
            if(!(uc & B_AVMUTE))
            {
                EndAutoMuteOffTimer() ;
                TurnOffMute = TRUE ;
            }
        }
    }

    if( TurnOffMute )
    {
        if(VState == VSTATE_VideoOn )
        {
            SetVideoMute(OFF) ;
            if(AState == ASTATE_AudioOn )
            {
                SetAudioMute(OFF) ;
            }
        }
    }
}

void AssignVideoTimerTimeout(USHORT TimeOut)
//Dongzejun:
{
    VideoCountingTimer = TimeOut ;
}

void
AssignAudioTimerTimeout(USHORT TimeOut)
{
    AudioCountingTimer = TimeOut ;

}

#if 0
void
ResetVideoTimerTimeout()
{
    VideoCountingTimer = 0 ;
}

void
ResetAudioTimerTimeout()
{
    AudioCountingTimer = 0 ;
}
#endif

void SwitchVideoState(Video_State_Type state)
//Dongzejun:Video ×´Ì¬ÉèÖÃ
{
	if( VState == state )
	{
		return ;
	}
	
	//Dongzejun:Ö®Ç°µÄ×´Ì¬ÊÇVideo On,ÏÖÔÚµÄ×´Ì¬²»ÊÇVideoOnÁË,ÔòÒª¹Ø±ÕÒôÆµ 2011/05/07
    if( VState == VSTATE_VideoOn && state != VSTATE_VideoOn)		
    {
		//Dongzejun:ÇÐ»»ÒôÆµ×´Ì¬ÎªAudioOff
        SwitchAudioState(ASTATE_AudioOff) ;		
    }

    VState = state ;

    HDMIRX_PRINTF1(("RX VState -> %s\n",VStateStr[VState]));

    if( VState != VSTATE_SyncWait && VState != VSTATE_SyncChecking )
    {
        SWResetTimeOut = FORCE_SWRESET_TIMEOUT;	//Dongzejun:µÈ´ýÍ¬²½SCDT¡¢VCLKÊ±¼äÒç³ö

    }

    switch(VState)
    {
    case VSTATE_PwrOff:
        AcceptCDRReset = TRUE ;
        break ;
    case VSTATE_SWReset:
        HDMIRX_WriteI2C_Byte(REG_RX_GEN_PKT_TYPE, 0x03) ;
        AssignVideoTimerTimeout(VSTATE_SWRESET_COUNT);
    	break ;

	//Dongzejun:1
    case VSTATE_SyncWait:

        SetIntMask1(~(B_SCDTOFF|B_VIDMODE_CHG),0) ;		//Dongzejun:Çå³ýÕâÁ½Î»ÖÐ¶ÏÆÁ±Î1Reg16[5/3]
        HDMIRX_WriteI2C_Byte(REG_RX_GEN_PKT_TYPE, 0x03) ;	//Dongzejun:General Control Packet
        SetVideoMute(ON) ;
        AssignVideoTimerTimeout(VSTATE_MISS_SYNC_COUNT);
        break ;
    case VSTATE_SyncChecking:	//Dongzejun:VSTATE_SyncWait==>VSTATE_SyncChecking
        HDMIRX_WriteI2C_Byte(REG_RX_GEN_PKT_TYPE, 0x03) ;

		 //Dongzejun:R	eg0x16ÖÐ¶Ï1ÆÁ±Î¼Ä´æÆ÷¡£¿ªÆô´ËÁ½Î»ÖÐ¶Ï
        SetIntMask1(~(B_SCDTOFF|B_VIDMODE_CHG),(B_SCDTOFF|B_VIDMODE_CHG)) ; 
        AssignVideoTimerTimeout(VSATE_CONFIRM_SCDT_COUNT);
        break ;
	case VSTATE_HDCP_Reset:
        SetVideoMute(ON) ;
		AssignVideoTimerTimeout(HDCP_WAITING_TIMEOUT);
		break ;

	//Dongzejun:¼ì²âµ½VIDEOÄ£Ê½£¬´ò¿ªÊÓÆµ
    case VSTATE_VideoOn:
        HDMIRX_WriteI2C_Byte(REG_RX_GEN_PKT_TYPE, 0x81) ;

        AssignVideoTimerTimeout(CDRRESET_TIMEOUT);

		//Dongzejun:Ã»ÓÐAVIÖ¡Ê±£¬ÒªÖ´ÐÐµÄ´Ë²¿·Ö
        if(!NewAVIInfoFrameF)    //ÎÞAVI
        {
            SetVideoInputFormatWithoutInfoFrame(F_MODE_RGB24) ;
            SetColorimetryByMode(/*&SyncInfo*/) ;
            SetColorSpaceConvert() ;
        }

        if( !IsCAT6023HDMIMode())  //Dongzejun:DVI
        {
            SetIntMask1(~(B_SCDTOFF|B_PWR5VOFF),(B_SCDTOFF|B_PWR5VOFF)) ;
            SetVideoMute(OFF) ;
            SwitchAudioState(ASTATE_AudioOff) ;
            NewAVIInfoFrameF = FALSE ;
        }
				
        else		//Dongzejun:HDMI
        {
        	BYTE uc ;

            if( NewAVIInfoFrameF )
            {
                SetNewInfoVideoOutput() ;
            }

        #ifdef SUPPORT_REPEATER
            if( bHDCPMode & HDCP_REPEATER )
            {
                SetIntMask3(0,B_ECCERR|B_R_AUTH_DONE|B_R_AUTH_START) ;
        	}
        	else
        #endif
        	{
                SetIntMask3(~(B_R_AUTH_DONE|B_R_AUTH_START),B_ECCERR) ;
        	}
            SetIntMask2(~(B_NEW_AVI_PKG|B_PKT_SET_MUTE|B_PKT_CLR_MUTE),(B_NEW_AVI_PKG|B_PKT_SET_MUTE|B_PKT_CLR_MUTE)) ;
            SetIntMask1(~(B_SCDTOFF|B_PWR5VOFF),(B_SCDTOFF|B_PWR5VOFF)) ;
            SetIntMask4(0,B_M_RXCKON_DET) ;
//======================ÒÔÏÂÊÇÒôÆµ²¿·Öµ÷Õû=================================================
            MuteByPKG =  (HDMIRX_ReadI2C_Byte(REG_RX_VID_INPUT_ST) & B_AVMUTE)?TRUE:FALSE ;

            SetVideoMute(MuteByPKG) ;

            ucHDMIAudioErrorCount = 0 ;

            ucAudioSampleClock=DEFAULT_START_FIXED_AUD_SAMPLEFREQ ;

            uc = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CTRL) ;
            uc &= ~B_FORCE_FS ;
            HDMIRX_WriteI2C_Byte(REG_RX_AUDIO_CTRL, uc) ;

            uc = HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL) & (~B_CTSINI_EN);
            HDMIRX_WriteI2C_Byte(REG_RX_MCLK_CTRL, uc) ;

            HDMIRX_PRINTF(("[%s:%d] reg%02X = %02X\n",__FILE__,__LINE__,(int)REG_RX_AUDIO_CTRL, (int)uc));

			EnableAudio() ;
        }

		currHTotal = s_CurrentVM.HTotal ;
		currXcnt = s_CurrentVM.xCnt ;
		currScanMode = s_CurrentVM.ScanMode ;

        break ;
    }
}

void SwitchAudioState(Audio_State_Type state)
//Dongzejun:ÒôÆµ×´Ì¬ÇÐ»»
{
    AState = state ;
    HDMIRX_PRINTF(("AState -> %s\n",AStateStr[AState]));

    switch(AState)
    {
    case ASTATE_AudioOff:
        SetAudioMute(TRUE) ;
        break ;

    case ASTATE_WaitForReady:
        AssignAudioTimerTimeout(AUDIO_READY_TIMEOUT) ;
        break ;
    case ASTATE_AudioOn:
        SetAudioMute(MuteByPKG) ;
        AssignAudioTimerTimeout(AUDIO_CLEARERROR_TIMEOUT) ;
        if( MuteByPKG )
        {
            HDMIRX_PRINTF(("AudioOn, but still in mute.\n"));
            EnableMuteProcessTimer() ;
        }
        break ;
    }
}

static void
DumpSyncInfo(VTiming *pVTiming)
{
    double VFreq ;
    HDMIRX_PRINTF2(("{%4d,",pVTiming->HActive));
    HDMIRX_PRINTF2(("%4d,",pVTiming->VActive));
    HDMIRX_PRINTF2(("%4d,",pVTiming->HTotal));
    HDMIRX_PRINTF2(("%4d,",pVTiming->VTotal));
    HDMIRX_PRINTF2(("%8ld,",pVTiming->PCLK));
    HDMIRX_PRINTF2(("0x%02x,",pVTiming->xCnt));
    HDMIRX_PRINTF2(("%3d,",pVTiming->HFrontPorch));
    HDMIRX_PRINTF2(("%3d,",pVTiming->HSyncWidth));
    HDMIRX_PRINTF2(("%3d,",pVTiming->HBackPorch));
    HDMIRX_PRINTF2(("%2d,",pVTiming->VFrontPorch));
    HDMIRX_PRINTF2(("%2d,",pVTiming->VSyncWidth));
    HDMIRX_PRINTF2(("%2d,",pVTiming->VBackPorch));
    HDMIRX_PRINTF2(("%s,",pVTiming->ScanMode?"PROG":"INTERLACE"));
    HDMIRX_PRINTF2(("%s,",pVTiming->VPolarity?"Vpos":"Vneg"));
    HDMIRX_PRINTF2(("%s},",pVTiming->HPolarity?"Hpos":"Hneg"));
    VFreq = (double)pVTiming->PCLK ;
    VFreq *= 1000.0 ;
    VFreq /= pVTiming->HTotal ;
    VFreq /= pVTiming->VTotal ;
}

static BOOL bGetSyncInfo()
//Dongzejun:»ñµÃVIDEOÐÅºÅ¸ñÊ½
// 1:»ñµÃÕýÈ·µÄVIDEO¡¡ID
// 0:»ñµÃ´íÎóµÄVIDEO ID
{
    long diff ;

    BYTE uc1, uc2, uc3 ;
    int i ;

    uc1 = HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_L) ;
    uc2 = HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_H) ;
    uc3 = HDMIRX_ReadI2C_Byte(REG_RX_VID_HACT_L) ;

    s_CurrentVM.HTotal = ((WORD)(uc2&0xF)<<8) | (WORD)uc1;
    s_CurrentVM.HActive = ((WORD)(uc2 & 0x70)<<4) | (WORD)uc3 ;
    if( (s_CurrentVM.HActive | (1<<11)) <s_CurrentVM.HTotal )
    {
        s_CurrentVM.HActive |= (1<<11) ;
    }
    uc1 = HDMIRX_ReadI2C_Byte(REG_RX_VID_HSYNC_WID_L) ;
    uc2 = HDMIRX_ReadI2C_Byte(REG_RX_VID_HSYNC_WID_H) ;
    uc3 = HDMIRX_ReadI2C_Byte(REG_RX_VID_H_FT_PORCH_L) ;

    s_CurrentVM.HSyncWidth = ((WORD)(uc2&0x1)<<8) | (WORD)uc1;
    s_CurrentVM.HFrontPorch = ((WORD)(uc2 & 0xf0)<<4) | (WORD)uc3 ;
    s_CurrentVM.HBackPorch = s_CurrentVM.HTotal - s_CurrentVM.HActive - s_CurrentVM.HSyncWidth - s_CurrentVM.HFrontPorch ;

    uc1 = HDMIRX_ReadI2C_Byte(REG_RX_VID_VTOTAL_L) ;
    uc2 = HDMIRX_ReadI2C_Byte(REG_RX_VID_VTOTAL_H) ;
    uc3 = HDMIRX_ReadI2C_Byte(REG_RX_VID_VACT_L) ;

    s_CurrentVM.VTotal = ((WORD)(uc2&0x7)<<8) | (WORD)uc1;
    s_CurrentVM.VActive = ((WORD)(uc2 & 0x30)<<4) | (WORD)uc3 ;
    if( (s_CurrentVM.VActive | (1<<10)) <s_CurrentVM.VTotal )
    {
        s_CurrentVM.VActive |= (1<<10) ;
    }

    s_CurrentVM.VBackPorch = HDMIRX_ReadI2C_Byte(REG_RX_VID_VSYNC2DE) ;
    s_CurrentVM.VFrontPorch = HDMIRX_ReadI2C_Byte(REG_RX_VID_V_FT_PORCH) ;
    s_CurrentVM.VSyncWidth = 0 ;
		
//Dongzejun:Ö¸Ê¾ÊÇ·ñÊÇ¸ôÐÐ/gÖðÐÐ?
    s_CurrentVM.ScanMode = (HDMIRX_ReadI2C_Byte(REG_RX_VID_MODE)&B_INTERLACE)?INTERLACE:PROG ;

    s_CurrentVM.xCnt = HDMIRX_ReadI2C_Byte(REG_RX_VID_XTALCNT_128PEL) ;

    if(  s_CurrentVM.xCnt )
    {
        s_CurrentVM.PCLK = 128L * 27000L / s_CurrentVM.xCnt ;	//Dongzejun:ÏñËØÊ±ÖÓ
    }
    else
    {
        HDMIRX_PRINTF(("s_CurrentVM.xCnt == %02x\n",s_CurrentVM.xCnt));
        s_CurrentVM.PCLK = 1234 ;
        /*
        for( i = 0x58 ; i < 0x66 ; i++ )
        {
            HDMIRX_PRINTF(("HDMIRX_ReadI2C_Byte(%02x) = %02X\n",i,(int)HDMIRX_ReadI2C_Byte(i)));
        }
        */
        return FALSE ;		//Dongzejun: videoÓÐ´íÎó·µ»Ø0
    }


//Dongzejun:Î´¶¨Òå¸÷ÖÖVIDEO¡¡ID¡¡±í¸ñ
#ifndef USE_MODE_TABLE
	if( (s_CurrentVM.VActive > 200)&&(s_CurrentVM.VTotal>s_CurrentVM.VActive )&&(s_CurrentVM.HActive > 300)&&(s_CurrentVM.HTotal>s_CurrentVM.HActive ))
	{
		return TRUE ;
	}
#else

//Dongzejun:¶¨ÒåÁË¸÷ÖÖVIDEO¡¡ID¡¡
    for( i = 0 ; i < SizeofVMTable ; i++ )
    {

        diff = ABS(s_VMTable[i].PCLK - s_CurrentVM.PCLK) ;
        diff *= 100 ;
        diff /= s_VMTable[i].PCLK ;

        if( diff > 3 )
        {

            continue ;
        }

        if( s_VMTable[i].HActive != s_CurrentVM.HActive )
        {
            continue ;
        }

        diff = (long)s_VMTable[i].HTotal - (long)s_CurrentVM.HTotal ;
        if( ABS(diff)>4)
        {
            continue ;
        }

        diff = (long)s_VMTable[i].VActive - (long)s_CurrentVM.VActive ;
        if( ABS(diff)>10)
        {
            continue ;
        }

        diff = (long)s_VMTable[i].VTotal - (long)s_CurrentVM.VTotal ;
        if( ABS(diff)>40)
        {
            continue ;
        }

        if( s_VMTable[i].ScanMode != s_CurrentVM.ScanMode )
        {
            continue ;
        }

        s_CurrentVM = s_VMTable[i] ;

        return TRUE ;		//Dongzejun:»ñµÃÒ»¸öÕýÈ·µÄVIDEO¡¡ID
    }

    for( i = 0 ; i < SizeofVMTable ; i++ )
    {

        diff = ABS(s_VMTable[i].PCLK - s_CurrentVM.PCLK) ;
        diff *= 100 ;
        diff /= s_VMTable[i].PCLK ;

        if( diff > 3 )
        {

            continue ;
        }

        if( s_VMTable[i].HActive != s_CurrentVM.HActive )
        {
            continue ;
        }

        diff = (long)s_VMTable[i].HTotal - (long)s_CurrentVM.HTotal ;
        if( ABS(diff)>4)
        {
            continue ;
        }

        diff = (long)s_VMTable[i].VActive - (long)s_CurrentVM.VActive ;
        if( ABS(diff)>10)
        {
            continue ;
        }

        diff = (long)s_VMTable[i].VTotal - (long)s_CurrentVM.VTotal ;
        if( ABS(diff)>40)
        {
            continue ;
        }
        s_CurrentVM = s_VMTable[i] ;

        return TRUE ;
    }
#endif
    return FALSE ;
}

#define SIZE_OF_CSCOFFSET (REG_RX_CSC_RGBOFF - REG_RX_CSC_YOFF + 1)
#define SIZE_OF_CSCMTX  (REG_RX_CSC_MTX33_H - REG_RX_CSC_MTX11_L + 1)
#define SIZE_OF_CSCGAIN (REG_RX_CSC_GAIN3V_H - REG_RX_CSC_GAIN1V_L + 1)

void Video_Handler()
//Dongzejun:
{

    BOOL bHDMIMode;

    if(VState == VSTATE_ModeDetecting)
    {
        HDMIRX_PRINTF(("Video_Handler, VState = VSTATE_ModeDetecting.\n"));

        ClearIntFlags(B_CLR_MODE_INT) ;

        if(!bGetSyncInfo())
        {
        //Dongzejun:Î´»ñµÃÕýÈ·µÄÐÅÏ¢¡£
            HDMIRX_PRINTF(("Current Get: ")); DumpSyncInfo(&s_CurrentVM) ;

            SwitchVideoState(VSTATE_SyncWait) ;
            bGetSyncFailCount ++ ;
            HDMIRX_PRINTF(("bGetSyncInfo() fail, bGetSyncFailCount = %d ", bGetSyncFailCount));
            if( bGetSyncFailCount % 32 == 31 )
            {
                HDMIRX_PRINTF((" called SWReset\n"));
                SWReset_HDMIRX() ;
            }
            else if( bGetSyncFailCount % 8 == 7)
            {
                HDMIRX_PRINTF((" reset video.\n"));

                HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_VDORST ) ;
                delay1ms(1) ;
                HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, 0) ;

            }
            else
            {
                HDMIRX_PRINTF(("\n"));
            }
            return ;
        }
        else
        {

            HDMIRX_PRINTF(("Matched Result: ")); DumpSyncInfo(&s_CurrentVM) ;
            bGetSyncFailCount = 0 ;		//Dongzejun:»ñµÃÕýÈ·µÄVIDEO ID
        }

        SetDefaultRegisterValue() ;

        bHDMIMode = IsCAT6023HDMIMode() ;

        if(!bHDMIMode)
        {

            HDMIRX_PRINTF(("This is DVI Mode.\n"));
            NewAVIInfoFrameF = FALSE ;
        }

        if( HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT1) & (B_VIDMODE_CHG|B_SCDTOFF|B_PWR5VOFF))
        {
            SwitchVideoState(VSTATE_SyncWait) ;

        }
        else
        {

            SwitchVideoState(VSTATE_VideoOn) ;
        }

        return ;
    }
}

static void SetVideoInputFormatWithoutInfoFrame(BYTE bInMode)
//Dongzejun:Ã»ÓÐAVI
{
    BYTE uc ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL) ;
    uc |= B_FORCE_COLOR_MODE ;
    bInputVideoMode &= ~F_MODE_CLRMOD_MASK ;

    switch(bInMode)
    {
    case F_MODE_YUV444:
        uc &= ~(M_INPUT_COLOR_MASK<<O_INPUT_COLOR_MODE) ;
        uc |= B_INPUT_YUV444 << O_INPUT_COLOR_MODE ;
        bInputVideoMode |= F_MODE_YUV444 ;
        break ;
    case F_MODE_YUV422:
        uc &= ~(M_INPUT_COLOR_MASK<<O_INPUT_COLOR_MODE) ;
        uc |= B_INPUT_YUV422 << O_INPUT_COLOR_MODE ;
        bInputVideoMode |= F_MODE_YUV422 ;
        break ;
    case F_MODE_RGB24:
        uc &= ~(M_INPUT_COLOR_MASK<<O_INPUT_COLOR_MODE) ;
        uc |= B_INPUT_RGB24 << O_INPUT_COLOR_MODE ;
        bInputVideoMode |= F_MODE_RGB24 ;
        break ;
    default:
        HDMIRX_PRINTF(("Invalid Color mode %d, ignore.\n", bInMode));
        return ;
    }
    HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc) ;

}

//Dongzejun:¸ßÇå£¬»òÕß±êÇå?
static void SetColorimetryByMode(/*PSYNC_INFO pSyncInfo*/)
{

    bInputVideoMode &= ~F_MODE_ITU709 ;

    if((s_CurrentVM.HActive == 1920)||(s_CurrentVM.HActive == 1280 && s_CurrentVM.VActive == 720) )
    {

        bInputVideoMode |= F_MODE_ITU709 ;
    }
    else
    {

        bInputVideoMode &= ~F_MODE_ITU709 ;
    }
}

void SetVideoInputFormatWithInfoFrame()
//Dongzejun:½ÓÊÕµ½Ò»¸öÐÂµÄAVI°ü¡£Êä³öÉ«²Ê¿Õ¼äÉèÖÃbInputVideoMode
{
    BYTE uc ;
    BOOL bAVIColorModeIndicated = FALSE ;

	//Dongzejun:Á½¸ö±äÁ¿¸ñÊ½²»¶Ô£¬BOOL¡¢BYTE!!!!!!!!!!!!!!!!!!
	//BOOL==>BYTE
    BOOL bOldInputVideoMode = bInputVideoMode ;  
	
    HDMIRX_PRINTF(("SetVideoInputFormatWithInfoFrame(): "));

    uc = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB1) ;	//Dongzejun:Reg0xae P6 DATA BYTE1¡¡AVI×Ö½Ú1ÄÚÈÝ
    HDMIRX_PRINTF(("REG_RX_AVI_DB1 %02X get uc %02X ",(int)REG_RX_AVI_DB1,(int)uc));

    prevAVIDB1 = uc ;		//Dongzejun:ÖÐ¶ÏÒýÆðµÄ£¬½ÓÊÕÒ»¸öÐÂµÄAVI¡¡°ü
    bInputVideoMode &= ~F_MODE_CLRMOD_MASK ;

    switch((uc>>O_AVI_COLOR_MODE)&M_AVI_COLOR_MASK)
    {
    case B_AVI_COLOR_YUV444:
        HDMIRX_PRINTF(("input YUV444 mode "));
        bInputVideoMode |= F_MODE_YUV444 ;
        break ;
    case B_AVI_COLOR_YUV422:
        HDMIRX_PRINTF(("input YUV422 mode "));
        bInputVideoMode |= F_MODE_YUV422 ;
        break ;
    case B_AVI_COLOR_RGB24:
        HDMIRX_PRINTF(("input RGB24 mode "));
        bInputVideoMode |= F_MODE_RGB24 ;
        break ;
    default:
        HDMIRX_PRINTF(("Invalid input color mode, ignore.\n"));
        return ;
    }

    if( (bInputVideoMode & F_MODE_CLRMOD_MASK)!=(bOldInputVideoMode & F_MODE_CLRMOD_MASK))
    {
        HDMIRX_PRINTF(("Input Video mode changed."));
    }

    uc = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL) ;
    uc &= ~B_FORCE_COLOR_MODE ;
    HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc) ;

    HDMIRX_PRINTF(("\n"));
}

BOOL SetColorimetryByInfoFrame()
//Dongzejun: ÉèÖÃbInputVideoMode[4] 1:ITU709 0:ITU601
{
    BYTE uc ;
    BOOL bOldInputVideoMode = bInputVideoMode ;

    HDMIRX_PRINTF(("SetColorimetryByInfoFrame: NewAVIInfoFrameF = %s ",NewAVIInfoFrameF?"TRUE":"FALSE"));

    if(NewAVIInfoFrameF)
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB2) ;
        uc &= M_AVI_CLRMET_MASK<<O_AVI_CLRMET ;
        if(uc == (B_AVI_CLRMET_ITU601<<O_AVI_CLRMET))
        {
            HDMIRX_PRINTF(("F_MODE_ITU601\n"));
            bInputVideoMode &= ~F_MODE_ITU709 ;
            return TRUE ;
        }
        else if(uc == (B_AVI_CLRMET_ITU709<<O_AVI_CLRMET))
        {
            HDMIRX_PRINTF(("F_MODE_ITU709\n"));
            bInputVideoMode |= F_MODE_ITU709 ;
            return TRUE ;
        }

        if( (bInputVideoMode & F_MODE_ITU709)!=(bOldInputVideoMode & F_MODE_ITU709))
        {
            HDMIRX_PRINTF(("Input Video mode changed."));

        }
    }
    HDMIRX_PRINTF(("\n"));
    return FALSE ;
}

void SetColorSpaceConvert()
//Dongzejun:ÉèÖÃReg0x20ºÍReg0x1C
{
    BYTE uc, csc ;
    BYTE filter = 0 ;

    switch(bOutputVideoMode&F_MODE_CLRMOD_MASK)
    {
    //Dongzejun:ÎÒÃÇµÄs-1071fËùÉèÖÃµÄCAT6023Êä³öÊÇYUV444
	#ifdef OUTPUT_YUV444
    case F_MODE_YUV444:

	    switch(bInputVideoMode&F_MODE_CLRMOD_MASK)
	    {
	    case F_MODE_YUV444:

	        csc = B_CSC_BYPASS ;
	        break ;
	    case F_MODE_YUV422:

            csc = B_CSC_BYPASS ;
            if( bOutputVideoMode & F_MODE_EN_UDFILT)
            {
                filter |= B_RX_EN_UDFILTER ;
            }

            if( bOutputVideoMode & F_MODE_EN_DITHER)
            {
                filter |= B_RX_EN_UDFILTER | B_RX_DNFREE_GO ;
            }

            break ;
	    case F_MODE_RGB24:

            csc = B_CSC_RGB2YUV ;
            break ;
	    }
        break ;
	#endif
	#ifdef OUTPUT_YUV422
    case F_MODE_YUV422:
	    switch(bInputVideoMode&F_MODE_CLRMOD_MASK)
	    {
	    case F_MODE_YUV444:

	        if( bOutputVideoMode & F_MODE_EN_UDFILT)
	        {
	            filter |= B_RX_EN_UDFILTER ;
	        }
	        csc = B_CSC_BYPASS ;
	        break ;
	    case F_MODE_YUV422:

            csc = B_CSC_BYPASS ;

            if( bOutputVideoMode & F_MODE_EN_DITHER)
            {
                filter |= B_RX_EN_UDFILTER | B_RX_DNFREE_GO ;
            }
	    	break ;
	    case F_MODE_RGB24:

            if( bOutputVideoMode & F_MODE_EN_UDFILT)
            {
                filter |= B_RX_EN_UDFILTER ;
            }
            csc = B_CSC_RGB2YUV ;
	    	break ;
	    }
	    break ;
	#endif
	#ifdef OUTPUT_RGB444
    case F_MODE_RGB24:

	    switch(bInputVideoMode&F_MODE_CLRMOD_MASK)
	    {
	    case F_MODE_YUV444:

	        csc = B_CSC_YUV2RGB ;
	        break ;
	    case F_MODE_YUV422:

            csc = B_CSC_YUV2RGB ;
            if( bOutputVideoMode & F_MODE_EN_UDFILT)
            {
                filter |= B_RX_EN_UDFILTER ;
            }
            if( bOutputVideoMode & F_MODE_EN_DITHER)
            {
                filter |= B_RX_EN_UDFILTER | B_RX_DNFREE_GO ;
            }
	    	break ;
	    case F_MODE_RGB24:

            csc = B_CSC_BYPASS ;
	    	break ;
	    }
	    break ;
	#endif
    }

	#ifdef OUTPUT_YUV

    if( csc == B_CSC_RGB2YUV )	//Dongzejun:RGB=>YUV
    {

        if(bInputVideoMode & F_MODE_ITU709)	//Dongzejun: ¸ßÇå
        {
            HDMIRX_PRINTF(("ITU709 "));

            if(bInputVideoMode & F_MODE_16_235)
            {
                HDMIRX_PRINTF((" 16-235\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF,bCSCOffset_16_235,sizeof(bCSCOffset_16_235)) ;
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU709_16_235,sizeof(bCSCMtx_RGB2YUV_ITU709_16_235)) ;
            }
            else
            {
                HDMIRX_PRINTF((" 0-255\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF,bCSCOffset_0_255,sizeof(bCSCOffset_0_255)) ;
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU709_0_255,sizeof(bCSCMtx_RGB2YUV_ITU709_0_255)) ;
            }
        }
        else
        {
            HDMIRX_PRINTF(("ITU601 "));
            if(bInputVideoMode & F_MODE_16_235)
            {
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF,bCSCOffset_16_235,sizeof(bCSCOffset_16_235)) ;
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU601_16_235,sizeof(bCSCMtx_RGB2YUV_ITU601_16_235)) ;
                HDMIRX_PRINTF((" 16-235\n"));
            }
            else
            {
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF,bCSCOffset_0_255,sizeof(bCSCOffset_0_255)) ;
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU601_0_255,sizeof(bCSCMtx_RGB2YUV_ITU601_0_255)) ;
                HDMIRX_PRINTF((" 0-255\n"));
            }
        }
    }
	#endif

	#ifdef OUTPUT_RGB
	if ( csc == B_CSC_YUV2RGB )
    {
        HDMIRX_PRINTF(("CSC = YUV2RGB "));
        if(bInputVideoMode & F_MODE_ITU709)
        {
            HDMIRX_PRINTF(("ITU709 "));
            if(bOutputVideoMode & F_MODE_16_235)
            {
                HDMIRX_PRINTF(("16-235\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF,bCSCOffset_16_235,sizeof(bCSCOffset_16_235)) ;
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU709_16_235,sizeof(bCSCMtx_YUV2RGB_ITU709_16_235)) ;
            }
            else
            {
                HDMIRX_PRINTF(("0-255\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF,bCSCOffset_0_255,sizeof(bCSCOffset_0_255)) ;
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU709_0_255,sizeof(bCSCMtx_YUV2RGB_ITU709_0_255)) ;
            }
        }
        else
        {
            HDMIRX_PRINTF(("ITU601 "));
            if(bOutputVideoMode & F_MODE_16_235)
            {
                HDMIRX_PRINTF(("16-235\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF,bCSCOffset_16_235,sizeof(bCSCOffset_16_235)) ;
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU601_16_235,sizeof(bCSCMtx_YUV2RGB_ITU601_16_235)) ;
            }
            else
            {
                HDMIRX_PRINTF(("0-255\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF,bCSCOffset_0_255,sizeof(bCSCOffset_0_255)) ;
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU601_0_255,sizeof(bCSCMtx_YUV2RGB_ITU601_0_255)) ;
            }
        }

    }
	#endif

    uc = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL) ;
    uc = (uc & ~M_CSC_SEL_MASK)|csc ;
    HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL,uc) ;

	//Dongzejun:ÉèÖÃREG0x1c  Video Ctr
    uc = HDMIRX_ReadI2C_Byte(REG_RX_VIDEO_CTRL1) ;
    uc &= ~(B_RX_DNFREE_GO|B_RX_EN_DITHER|B_RX_EN_UDFILTER) ;
    uc |= filter ;
    HDMIRX_WriteI2C_Byte(REG_RX_VIDEO_CTRL1, uc) ;
}

void
SetDVIVideoOutput()
{

    SetVideoInputFormatWithoutInfoFrame(F_MODE_RGB24) ;
    SetColorimetryByMode(/*&SyncInfo*/) ;
    SetColorSpaceConvert() ;
}

void SetNewInfoVideoOutput()
//Dongzejun: Reg0xAE P6¡¡ÉèÖÃÐÂµÄVideoÊä³ö
{
    /*
    BYTE db1,db2,db3 ;

    do {
        delay1ms(10) ;
        db1 = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB1) ;
        delay1ms(10) ;
        db2 = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB1) ;
        delay1ms(10) ;
        db3 = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB1) ;
        HDMIRX_PRINTF(("SetNewInfoVideoOutput(): %02X %02X %02X\n",(int)db1,(int)db2,(int)db3));
    } while ( (db1 != db2)||(db2!=db3)) ;
    */

    SetVideoInputFormatWithInfoFrame() ;//Dongzejun:ÉèÖÃbInputVideoMode[1:0]½ÓÊÕµ½Ò»¸öÐÂµÄAVI°ü¡£Êä³öÉ«²Ê¿Õ¼äÉèÖÃ
    SetColorimetryByInfoFrame() ;			//Dongzejun: ÉèÖÃbInputVideoMode[4] 1:ITU709 0:ITU601
    SetColorSpaceConvert() ;
    DumpCat6023Reg();		//Dongzejun:CAT6023¼Ä´æÆ÷
}

void
SetCAT6023VideoOutputFormat(BYTE bOutputMapping, BYTE bOutputType, BYTE bOutputColorMode)
{
    BYTE uc ;
    SetVideoMute(ON) ;
    HDMIRX_PRINTF1(("%02X %02X %02X\n",(int)bOutputMapping,(int)bOutputType,(int)bOutputColorMode));
    HDMIRX_WriteI2C_Byte(REG_RX_VIDEO_CTRL1,bOutputType) ;
    HDMIRX_WriteI2C_Byte(REG_RX_VIDEO_MAP,bOutputMapping) ;
    bOutputVideoMode&=~F_MODE_CLRMOD_MASK;

    bOutputVideoMode |= bOutputColorMode&F_MODE_CLRMOD_MASK ;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_PG_CTRL2) & ~(M_OUTPUT_COLOR_MASK<<O_OUTPUT_COLOR_MODE);

    switch(bOutputVideoMode&F_MODE_CLRMOD_MASK)
    {
    case F_MODE_YUV444:
        uc |= B_OUTPUT_YUV444 << O_OUTPUT_COLOR_MODE ;
        break ;
    case F_MODE_YUV422:
        uc |= B_OUTPUT_YUV422 << O_OUTPUT_COLOR_MODE ;
        break ;
    }
    HDMIRX_PRINTF1(("write %02X %02X\n",(int)REG_RX_PG_CTRL2,(int)uc));
    HDMIRX_WriteI2C_Byte(REG_RX_PG_CTRL2, uc) ;

    if( VState == VSTATE_VideoOn )
    {
        if( IsCAT6023HDMIMode() )
        {
            SetNewInfoVideoOutput();
        }
        else
        {
            SetDVIVideoOutput();
        }
        SetVideoMute(MuteByPKG) ;
    }

}

void ResetAudio()
//Dongzejun:
{

    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_AUDRST) ;
    delay1ms(1) ;
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, 0) ;

}

void SetHWMuteCTRL(BYTE AndMask, BYTE OrMask)
//Dongzejun:Reg0x87
{
    BYTE uc ;

    if( AndMask )
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_HWMUTE_CTRL) ;
    }
    uc &= AndMask ;
    uc |= OrMask ;
    HDMIRX_WriteI2C_Byte(REG_RX_HWMUTE_CTRL,uc) ;

}

void SetVideoMute(BOOL bMute)
//Dongzejun:ÊÓÆµ¾²Ö¹or´ò¿ª  bMute=1¾²Òô£»£½0Õý³£
{
    BYTE uc ;
#ifdef SUPPORT_REPEATER
    if( bHDCPMode & HDCP_REPEATER )
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL) ;
        uc &= ~(B_TRI_VIDEO | B_TRI_VIDEOIO) ;
        uc |= B_VDO_MUTE_DISABLE ;
        HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc) ;
        return ;
    }

#endif
    if( bMute )	//Dongzejun:ÊÓÆµÊä³ö½ûÖ¹
    {

		uc = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL) ;
		uc |= B_VDIO_GATTING ;
		HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc) ;

        uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL) ;
        uc &= ~(B_TRI_VIDEO | B_TRI_VIDEOIO) ;
        uc |= B_VDO_MUTE_DISABLE ;
        HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc) ;

    }
    else  //Dongzejun:ÊÓÆµÍ¨µÀ´ò¿ª
    {
        if( VState == VSTATE_VideoOn )
        {
            uc = HDMIRX_ReadI2C_Byte(REG_RX_VIDEO_CTRL1) ;
            HDMIRX_WriteI2C_Byte(REG_RX_VIDEO_CTRL1,uc|B_565FFRST) ;	//Dongzejun:P4 Reg0x1C[1]='1'->'0' Reset video FIFO
            HDMIRX_WriteI2C_Byte(REG_RX_VIDEO_CTRL1,uc&(~B_565FFRST)) ;

            uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL) ;
            uc &= ~(B_TRI_VIDEO | B_TRI_VIDEOIO) ;
            if(HDMIRX_ReadI2C_Byte(REG_RX_VID_INPUT_ST)&B_AVMUTE)
            {
                uc |= B_VDO_MUTE_DISABLE ;
                HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc) ;
            }
            else
            {
                uc &= ~B_VDO_MUTE_DISABLE ;
                HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc) ;

        		uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL) ;
        		uc |= B_TRI_VIDEOIO ;
        		HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc) ;
        		HDMIRX_PRINTF(("reg %02X <- %02X = %02X\n",REG_RX_TRISTATE_CTRL,uc, HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL)));
        		uc &= ~B_TRI_VIDEOIO ;
        		HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc) ;
        		HDMIRX_PRINTF(("reg %02X <- %02X = %02X\n",REG_RX_TRISTATE_CTRL,uc, HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL)));

        		uc = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL) ;
        		uc |= B_VDIO_GATTING ;
        		HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc) ;
        		HDMIRX_PRINTF(("reg %02X <- %02X = %02X\n",REG_RX_CSC_CTRL,uc, HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL)));
        		uc &= ~B_VDIO_GATTING ;
        		HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc) ;
        		HDMIRX_PRINTF(("reg %02X <- %02X = %02X\n",REG_RX_CSC_CTRL,uc, HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL)));
        	}

        }
    }
}

void SetAudioMute(BOOL bMute)
//Dongzejun:1->ÒôÆµ¾²Òôor 0->´ò¿ª
{
    if( bMute )	//Dongzejun:¾²Òô
    {
        SetMUTE(~B_TRI_AUDIO, B_TRI_AUDIO) ;
    }
    else		//Dongzejun:ÒôÆµ´ò¿ª¡£
    {

        SetMUTE(~B_TRI_AUDIO, 0) ;
    }
}

BYTE
getCAT6023AudioStatus()
{
    BYTE uc,audio_status ;

    SwitchHDMIRXBank(0) ;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT) ;
    audio_status = 0 ;

    if( (uc & (B_AUDIO_ON|B_HBRAUDIO|B_DSDAUDIO)) == (BYTE)(B_AUDIO_ON|B_HBRAUDIO) )
    {
        audio_status = T_AUDIO_HBR ;
    }
    else if( (uc & (B_AUDIO_ON|B_HBRAUDIO|B_DSDAUDIO)) == (BYTE)(B_AUDIO_ON|B_DSDAUDIO) )
    {
        audio_status = T_AUDIO_DSD ;
    }
    else if( uc & B_AUDIO_ON)
    {
        if( HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT0) & (1<<1) )
        {

            audio_status = T_AUDIO_NLPCM ;
        }
        else
        {
            audio_status = T_AUDIO_LPCM ;
        }

        if( uc & B_AUDIO_LAYOUT )
        {
            audio_status |= F_AUDIO_LAYOUT_1 ;
        }

        if( uc & (1<<3) )
        {
            audio_status |= 4 ;
        }
        else if( uc & (1<<2) )
        {
            audio_status |= 3 ;
        }
        else if( uc & (1<<1) )
        {
            audio_status |= 2 ;
        }
        else if( uc & (1<<0) )
        {
            audio_status |= 1 ;
        }
    }

    return audio_status ;
}

BOOL
getCAT6023AudioChannelStatus(BYTE ucIEC60958ChStat[])
{
    BYTE fs,audio_status ;

    audio_status = getCAT6023AudioStatus() ;

    if( ((audio_status & T_AUDIO_MASK) == T_AUDIO_OFF) ||
        ((audio_status & T_AUDIO_MASK) == T_AUDIO_DSD) )
    {

        return FALSE ;
    }

    SwitchHDMIRXBank(0) ;
    ucIEC60958ChStat[0] = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT0) ;
    ucIEC60958ChStat[1] = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT1) ;
    ucIEC60958ChStat[2] = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT2) ;
    fs = HDMIRX_ReadI2C_Byte(REG_RX_FS) & M_Fs ;

    if( (audio_status & T_AUDIO_MASK) == T_AUDIO_HBR )
    {
        fs = B_Fs_HBR ;
        ucIEC60958ChStat[0] |= B_AUD_NLPCM ;
    }

    ucIEC60958ChStat[3] = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT3) ;

    ucIEC60958ChStat[4] = (ucIEC60958ChStat[3] >> 4) & 0xF ;
    ucIEC60958ChStat[4] |= ((~fs) & 0xF)<<4 ;

    ucIEC60958ChStat[3] &= 3 ;
    ucIEC60958ChStat[3] <<= 4 ;
    ucIEC60958ChStat[3] |= fs & 0xF ;

    return TRUE ;
}

void
setCAT6023_HBROutput(BOOL HBR_SPDIF)
{
    BYTE uc ;
    SwitchHDMIRXBank(0) ;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_HWAMP_CTRL) ;

    if(HBR_SPDIF)
    {
        HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL, uc | B_HBR_SPDIF ) ;

        uc = HDMIRX_ReadI2C_Byte(REG_RX_FS_SET) ;
        uc &= ~0x30 ;
        uc |= 0x20 ;
        uc = HDMIRX_WriteI2C_Byte(REG_RX_FS_SET, uc) ;
        SetMUTE(~B_TRI_AUDIO, B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0) ;

    }
    else
    {
        HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL, uc | B_HBR_SPDIF ) ;
        SetMUTE(~B_TRI_AUDIO, B_TRI_SPDIF) ;

    }
}

void
setCAT6023_SPDIFOutput()
{
    BYTE uc ;
    SwitchHDMIRXBank(0) ;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_FS_SET) ;
    uc &= ~0x30 ;
    uc |= 0x20 ;
    uc = HDMIRX_WriteI2C_Byte(REG_RX_FS_SET, uc) ;
    SetMUTE(~B_TRI_AUDIO, B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0) ;
}

void
setCAT6023_I2SOutput(BYTE src_enable)
{
    SwitchHDMIRXBank(0) ;

    src_enable &= 0xF ;
    src_enable ^= 0xF ;
    SetMUTE(~B_TRI_AUDIO, B_TRI_SPDIF|src_enable) ;
}

#ifdef DEBUG
void
DumpCat6023Reg()
{
    int i,j ;
    BYTE ucData ;

    HDMIRX_PRINTF1(("       "));
    for( j = 0 ; j < 16 ; j++ )
    {
        HDMIRX_PRINTF1((" %02X",(int)j));
        if( (j == 3)||(j==7)||(j==11))
        {
            HDMIRX_PRINTF1(("  "));
        }
    }
    HDMIRX_PRINTF1(("\n        -----------------------------------------------------\n"));

    Switch_HDMIRX_Bank(0);

    for(i = 0 ; i < 0x100 ; i+=16 )
    {
        HDMIRX_PRINTF1(("[%3X]  ",(int)i));
        for( j = 0 ; j < 16 ; j++ )
        {
            ucData = HDMIRX_ReadI2C_Byte((BYTE)((i+j)&0xFF));
            HDMIRX_PRINTF1((" %02X",(int)ucData));
            if( (j == 3)||(j==7)||(j==11))
            {
                HDMIRX_PRINTF1((" -"));
            }
        }
        HDMIRX_PRINTF1(("\n"));
        if( (i % 0x40) == 0x30)
        {
            HDMIRX_PRINTF1(("        -----------------------------------------------------\n"));
        }
    }

    Switch_HDMIRX_Bank(1);
    for(i = 0x180; i < 0x200 ; i+=16 )
    {
        HDMIRX_PRINTF1(("[%3X]  ",(int)i));
        for( j = 0 ; j < 16 ; j++ )
        {
            ucData = HDMIRX_ReadI2C_Byte((BYTE)((i+j)&0xFF));
            HDMIRX_PRINTF1((" %02X",(int)ucData));
            if( (j == 3)||(j==7)||(j==11))
            {
                HDMIRX_PRINTF1((" -"));
            }
        }
        HDMIRX_PRINTF1(("\n"));
        if( (i % 0x40) == 0x30)
        {
            HDMIRX_PRINTF1(("        -----------------------------------------------------\n"));
        }

    }

    Switch_HDMIRX_Bank(0);
}
#endif
