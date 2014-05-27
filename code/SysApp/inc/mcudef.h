#ifndef _MCUDEF_H
#define _MCUDEF_H

#include "mk20d10.h" 
#include "flash.h"  


#define     __I     volatile const            /*!< defines 'read only' permissions      */
#define     __O     volatile                  /*!< defines 'write only' permissions     */
#define     __IO    volatile                  /*!< defines 'read / write' permissions   */


#define PAGE_SIZE                          0x00000800 /*   2KB */
#define FLASH_SIZE                         0x00040000 /* 256KB */
 
#define SYSTEM_BOOTLOADER_LENGTH		   0x00004000 /*  16KB*/
#define SYSTEM_USERAPP_LIMITED			   0x00040000 /* 256KB*/

#define ApplicationAddress                 0x00004000 /*  16KB*/





#endif

