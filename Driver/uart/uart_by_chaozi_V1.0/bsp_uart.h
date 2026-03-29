#ifndef __BSP_UART_H
#define __BSP_UART_H
#include "stm32f4xx_hal.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"


#ifndef u32
#define u32 uint32_t
#endif
#ifndef u16
#define u16 uint16_t
#endif
#ifndef u8
#define u8 uint8_t
#endif

#define U1_RX_SIZE  2048
#define U1_TX_SIZE  2048
#define U1_RX_MAX   256

#define U2_RX_SIZE  2048
#define U2_TX_SIZE  2048
#define U2_RX_MAX   256

#define U3_RX_SIZE  2048
#define U3_TX_SIZE  2048
#define U3_RX_MAX   256


#define NUM         10

//定义一次串口接受或者发送的缓冲区的其实指针和结束指针
typedef struct
{
    u8  *start;
    u8  *end;
}LCB;

//定义串口DMA状态枚举
typedef enum
{
    UART_STATE_IDLE = 0,    // 空闲/就绪 (包含初始化完成或传输结束)
    UART_STATE_BUSY = 1     // 忙碌 (正在DMA传输中)
}UartState;
//定义串口控制块结构体，包含接收和发送的相关信息
typedef struct
{
    u16                 RxCounter;              //当前接收缓冲区中数据的总长度
    u16                 RxMaxNum;               //每次DMA接收的最大数据长度
    UartState           RxDmaState;             //当前接收DMA的状态，IDLE表示空闲，BUSY表示正在接收 
    LCB                 RxLocation[NUM];        //接收缓冲区位置数组，包含多个缓冲区的起始和结束指针
    LCB                 *RxInPtr;               //接收缓冲区输入指针，指向当前DMA接收数据应该放入的缓冲区位置
    LCB                 *RxOutPtr;              //接收缓冲区输出指针，指向当前处理数据应该从哪个缓冲区位置读取
    LCB                 *RxEndPtr;              //接收缓冲区结束指针，指向接收缓冲区位置数组的最后一个元素，方便进行回绕处理

    u16                 TxCounter;              //当前发送缓冲区中数据的总长度
    UartState           TxDmaState;             //当前发送DMA的状态，IDLE表示空闲，BUSY表示正在发送
    LCB                 TxLocation[NUM];        //发送缓冲区位置数组，包含多个缓冲区的起始和结束指针
    LCB                 *TxInPtr;               //发送缓冲区输入指针，指向当前准备发送的数据应该放入的缓冲区位置
    LCB                 *TxOutPtr;              //发送缓冲区输出指针，指向当前正在发送的数据应该从哪个缓冲区位置读取
    LCB                 *TxEndPtr;              //发送缓冲区结束指针，指向发送缓冲区位置数组的最后一个元素，方便进行回绕处理
}UCB_t;

//定义串口设备结构体，包含指向UART句柄、DMA句柄、接收和发送缓冲区的指针，以及串口控制块
typedef struct _UartDevice{
    UART_HandleTypeDef *huart;              //指向串口外设句柄的指针，包含串口的寄存器地址和配置参数等信息
    DMA_HandleTypeDef *hdma_rx;             //指向串口接收DMA句柄的指针，包含DMA的寄存器地址和配置参数等信息
    DMA_HandleTypeDef *hdma_tx;             //指向串口发送DMA句柄的指针，包含DMA的寄存器地址和配置参数等信息     

    void (*WaitTxCpl)(struct _UartDevice *uartDevice); //等待发送完成的函数指针，指向一个函数，该函数接受一个指向串口设备结构体的指针作为参数，用于等待当前发送完成
    void (*WaitRxCpl)(struct _UartDevice *uartDevice); //等待接收完成的函数指针，指向一个函数，该函数接受一个指向串口设备结构体的指针作为参数，用于等待当前接收完成
    void (*SendData)(struct _UartDevice *uartDevice, u8 *pData, u16 len, u8 use_dma); //发送数据的函数指针，指向一个函数，该函数接受一个指向串口设备结构体的指针、一个指向要发送数据的指针和数据长度作为参数，用于将数据放入发送缓冲区准备发送

    void (*Flush)(struct _UartDevice *uartDevice); //刷新串口接收缓冲区的函数指针，指向一个函数，该函数接受一个指向串口设备结构体的指针作为参数，用于刷新当前接收缓冲区

    u8 *RxBuff;                             //指向串口接收数据缓冲区的指针，用于存放DMA接收到的数据
    u32 RxBufSize;                          //串口接收缓冲区的大小，单位字节，用于DMA接收时的长度配置
    u8 *TxBuff;                             //指向串口发送数据缓冲区的指针，用于存放准备发送的数据
    u32 TxBufSize;                          //串口发送缓冲区的大小，单位字节，用于DMA发送时的长度配置
    UCB_t UCB;                              //串口控制块，包含接收和发送的相关信息，如缓冲区指针、计数器和DMA状态等
}UartDevice_t; 



extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;

extern UartDevice_t Uart1;
extern UartDevice_t Uart2;
extern UartDevice_t Uart3;

#define UART_DEBUG  Uart1


/***************************************************************************************/
void UART_Init(void); //串口初始化函数，包含缓冲区初始化和接收开启
void UARTRX_Init(void); //串口接收初始化函数，包含接收缓冲区初始化和接收开启
void UARTTX_Init(void); //串口发送初始化函数，包含发送缓冲区初始化

void UART_RX_Handle_Common(UartDevice_t *uartDevice);//串口接收处理函数，用在中断处理函数中
void UART_Echo_Data_Process(UartDevice_t *RECEIVE_DEVICE, UartDevice_t *SEND_DEVICE);
                                                 //串口A接收到的数据用串口B回显

void UART_printf(UartDevice_t *uartDevice, char *format, ...);    //串口发送函数（阻塞）
void UART_printf_DMA(UartDevice_t *uartDevice, char *format, ...);//串口发送函数（DMA）

void UART_Flush(UartDevice_t *uartDevice); //串口刷新函数，丢弃当前接收缓冲区中的数据，重置接收输出指针

/***************************************************************************************/
static void UART_Packaging_Init(UartDevice_t *uartDevice);//初始化串口的缓冲区初始化和串口的接收开启
static void UART_RxPtrInit(UartDevice_t *uartDevice);     //初始化串口接收缓冲区指针
static void UART_TxPtrInit(UartDevice_t *uartDevice);     //初始化串口发送缓冲区指针
static void StartUartRev(UartDevice_t *uartDevice);       //开启串口接收
static void UART_DMA_TX_Start(UartDevice_t *uartDevice);                        //开启串口dma发送
static void UART_TX_Buf_DATA_Send(UartDevice_t *uartDevice, u8 *pData, u16 len);//填充发送数据至发送缓冲区
static void Uart_SendData(UartDevice_t *uartDevice, u8 *pData, u16 len, u8 use_dma); //发送数据函数，包含阻塞和DMA两种发送方式

void UART_Wait_Tx_Complete(UartDevice_t *uartDevice);//等待发送完成
void UART_Wait_Rx_Complete(UartDevice_t *uartDevice);//等待接收完成
/***************************************************************************************/
#endif

