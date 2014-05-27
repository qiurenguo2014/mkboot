
#include "common.h"
#include "flash.h"
#include "ymodem.h"

uint8_t file_name[FILE_NAME_LENGTH];
uint32_t FlashDestination = ApplicationAddress;
uint16_t PageSize = PAGE_SIZE;
uint32_t EraseCounter = 0x0;
uint32_t NbrOfPage = 0;
uint8_t FLASHStatus = FLASH_OK;
uint32_t RamSource;
extern uint8_t tab_1024[1024];


static  int32_t Receive_Byte (uint8_t *c, uint32_t timeout)
{
	u8 temp8;
	while(timeout-- > 0)
		{
		if(GetUart(&temp8) == 1)
			{
			*c = temp8; 
			return 0;
			}
		}
  return -1;
}


static uint32_t Send_Byte(uint8_t c)
{
  UartSend(c);
  return 0;
}


static int32_t Receive_Packet (uint8_t *data, int32_t *length, uint32_t timeout)
{
	uint16_t i, packet_size;
	uint8_t c;
	*length = 0;
	
	if (Receive_Byte(&c, timeout) != 0)
		return -1;

	switch (c)
		{
		case SOH:
			packet_size = PACKET_SIZE;
			break;
			
		case STX:
			packet_size = PACKET_1K_SIZE;
			break;
			
		case EOT:
			return 0;
			
		case CA:
			if ((Receive_Byte(&c, timeout) == 0) && (c == CA))
				{
				*length = -1;
				return 0;
				}
			else
				return -1;

		case ABORT1:
		case ABORT2:
			return 1;
			
		default:
			return -1;
		}
	
	*data = c;
	for (i = 1; i < (packet_size + PACKET_OVERHEAD); i ++)
		{
		if (Receive_Byte(data + i, timeout) != 0)
			return -1;
		}
	
	if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
		return -1;

	*length = packet_size;
	return 0;
}


int32_t Ymodem_Receive(uint8_t *buf)
{
	uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH], *file_ptr, *buf_ptr;
	int32_t i,j,packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;

	/* Initialize FlashDestination variable */
	FlashDestination = ApplicationAddress;

	for(session_done = 0, errors = 0, session_begin = 0; ;)
		{
		for(packets_received = 0, file_done = 0, buf_ptr = buf; ;)
			{		
			
			switch(Receive_Packet(packet_data, &packet_length, NAK_TIMEOUT))
				{	
				case 0:
					errors = 0;
					switch (packet_length)
						{
						/* Abort by sender */
						case - 1:
							Send_Byte(ACK);
							return 0;
						
						/* End of transmission */
						case 0:
							Send_Byte(ACK);
							file_done = 1;
							break;
						
						/* Normal packet */
						default:
							if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff))//数据包编号
								{
								Send_Byte(NAK);
								}
							else
								{
								if(packets_received == 0)/* 第一次传输文件属性  */
									{
									if(packet_data[PACKET_HEADER] != 0)/* Filename packet has valid data */
										{
										for(i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH);)
											file_name[i++] = *file_ptr++;
										file_name[i++] = '\0';/*接收文件名结束*/
										
										for(i = 0, file_ptr ++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH);)
											file_size[i++] = *file_ptr++;
										file_size[i++] = '\0';/*接收文件名大小结束*/
										
										Str2Int(file_size, &size);

										if(size > (FLASH_SIZE - SYSTEM_BOOTLOADER_LENGTH -1))//代码长度超过空间大小
											{
											Send_Byte(CA);
											Send_Byte(CA);
											return -1;
											}
										
										if ((size % PAGE_SIZE) != 0)//根据代码大小分配块
											NbrOfPage = (size / PAGE_SIZE) + 1;
										else
											NbrOfPage = size / PAGE_SIZE;
										
										for (EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_OK); EraseCounter++)
											{
											FLASHStatus = FLASH_EraseSector(FlashDestination/PageSize + EraseCounter);//擦除代码所需要块
											FLASHStatus = FLASH_OK;
											}

										Send_Byte(ACK);
										Send_Byte(CRC16);
										}
									else/* Filename packet is empty, end session */
										{
										Send_Byte(ACK);
										file_done = 1;
										session_done = 1;
										break;
										}
									}
								else/* Data packet */
									{
									memcpy(buf_ptr, packet_data + PACKET_HEADER, packet_length);
									RamSource = (uint32_t)buf;
									for (j = 0;(j < packet_length) && (FlashDestination <  ApplicationAddress + size);j += 4)
										{
										/* Program the data received into STM32F10x Flash */
										FLASH_WriteWord(FlashDestination, *(uint32_t*)RamSource);
										if (*(uint32_t*)FlashDestination != *(uint32_t*)RamSource)
											{
											/* End session */
											Send_Byte(CA);
											Send_Byte(CA);
											return -2;
											}
										FlashDestination += 4;
										RamSource += 4;
										}
									Send_Byte(ACK);
									}
								packets_received ++;
								session_begin = 1;//错误计数器
								}
							}
						break;
					
					case 1:
						Send_Byte(CA);
						Send_Byte(CA);
						return -3;
					
					default:
						if (session_begin > 0)
							{
							errors ++;
							}
						if (errors > MAX_ERRORS)
							{
							Send_Byte(CA);
							Send_Byte(CA);
							return 0;
							}
						Send_Byte(CRC16);
						break;
					}	
			if (file_done != 0)
				{
				break;
				}
			}
		if (session_done != 0)
			{
			break;
			}
		}
	
	return (int32_t)size;
}

