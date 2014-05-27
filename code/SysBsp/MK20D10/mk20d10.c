
#include "macro.h"
#include "bsp.h"

u8 Revdata,RevFlag;

static UartInit(u32 baudrate);

void Power_Init(void);
void Power_SetDevicePower(u8 isPowerOn);
static void Push_Button_Config(void);


GLOBAL void Mk20d10Init(void)
{
	SystemCoreClockUpdate();
	FLASH_Init();
	//Push_Button_Config();
	UartInit(115200);
}

/* Uart */
typedef struct {
uint32_t UART_Index: 3;
uint32_t UART_Alt_Index: 3;
uint32_t UART_GPIO_Index: 3;
uint32_t UART_TX_Pin_Index: 5;
uint32_t UART_RX_Pin_Index: 5;
uint32_t UART_CTS_Pin_Index: 5;
uint32_t UART_RTS_Pin_Index: 5;
}UART_MapTypeDef;


//可使用的UART初始化结构
//例如: UART0_RX_PA1_TX_PA2: UART0 PA1引脚作为TX  PA2引脚作为RX
#define UART0_RX_PA1_TX_PA2    (0x00004410U)
#define UART0_RX_PA14_TX_PA15  (0x00039E18U)
#define UART0_RX_PB16_TX_PB17  (0x00042258U)
#define UART0_RX_PD6_TX_PD7    (0x00018ED8U)
#define UART1_RX_PE0_TX_PE1    (0x00000319U)
#define UART1_RX_C3_TX_C4      (0x0000C899U)
#define UART2_RX_D2_TX_D3      (0x000086DAU)
#define UART3_RX_B10_TX_B11    (0x0002965BU)
#define UART3_RX_C16_TX_C17    (0x0004229BU)
#define UART3_RX_E4_TX_E5      (0x00010B1BU)
#define UART4_RX_E24_TX_E25    (0x0006331CU)
#define UART4_RX_C14_TX_C15    (0x00039E9CU)

typedef struct {
uint32_t UART_BaudRate; 	 //波特率
uint32_t UARTxMAP;			 //初始化结构
}UART_InitTypeDef;


void UART_Init(UART_InitTypeDef* UART_InitStruct)
{
	UART_Type* UARTx = NULL;
	PORT_Type *UART_PORT = NULL;
	uint16_t sbr;
	uint8_t brfa;
	uint32_t clock;
	UART_MapTypeDef *pUART_Map = NULL;
	pUART_Map = (UART_MapTypeDef*) & (UART_InitStruct->UARTxMAP);

	//找出对应的UART端口
	switch(pUART_Map->UART_Index) {
	case 0:
		SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
		UARTx = UART0;
		break;

	case 1:
		SIM->SCGC4 |= SIM_SCGC4_UART1_MASK;
		UARTx = UART1;
		break;

	case 2:
		SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
		UARTx = UART2;
		break;

	case 3:
		SIM->SCGC4 |= SIM_SCGC4_UART3_MASK;
		UARTx = UART3;
		break;

	case 4:
		SIM->SCGC1 |= SIM_SCGC1_UART4_MASK;
		UARTx = UART4;
		break;

	default:
		UARTx = NULL;
		break;
	}

	//找出 PORT端口 并使能时钟
	switch(pUART_Map->UART_GPIO_Index) {
	case 0:
		SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
		UART_PORT = PORTA;
		break;

	case 1:
		SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
		UART_PORT = PORTB;
		break;

	case 2:
		SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
		UART_PORT = PORTC;
		break;

	case 3:
		SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
		UART_PORT = PORTD;
		break;

	case 4:
		SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
		UART_PORT = PORTE;
		break;

	default:
		break;
	}

	//配置对应引脚为串口模式
	UART_PORT->PCR[pUART_Map->UART_RX_Pin_Index] &= ~PORT_PCR_MUX_MASK;
	UART_PORT->PCR[pUART_Map->UART_RX_Pin_Index] |= PORT_PCR_MUX(pUART_Map->UART_Alt_Index);
	UART_PORT->PCR[pUART_Map->UART_TX_Pin_Index] &= ~PORT_PCR_MUX_MASK;
	UART_PORT->PCR[pUART_Map->UART_TX_Pin_Index] |= PORT_PCR_MUX(pUART_Map->UART_Alt_Index);
	
	//配置传输频率
	clock = SystemCoreClock/(((SIM->CLKDIV1&SIM_CLKDIV1_OUTDIV2_MASK)>>SIM_CLKDIV1_OUTDIV2_SHIFT)+1);
	if((uint32_t)UARTx == UART0_BASE || (uint32_t)UARTx == UART1_BASE)
		clock = SystemCoreClock; //UART0 UART1使用CoreClock

	sbr = (uint16_t)((clock) / ((UART_InitStruct->UART_BaudRate) * 16));
	brfa = ((clock * 2) / (UART_InitStruct->UART_BaudRate) - (sbr * 32));
	UARTx->BDH |= ((sbr >> 8)&UART_BDH_SBR_MASK); //设置高5位的数据
	UARTx->BDL = (sbr & UART_BDL_SBR_MASK); //设置低8位数据
	UARTx->C4 |= brfa & (UART_BDL_SBR_MASK >> 3); //设置小数位
	//配置uart控制寄存器，实现基本的八位传输功能
	UARTx->C2 &= ~(UART_C2_RE_MASK | UART_C2_TE_MASK);	 //禁止发送接受
	UARTx->C1 &= ~UART_C1_M_MASK;                      //配置数据位数为8位
	UARTx->C1 &= ~(UART_C1_PE_MASK);                   //配置为无奇偶校检位
	UARTx->S2 &= ~UART_S2_MSBF_MASK;                   //配置为最低位优先传输
	//使能接收器与发送器
	UARTx->C2 |= (UART_C2_RE_MASK | UART_C2_TE_MASK);	 //开启数据发送接受,参见手册1221页
}

#define UART_PORT  UART3
#define UART_PIN   UART3_RX_E4_TX_E5

static UartInit(u32 baudrate)
{
	UART_InitTypeDef UART_InitStruct;
	
	UART_InitStruct.UART_BaudRate = 115200;
	UART_InitStruct.UARTxMAP      = UART_PIN;
	
	UART_Init(&UART_InitStruct);
}

GLOBAL void UartSend(u8 send_data)
{
	while(!(UART_PORT->S1 & UART_S1_TDRE_MASK));
	UART_PORT->D = send_data;
}

GLOBAL bool GetUart(u8 *rdate)
{
	if((UART_PORT->S1 & UART_S1_RDRF_MASK) != 0) 
		{
		*rdate = (UART_PORT->D);
		return 1;
		}
	return 0;
}




//================================
//电源初始化
//================================
//GPIO引脚
#define GPIO_Pin_0             (uint16_t)(0)
#define GPIO_Pin_1             (uint16_t)(1)
#define GPIO_Pin_2             (uint16_t)(2)
#define GPIO_Pin_3             (uint16_t)(3)
#define GPIO_Pin_4             (uint16_t)(4)
#define GPIO_Pin_5             (uint16_t)(5)
#define GPIO_Pin_6             (uint16_t)(6)
#define GPIO_Pin_7             (uint16_t)(7)
#define GPIO_Pin_8             (uint16_t)(8)
#define GPIO_Pin_9             (uint16_t)(9)
#define GPIO_Pin_10            (uint16_t)(10)
#define GPIO_Pin_11            (uint16_t)(11)
#define GPIO_Pin_12            (uint16_t)(12)
#define GPIO_Pin_13            (uint16_t)(13)
#define GPIO_Pin_14            (uint16_t)(14)
#define GPIO_Pin_15            (uint16_t)(15)
#define GPIO_Pin_16            (uint16_t)(16)
#define GPIO_Pin_17            (uint16_t)(17)
#define GPIO_Pin_18            (uint16_t)(18)
#define GPIO_Pin_19            (uint16_t)(19)
#define GPIO_Pin_20            (uint16_t)(20)
#define GPIO_Pin_21            (uint16_t)(21)
#define GPIO_Pin_22            (uint16_t)(22)
#define GPIO_Pin_23            (uint16_t)(23)
#define GPIO_Pin_24            (uint16_t)(24)
#define GPIO_Pin_25            (uint16_t)(25)
#define GPIO_Pin_26            (uint16_t)(26)
#define GPIO_Pin_27            (uint16_t)(27)
#define GPIO_Pin_28            (uint16_t)(28)
#define GPIO_Pin_29            (uint16_t)(29)
#define GPIO_Pin_30            (uint16_t)(30)
#define GPIO_Pin_31            (uint16_t)(31)

typedef enum
{ Bit_RESET = 0,
  Bit_SET
}BitAction;

//中断配置
typedef enum
{
  	GPIO_IT_DISABLE = 0x00,              //禁止外部中断
	GPIO_IT_DMA_RISING = 0x01,           //DMA上升沿触发
  	GPIO_IT_DMA_FALLING = 0x02,          //DMA下降沿触发
	GPIO_IT_DMA_RASING_FALLING = 0x03,   //DMA上升或下降沿触发
	GPIO_IT_LOW = 0x08,                  //逻辑0状态触发
	GPIO_IT_RISING = 0x09,               //上升沿触发
	GPIO_IT_FALLING = 0x0A,              //下降沿触发
	GPIO_IT_RISING_FALLING = 0x0B,       //上升或下降沿触发
	GPIO_IT_HIGH = 0x0C,                 //逻辑1触发
}GPIO_IT_TypeDef;

typedef enum
{
	GPIO_Mode_IN_FLOATING = 0x04,     //浮空输入
	GPIO_Mode_IPD = 0x05,             //下拉输入
	GPIO_Mode_IPU = 0x06,             //上拉输入
	GPIO_Mode_OOD = 0x07,             //开漏输出
	GPIO_Mode_OPP = 0x08,             //推挽输出
}GPIO_Mode_TypeDef;

typedef struct
{
	uint16_t GPIO_Pin;                //引脚
	BitAction GPIO_InitState;         //初始状态
	GPIO_IT_TypeDef GPIO_IRQMode;     //GPIO中断状态配置
	GPIO_Mode_TypeDef GPIO_Mode;      //GPIO模式配置
	GPIO_Type *GPIOx;                 //GPIO端口号
}GPIO_InitTypeDef;

void GPIO_Init(GPIO_InitTypeDef* GPIO_InitStruct)
{
	GPIO_Type *GPIOx = NULL;
	PORT_Type *PORTx = NULL;
	
	GPIOx = GPIO_InitStruct->GPIOx;
	//开端口时钟
	switch((uint32_t)GPIOx)
	{
		case PTA_BASE:PORTx=PORTA;SIM->SCGC5|=SIM_SCGC5_PORTA_MASK;break; //开启PORTA口使能时钟，在设置前首先开启使能时钟参见k10手册268页，
	  case PTB_BASE:PORTx=PORTB;SIM->SCGC5|=SIM_SCGC5_PORTB_MASK;break;	//开启PORTB口使能时钟
	  case PTC_BASE:PORTx=PORTC;SIM->SCGC5|=SIM_SCGC5_PORTC_MASK;break;	//开启PORTC口使能时钟
	  case PTD_BASE:PORTx=PORTD;SIM->SCGC5|=SIM_SCGC5_PORTD_MASK;break;	//开启PORTD口使能时钟
	  case PTE_BASE:PORTx=PORTE;SIM->SCGC5|=SIM_SCGC5_PORTE_MASK;break;	//开启PORTE口使能时钟
	  default : break;
	} 
	//设置为GPIO模式
	PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&=~(PORT_PCR_MUX_MASK);    
	PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|=PORT_PCR_MUX(1); 
	//确定是输入还是输出
	if((GPIO_InitStruct->GPIO_Mode == GPIO_Mode_OOD) || (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_OPP))
	{
		//配置GPIOx口的第GPIO_Pin引脚为输出
		GPIOx->PDDR |= (1<<(GPIO_InitStruct->GPIO_Pin));	
		//作为输出口时关闭该引脚的上下拉电阻功能
	  PORTx->PCR[(GPIO_InitStruct->GPIO_Pin)]&=~(PORT_PCR_PE_MASK); 
		//输出电平配置
		(Bit_SET == GPIO_InitStruct->GPIO_InitState)?(GPIOx->PDOR |= (1<<(GPIO_InitStruct->GPIO_Pin))):(GPIOx->PDOR &= ~(1<<(GPIO_InitStruct->GPIO_Pin)));
		//开漏或者推挽输出
		if(GPIO_InitStruct->GPIO_Mode == GPIO_Mode_OOD)
		{
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|= PORT_PCR_ODE_MASK;
		}
		else if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_OPP)
		{
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&= ~PORT_PCR_ODE_MASK;
		}
	}
	//如果是输入模式
	else if ((GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IN_FLOATING) || (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPD) || (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPU))
	{
		//配置GPIOx口的第GPIO_Pin引脚为输入
		GPIOx->PDDR &= ~(1<<(GPIO_InitStruct->GPIO_Pin));		
		if(GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IN_FLOATING)
		{
			//关闭上下拉电阻
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&=~PORT_PCR_PE_MASK; 	//上下拉电阻DISABLE
		}
		else if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPD) //下拉
		{
			//开启上拉电阻
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|= PORT_PCR_PE_MASK; 	//上下拉电阻使能
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&= ~PORT_PCR_PS_MASK;
			
		}
		else if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPU) //上拉
		{
			//开启上拉电阻
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|= PORT_PCR_PE_MASK; 	//上下拉电阻使能
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|= PORT_PCR_PS_MASK;
		}
	}
	//配置中断模式
	PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&=~PORT_PCR_IRQC_MASK;
	PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|=PORT_PCR_IRQC(GPIO_InitStruct->GPIO_IRQMode);//外部中断触发设置 
}


/* 进入下载管脚配置 */
#define BOOT_PORT			PTA
#define BOOT_PIN			GPIO_Pin_3

static void Push_Button_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;	
	
	GPIO_InitStruct.GPIOx          = BOOT_PORT;
	GPIO_InitStruct.GPIO_Pin       = BOOT_PIN;
	
	GPIO_InitStruct.GPIO_InitState = Bit_SET;
	GPIO_InitStruct.GPIO_IRQMode   = GPIO_IT_DISABLE;
	GPIO_InitStruct.GPIO_Mode      = GPIO_Mode_IPU;
	
	GPIO_Init(&GPIO_InitStruct);
}

GLOBAL bool GetButton(void)
{
	if(((BOOT_PORT->PDIR >> BOOT_PIN)& 0x01) != (uint32_t)Bit_RESET)	
		return 1;	
	else
		return 0;
}

