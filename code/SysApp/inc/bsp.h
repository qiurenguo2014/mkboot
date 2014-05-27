#ifndef _BSP_H
#define _BSP_H

#include "mcudef.h"

GLOBAL void Mk20d10Init(void);
GLOBAL void UartSend(u8 send_data);
GLOBAL bool GetUart(u8 *rdate);
GLOBAL bool GetButton(void);


extern u8 Revdata,RevFlag;

#endif

