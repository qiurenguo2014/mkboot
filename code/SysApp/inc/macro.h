#ifndef _MACRO_H
#define _MACRO_H

typedef unsigned char Bool;
typedef unsigned char bool;
typedef unsigned char u8;
typedef unsigned char uint8;
typedef unsigned char uchar;

typedef unsigned short u16;
typedef unsigned short uint16;
typedef short int16;

typedef unsigned long u32;

typedef  unsigned char             UInt8;
typedef  unsigned short            UInt16;
typedef  signed short              Int16;
typedef  Int16                     rval_t;
typedef  unsigned long             UInt32;
typedef  UInt32		               Inst_t;

typedef short SETUP_T;

#define  GLOBAL


#define ARRAY_SIZE(m)  (sizeof(m)/sizeof((m)[0]))


#define MSB(x)	(((x)&0xff00)>>8)
#define LSB(x)	((x)&0xff)


#ifndef NULL
#define NULL  0
#endif

#ifndef FALSE
#define	FALSE 0
#endif

#ifndef	TRUE
#define TRUE  1
#endif

//System Define
#define  HEARBEAT_RATE   1000     /*Hz 心率*/
#define  GLOBAL
#define _GLOBAL_DEBUG_ENABLE      /*串口打印调试信息*/

#ifdef _GLOBAL_DEBUG_ENABLE
#define _GLOBAL_COMDBG_ENABLE     /*串口调试命令    */
#define _GLOBAL_VERSION_ENABLE    /*系统版本信息    */
#endif


//MCU Define

#if 0

	typedef unsigned char MCUTYPE;
	#define SYSTEM_OSC_FCLK 22148000uL    // Hz
	//#define SYSTEM_OSC_FCLK 24000000uL	// Hz
	#define _XDATA   xdata
	#define _CODE    code
	#define _ROM     code
#else
	typedef unsigned int  MCUTYPE;
	#define _XDATA
	#define _CODE
	#define _ROM     const
#endif


#endif

