
#ifndef _COMMON_H
#define _COMMON_H

#include "macro.h"
#include "bsp.h"
#include "ymodem.h" 

#include "stdio.h"
#include "string.h"
#include <stdint.h>


typedef  void (*pFunction)(void);

#define IS_AF(c)  ((c >= 'A') && (c <= 'F'))
#define IS_af(c)  ((c >= 'a') && (c <= 'f'))
#define IS_09(c)  ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  (c - '0')

#define CONVERTHEX_alpha(c)  (IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
#define CONVERTHEX(c)        (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

uint32_t Str2Int(uint8_t *inputstr,int32_t *intnum);
extern void SerialPutString(u8 *s);
extern void Main_Menu(void);
extern void GotoApiMask(void);

#endif

