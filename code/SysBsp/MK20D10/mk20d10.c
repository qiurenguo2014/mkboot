
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


//��ʹ�õ�UART��ʼ���ṹ
//����: UART0_RX_PA1_TX_PA2: UART0 PA1������ΪTX  PA2������ΪRX
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
uint32_t UART_BaudRate; 	 //������
uint32_t UARTxMAP;			 //��ʼ���ṹ
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

	//�ҳ���Ӧ��UART�˿�
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

	//�ҳ� PORT�˿� ��ʹ��ʱ��
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

	//���ö�Ӧ����Ϊ����ģʽ
	UART_PORT->PCR[pUART_Map->UART_RX_Pin_Index] &= ~PORT_PCR_MUX_MASK;
	UART_PORT->PCR[pUART_Map->UART_RX_Pin_Index] |= PORT_PCR_MUX(pUART_Map->UART_Alt_Index);
	UART_PORT->PCR[pUART_Map->UART_TX_Pin_Index] &= ~PORT_PCR_MUX_MASK;
	UART_PORT->PCR[pUART_Map->UART_TX_Pin_Index] |= PORT_PCR_MUX(pUART_Map->UART_Alt_Index);
	
	//���ô���Ƶ��
	clock = SystemCoreClock/(((SIM->CLKDIV1&SIM_CLKDIV1_OUTDIV2_MASK)>>SIM_CLKDIV1_OUTDIV2_SHIFT)+1);
	if((uint32_t)UARTx == UART0_BASE || (uint32_t)UARTx == UART1_BASE)
		clock = SystemCoreClock; //UART0 UART1ʹ��CoreClock

	sbr = (uint16_t)((clock) / ((UART_InitStruct->UART_BaudRate) * 16));
	brfa = ((clock * 2) / (UART_InitStruct->UART_BaudRate) - (sbr * 32));
	UARTx->BDH |= ((sbr >> 8)&UART_BDH_SBR_MASK); //���ø�5λ������
	UARTx->BDL = (sbr & UART_BDL_SBR_MASK); //���õ�8λ����
	UARTx->C4 |= brfa & (UART_BDL_SBR_MASK >> 3); //����С��λ
	//����uart���ƼĴ�����ʵ�ֻ����İ�λ���书��
	UARTx->C2 &= ~(UART_C2_RE_MASK | UART_C2_TE_MASK);	 //��ֹ���ͽ���
	UARTx->C1 &= ~UART_C1_M_MASK;                      //��������λ��Ϊ8λ
	UARTx->C1 &= ~(UART_C1_PE_MASK);                   //����Ϊ����żУ��λ
	UARTx->S2 &= ~UART_S2_MSBF_MASK;                   //����Ϊ���λ���ȴ���
	//ʹ�ܽ������뷢����
	UARTx->C2 |= (UART_C2_RE_MASK | UART_C2_TE_MASK);	 //�������ݷ��ͽ���,�μ��ֲ�1221ҳ
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
//��Դ��ʼ��
//================================
//GPIO����
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

//�ж�����
typedef enum
{
  	GPIO_IT_DISABLE = 0x00,              //��ֹ�ⲿ�ж�
	GPIO_IT_DMA_RISING = 0x01,           //DMA�����ش���
  	GPIO_IT_DMA_FALLING = 0x02,          //DMA�½��ش���
	GPIO_IT_DMA_RASING_FALLING = 0x03,   //DMA�������½��ش���
	GPIO_IT_LOW = 0x08,                  //�߼�0״̬����
	GPIO_IT_RISING = 0x09,               //�����ش���
	GPIO_IT_FALLING = 0x0A,              //�½��ش���
	GPIO_IT_RISING_FALLING = 0x0B,       //�������½��ش���
	GPIO_IT_HIGH = 0x0C,                 //�߼�1����
}GPIO_IT_TypeDef;

typedef enum
{
	GPIO_Mode_IN_FLOATING = 0x04,     //��������
	GPIO_Mode_IPD = 0x05,             //��������
	GPIO_Mode_IPU = 0x06,             //��������
	GPIO_Mode_OOD = 0x07,             //��©���
	GPIO_Mode_OPP = 0x08,             //�������
}GPIO_Mode_TypeDef;

typedef struct
{
	uint16_t GPIO_Pin;                //����
	BitAction GPIO_InitState;         //��ʼ״̬
	GPIO_IT_TypeDef GPIO_IRQMode;     //GPIO�ж�״̬����
	GPIO_Mode_TypeDef GPIO_Mode;      //GPIOģʽ����
	GPIO_Type *GPIOx;                 //GPIO�˿ں�
}GPIO_InitTypeDef;

void GPIO_Init(GPIO_InitTypeDef* GPIO_InitStruct)
{
	GPIO_Type *GPIOx = NULL;
	PORT_Type *PORTx = NULL;
	
	GPIOx = GPIO_InitStruct->GPIOx;
	//���˿�ʱ��
	switch((uint32_t)GPIOx)
	{
		case PTA_BASE:PORTx=PORTA;SIM->SCGC5|=SIM_SCGC5_PORTA_MASK;break; //����PORTA��ʹ��ʱ�ӣ�������ǰ���ȿ���ʹ��ʱ�Ӳμ�k10�ֲ�268ҳ��
	  case PTB_BASE:PORTx=PORTB;SIM->SCGC5|=SIM_SCGC5_PORTB_MASK;break;	//����PORTB��ʹ��ʱ��
	  case PTC_BASE:PORTx=PORTC;SIM->SCGC5|=SIM_SCGC5_PORTC_MASK;break;	//����PORTC��ʹ��ʱ��
	  case PTD_BASE:PORTx=PORTD;SIM->SCGC5|=SIM_SCGC5_PORTD_MASK;break;	//����PORTD��ʹ��ʱ��
	  case PTE_BASE:PORTx=PORTE;SIM->SCGC5|=SIM_SCGC5_PORTE_MASK;break;	//����PORTE��ʹ��ʱ��
	  default : break;
	} 
	//����ΪGPIOģʽ
	PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&=~(PORT_PCR_MUX_MASK);    
	PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|=PORT_PCR_MUX(1); 
	//ȷ�������뻹�����
	if((GPIO_InitStruct->GPIO_Mode == GPIO_Mode_OOD) || (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_OPP))
	{
		//����GPIOx�ڵĵ�GPIO_Pin����Ϊ���
		GPIOx->PDDR |= (1<<(GPIO_InitStruct->GPIO_Pin));	
		//��Ϊ�����ʱ�رո����ŵ����������蹦��
	  PORTx->PCR[(GPIO_InitStruct->GPIO_Pin)]&=~(PORT_PCR_PE_MASK); 
		//�����ƽ����
		(Bit_SET == GPIO_InitStruct->GPIO_InitState)?(GPIOx->PDOR |= (1<<(GPIO_InitStruct->GPIO_Pin))):(GPIOx->PDOR &= ~(1<<(GPIO_InitStruct->GPIO_Pin)));
		//��©�����������
		if(GPIO_InitStruct->GPIO_Mode == GPIO_Mode_OOD)
		{
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|= PORT_PCR_ODE_MASK;
		}
		else if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_OPP)
		{
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&= ~PORT_PCR_ODE_MASK;
		}
	}
	//���������ģʽ
	else if ((GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IN_FLOATING) || (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPD) || (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPU))
	{
		//����GPIOx�ڵĵ�GPIO_Pin����Ϊ����
		GPIOx->PDDR &= ~(1<<(GPIO_InitStruct->GPIO_Pin));		
		if(GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IN_FLOATING)
		{
			//�ر�����������
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&=~PORT_PCR_PE_MASK; 	//����������DISABLE
		}
		else if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPD) //����
		{
			//������������
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|= PORT_PCR_PE_MASK; 	//����������ʹ��
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&= ~PORT_PCR_PS_MASK;
			
		}
		else if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPU) //����
		{
			//������������
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|= PORT_PCR_PE_MASK; 	//����������ʹ��
			PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|= PORT_PCR_PS_MASK;
		}
	}
	//�����ж�ģʽ
	PORTx->PCR[GPIO_InitStruct->GPIO_Pin]&=~PORT_PCR_IRQC_MASK;
	PORTx->PCR[GPIO_InitStruct->GPIO_Pin]|=PORT_PCR_IRQC(GPIO_InitStruct->GPIO_IRQMode);//�ⲿ�жϴ������� 
}


/* �������عܽ����� */
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

