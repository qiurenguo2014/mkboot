
#include "macro.h"
#include "bsp.h"
#include "common.h"
#include "ymodem.h"
#include "core_cmFunc.h"

pFunction Jump_To_Application;

uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum)
{
  uint32_t i = 0, res = 0;
  uint32_t val = 0;

  if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X'))
  {
    if (inputstr[2] == '\0')
    {
      return 0;
    }
    for (i = 2; i < 11; i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1; */
        res = 1;
        break;
      }
      if (ISVALIDHEX(inputstr[i]))
      {
        val = (val << 4) + CONVERTHEX(inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* over 8 digit hex --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }
  else /* max 10-digit decimal input */
  {
    for (i = 0;i < 11;i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1 */
        res = 1;
        break;
      }
      else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0))
      {
        val = val << 10;
        *intnum = val;
        res = 1;
        break;
      }
      else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0))
      {
        val = val << 20;
        *intnum = val;
        res = 1;
        break;
      }
      else if (ISVALIDDEC(inputstr[i]))
      {
        val = val * 10 + CONVERTDEC(inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* Over 10 digit decimal --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }

  return res;
}

void SerialPutString(u8 *s)
{
	while(*s != '\0')
		{
		UartSend(*s);
		s++;
		}
}

void GotoApiMask(void)
{
	pFunction Jump_To_Application;
	UInt32 jumpAddress = *(__IO u32*)(ApplicationAddress + 4);

	SerialPutString("\r\nGo to Api!\r\n");
	Jump_To_Application = (pFunction)jumpAddress;
	__set_MSP(*(__IO u32*) ApplicationAddress);
	Jump_To_Application();
}


u8 tab_1024[1024] ={0};
void SerialDownload(void)
{
	int32_t Size = 0;

	SerialPutString("\r\n请使用Ymodem协议传送Bin文件........(press 'a' to abort)\r\n");
	Size = Ymodem_Receive(&tab_1024[0]);
	if (Size > 0)
		{
		SerialPutString(file_name);
		GotoApiMask();
		}
	else if (Size == -1)
		{
		SerialPutString("\n\n\rThe image size is higher than the allowed space memory!\n\r");
		}
	else if (Size == -2)
		{
		SerialPutString("\n\n\rVerification failed!\n\r");
		}
	else if (Size == -3)
		{
		SerialPutString("\r\n\nAborted by user.\n\r");
		}
	else
		{
		SerialPutString("\n\rFailed to receive the file!\n\r");
		}
}
/**
  * @brief  power manager.
  * @param  None
  * @retval None
  */
#define POWER PTC
#define POWER_PORT PORTC
#define POWER_Pin 12
void BSP_PowerInit (void)
{
	SIM->SCGC5|=SIM_SCGC5_PORTC_MASK;
	POWER_PORT->PCR[POWER_Pin]&=~(PORT_PCR_MUX_MASK);    
	POWER_PORT->PCR[POWER_Pin]|=PORT_PCR_MUX(1); 
	POWER->PDDR |= (1<<POWER_Pin);
	POWER_PORT->PCR[(POWER_Pin)]&=~(PORT_PCR_PE_MASK); 
	POWER->PDOR |= (1<<POWER_Pin);
	POWER_PORT->PCR[POWER_Pin]&= ~PORT_PCR_ODE_MASK;
}
#define DELAY 10000000
int main(void)
{
	u8 key;
	u32 temp32;
	u32 delay=0;
	BSP_PowerInit ();
	Mk20d10Init();
	
	temp32 = *(u32*)(ApplicationAddress);

	while(1)
		{
		//if(temp32 == 0XFFFFFFFF || (GetButton() == 0))
		if(1)
			{
			SerialPutString("\r\n==============BootLoader System =========================\r\n");
			SerialPutString(" 版本:V1.01                                               \r\n");
			SerialPutString(" (PA2==0) (0X08004000==0XFFFFFFFF)                        \r\n");
			SerialPutString(" MK20K10 USART0 115200                                    \r\n");
			SerialPutString(" IAR Start Addr:0X08000000   API Start addr:0X08004000    \r\n");
			SerialPutString("按1进入升级--------------------------------------------- 1\r\n");
			SerialPutString("按2执行应用程序----------------------------------------- 2\r\n");
			SerialPutString("==========================================================\r\n");

			while(1)
				{
				if(GetUart(&key))
					{
					if(key == 0x31)
						SerialDownload();
					else if(key == 0x32)
						GotoApiMask();
					else{						
						SerialPutString("Invalid Number ! ==> The number should be either 1 or 2\r");
					}
					}
					delay ++;
					if(delay>DELAY){
						GotoApiMask();
					}
				}
				
			}
		else
			{
			GotoApiMask();
			}
		}
}


