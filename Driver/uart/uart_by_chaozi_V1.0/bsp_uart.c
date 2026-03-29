#include "bsp_uart.h"
#include "usart.h"
#include "main.h"

/*********************************创建串口设备********************************************************/
/****************************************************************************************************/
//设备组
UartDevice_t *UART_DEVICES[] = {
    &Uart1,
    &Uart2,
    &Uart3,
    NULL // Sentinel value
};

UartDevice_t Uart1 = {
    .huart = &huart1,
    .hdma_rx = &hdma_usart1_rx,
    .hdma_tx = &hdma_usart1_tx,
    .RxBuff = (u8[U1_RX_SIZE]){0},   // ！！！【C99复合字面量】！！直接在这里定义Rx缓冲区
    .RxBufSize = U1_RX_SIZE,
    .TxBuff = (u8[U1_TX_SIZE]){0},   // 直接在这里定义Tx缓冲区
    .TxBufSize = U1_TX_SIZE,
    .UCB = {
        .RxCounter = 0,
        .RxMaxNum = U1_RX_MAX,
        .RxDmaState = UART_STATE_IDLE,
        .RxInPtr = &Uart1.UCB.RxLocation[0],
        .RxOutPtr = &Uart1.UCB.RxLocation[0],
        .RxEndPtr = &Uart1.UCB.RxLocation[NUM - 1],

        .TxCounter = 0,
        .TxDmaState = UART_STATE_IDLE,
        .TxInPtr = &Uart1.UCB.TxLocation[0],
        .TxOutPtr = &Uart1.UCB.TxLocation[0],
        .TxEndPtr = &Uart1.UCB.TxLocation[NUM - 1]
    
    },

    .WaitTxCpl = UART_Wait_Tx_Complete,
    .WaitRxCpl = UART_Wait_Rx_Complete,
    .SendData = Uart_SendData,
    .Flush = UART_Flush

};

UartDevice_t Uart2 = {
    .huart = &huart2,
    .hdma_rx = &hdma_usart2_rx,
    .hdma_tx = &hdma_usart2_tx,
    .RxBuff = (u8[U2_RX_SIZE]){0},
    .RxBufSize = U2_RX_SIZE,
    .TxBuff = (u8[U2_TX_SIZE]){0},
    .TxBufSize = U2_TX_SIZE,

    .UCB = {
        .RxCounter = 0,
        .RxMaxNum = U2_RX_MAX,
        .RxDmaState = UART_STATE_IDLE,
        .RxInPtr = &Uart2.UCB.RxLocation[0],
        .RxOutPtr = &Uart2.UCB.RxLocation[0],
        .RxEndPtr = &Uart2.UCB.RxLocation[NUM - 1],

        .TxCounter = 0,
        .TxDmaState = UART_STATE_IDLE,
        .TxInPtr = &Uart2.UCB.TxLocation[0],
        .TxOutPtr = &Uart2.UCB.TxLocation[0],
        .TxEndPtr = &Uart2.UCB.TxLocation[NUM - 1]
    },
    .WaitRxCpl = UART_Wait_Rx_Complete,
    .WaitTxCpl = UART_Wait_Tx_Complete,
    .SendData = Uart_SendData,
    .Flush = UART_Flush
};

UartDevice_t Uart3 = {
    .huart = &huart3,
    .hdma_rx = &hdma_usart3_rx,
    .hdma_tx = &hdma_usart3_tx,
    .RxBuff = (u8[U3_RX_SIZE]){0},
    .RxBufSize = U3_RX_SIZE,
    .TxBuff = (u8[U3_TX_SIZE]){0},
    .TxBufSize = U3_TX_SIZE,
    .UCB = {
        .RxCounter = 0,
        .RxMaxNum = U3_RX_MAX,
        .RxDmaState = UART_STATE_IDLE,
        .RxInPtr = &Uart3.UCB.RxLocation[0],
        .RxOutPtr = &Uart3.UCB.RxLocation[0],
        .RxEndPtr = &Uart3.UCB.RxLocation[NUM - 1],

        .TxCounter = 0,
        .TxDmaState = UART_STATE_IDLE,
        .TxInPtr = &Uart3.UCB.TxLocation[0],
        .TxOutPtr = &Uart3.UCB.TxLocation[0],
        .TxEndPtr = &Uart3.UCB.TxLocation[NUM - 1]
    },
    .WaitRxCpl = UART_Wait_Rx_Complete,
    .WaitTxCpl = UART_Wait_Tx_Complete,
    .SendData = Uart_SendData,
    .Flush = UART_Flush
};




/*
*   UART_Init: 初始化所有串口设备
*   功能：1. 调用UARTRX_Init函数初始化所有串口接收相关的缓冲区指针和DMA状态，并启动DMA接收
*       2. 调用UARTTX_Init函数初始化所有串口发送相关的缓冲区指针和DMA状态，为后续发送数据做好准备
*/
void UART_Init(void)
{
    UARTRX_Init();
    UARTTX_Init();
}

/*
*   UART_Flush: 刷新串口接收缓冲区，重置接收输出指针
*   参数：uartDevice - 指向要刷新的串口设备结构体的指针
*   功能：1. 将串口设备的接收输出指针重置为接收输入指针的位置，表示所有已接收但未处理的数据都被丢弃，准备接收新的数据
*       2. 这个函数可以在需要丢弃当前接收缓冲区中的数据时调用，例如在通信异常或协议错误时，确保后续接收的数据能够正确处理
*/
void UART_Flush(UartDevice_t *uartDevice)
{
    uartDevice->UCB.RxOutPtr = uartDevice->UCB.RxInPtr;
}



/*
*   UART_Init: 初始化所有串口设备
*   功能：1. 遍历串口设备数组，对每个非空设备调用UART_Packaging_Init函数进行初始化
*    2. UART_Packaging_Init函数负责初始化串口设备的缓冲区指针、计数器和DMA状态，并启动DMA接收
*/
void UARTRX_Init(void)
{
    for(int i = 0; UART_DEVICES[i] != NULL; i++)
    {
        UART_Packaging_Init(UART_DEVICES[i]);
    }
}

/*
*   UARTTX_Init: 初始化所有串口设备的发送缓冲区指针
*   功能：1. 遍历串口设备数组，对每个非空设备调用UART_TxPtrInit函数进行发送缓冲区指针的初始化
*/
void UARTTX_Init(void)
{
    for(int i = 0; UART_DEVICES[i] != NULL; i++)
    {
        UART_TxPtrInit(UART_DEVICES[i]);
    }
}

/*
*   UART_Packaging_Init: 初始化串口的缓冲区初始化和串口的接收开启
*   参数：uartDevice - 指向要初始化的串口设备结构体的指针
*   功能：1. 初始化串口设备的接收缓冲区指针，设置为缓冲区的起始位置
*         2. 将接收计数器清零，设置DMA状态为IDLE
*         3. 启动串口的DMA接收，使能串口接收功能
*/
static void UART_Packaging_Init(UartDevice_t *uartDevice)
{
    UART_RxPtrInit(uartDevice);
    StartUartRev(uartDevice);
}

/*
*   UART_RxPtrInit: 初始化串口接收缓冲区指针
*   参数：uartDevice - 指向要初始化的串口设备结构体的指针
*   功能：1. 将接收缓冲区的输入指针、输出指针和结束指针初始化为缓冲区的起始位置和结束位置
*        2. 将接收计数器清零，设置DMA状态为IDLE
*/
static void UART_RxPtrInit(UartDevice_t *uartDevice)
{
    uartDevice->UCB.RxInPtr = &uartDevice->UCB.RxLocation[0];
    uartDevice->UCB.RxOutPtr = &uartDevice->UCB.RxLocation[0];
    uartDevice->UCB.RxEndPtr = &uartDevice->UCB.RxLocation[NUM - 1];

    uartDevice->UCB.RxInPtr->start = uartDevice->RxBuff;
    uartDevice->UCB.RxCounter = 0;
    uartDevice->UCB.RxDmaState = UART_STATE_IDLE;
}

/*
*   UART_TxPtrInit: 初始化串口发送缓冲区指针
*   参数：uartDevice - 指向要初始化的串口设备结构体的指针
*   功能：1. 将发送缓冲区的输入指针、输出指针和结束指针初始化为缓冲区的起始位置和结束位置
*       2. 将发送计数器清零，设置DMA状态为IDLE
*/
static void UART_TxPtrInit(UartDevice_t *uartDevice)
{
    uartDevice->UCB.TxInPtr = &uartDevice->UCB.TxLocation[0];
    uartDevice->UCB.TxOutPtr = &uartDevice->UCB.TxLocation[0];
    uartDevice->UCB.TxEndPtr = &uartDevice->UCB.TxLocation[NUM - 1];

    uartDevice->UCB.TxInPtr->start = uartDevice->TxBuff;
    uartDevice->UCB.TxCounter = 0;
    uartDevice->UCB.TxDmaState = UART_STATE_IDLE;
}

/*
*   UART_Wait_Tx_Complete: 等待当前发送完成
*   参数：uartDevice - 指向要等待发送完成的串口设备结构体的指针
*   功能：1. 轮询检查串口设备的发送DMA状态，直到状态变为IDLE，表示发送完成
*       2. 在等待过程中，可以添加适当的延时以避免过度占用CPU资源（主要是考虑freertos任务调度的情况）
*/
void UART_Wait_Tx_Complete(UartDevice_t *uartDevice)
{
    while(uartDevice->UCB.TxDmaState == UART_STATE_BUSY)
    {
        Sys_Delay(1);
    }
}

/*
*   UART_Wait_Rx_Complete: 等待当前接收完成
*   参数：uartDevice - 指向要等待接收完成的串口设备结构体的指针
*   功能：1. 轮询检查串口设备的接收DMA状态，直到状态变为IDLE，表示接收完成
*       2. 在等待过程中，可以添加适当的延时以避免过度占用CPU资源（主要是考虑freertos任务调度的情况）
*/
void UART_Wait_Rx_Complete(UartDevice_t *uartDevice)
{
    while(uartDevice->UCB.RxDmaState == UART_STATE_BUSY)
    {
        Sys_Delay(1);
    }
}

/*
*   UART_RX_Handle_Common: 串口接收完成后的通用处理函数
*   参数：uartDevice - 指向发生接收完成事件的串口设备结构体的指针
*   功能：1. 计算本次接收的数据长度，根据DMA的计数器获取剩余数据量来确定已接收的数据长度
*    2. 更新接收缓冲区的输入指针，指向新接收数据的起始位置
*   3. 更新接收计数器，记录当前接收缓冲区中数据的总长度
*   4. 根据接收缓冲区的剩余空间情况，决定下一次DMA接收的起始位置，如果剩余空间足够，则继续在当前缓冲区后面接收，否则回绕到缓冲区的起始位置
*/
void UART_RX_Handle_Common(UartDevice_t *uartDevice)
{
    uint32_t received_len = uartDevice->UCB.RxMaxNum - __HAL_DMA_GET_COUNTER(uartDevice->hdma_rx);
    
    if (received_len == 0) return;

    uartDevice->UCB.RxInPtr->end = uartDevice->UCB.RxInPtr->start + received_len - 1;

    uint8_t *next_start_ptr = uartDevice->UCB.RxInPtr->end + 1;
    
    uartDevice->UCB.RxInPtr++;
    if(uartDevice->UCB.RxInPtr > uartDevice->UCB.RxEndPtr)
    {
        uartDevice->UCB.RxInPtr = &uartDevice->UCB.RxLocation[0];
    }
    
    int32_t space_left = (uartDevice->RxBuff + uartDevice->RxBufSize) - next_start_ptr;
    if (space_left >= (int32_t)uartDevice->UCB.RxMaxNum)
    {
        uartDevice->UCB.RxInPtr->start = next_start_ptr;
        uartDevice->UCB.RxCounter = (uint32_t)(next_start_ptr - uartDevice->RxBuff);
    }
    else
    {
        uartDevice->UCB.RxInPtr->start = uartDevice->RxBuff;
        uartDevice->UCB.RxCounter = 0;
    }
}

/*********************************串口发送相关封装********************************************************/
/****************************************************************************************************/
/*
*   UART_printf: 基于轮询方式的串口发送函数，支持格式化字符串输出
*   参数：uartDevice - 指向要发送数据的串口设备结构体的指针
*         format - 格式化字符串，类似于printf的格式
*/
void UART_printf(UartDevice_t *uartDevice, char *format, ...)
{
    u16 i;
    va_list listdata;
    va_start(listdata, format);
    vsprintf((char *)uartDevice->TxBuff, format, listdata);
    va_end(listdata);

    for(i = 0; i < strlen((const char *)uartDevice->TxBuff); i++)
    {
        while(__HAL_UART_GET_FLAG(uartDevice->huart, UART_FLAG_TXE) != 1)
        {
            Sys_Delay(1);
        }
        HAL_UART_Transmit(uartDevice->huart, uartDevice->TxBuff + i, 1, HAL_MAX_DELAY);
    }
    while(__HAL_UART_GET_FLAG(uartDevice->huart, UART_FLAG_TC) != 1)
    {
        Sys_Delay(1);
    }
}

/*
*   UART_printf_DMA: 基于DMA方式的串口发送函数，支持格式化字符串输出
*   参数：uartDevice - 指向要发送数据的串口设备结构体的指针
*      format - 格式化字符串，类似于printf的格式
*/
void UART_printf_DMA(UartDevice_t *uartDevice, char *format, ...)
{
    va_list listdata;
    va_start(listdata, format);
    vsprintf((char *)uartDevice->TxBuff, format, listdata);
    va_end(listdata);

    UART_TX_Buf_DATA_Send(uartDevice, uartDevice->TxBuff, strlen((const char *)uartDevice->TxBuff));
    UART_DMA_TX_Start(uartDevice);
}

/*
*   Uart_SendData: 发送数据的函数，支持DMA和轮询两种方式
*   参数：uartDevice - 指向要发送数据的串口设备结构体的指针
*         pData - 指向要发送的数据的指针
*         len - 要发送的数据长度
*         use_dma - 是否使用DMA发送，1表示使用DMA，0表示使用轮询方式
*/
static void Uart_SendData(UartDevice_t *uartDevice, u8 *pData, u16 len, u8 use_dma)
{
    if(use_dma)
    {
        UART_TX_Buf_DATA_Send(uartDevice, pData, len);
        UART_DMA_TX_Start(uartDevice);
    }
    else
    {
        while(__HAL_UART_GET_FLAG(uartDevice->huart, UART_FLAG_TC) != 1)
        {
            Sys_Delay(1);
        }
        HAL_UART_Transmit(uartDevice->huart, pData, len, HAL_MAX_DELAY);
    }
}


/*
*   UART_Echo_Data_Process: 串口数据回显处理函数
*   参数：RECEIVE_DEVICE - 指向接收数据的串口设备结构体的指针
*    SEND_DEVICE - 指向发送数据的串口设备结构体的指针
*   功能：1. 检查接收缓冲区是否有未处理的数据，如果有则进行处理
*   2. 输出接收到的数据长度和内容到指定的发送串口设备
*/
void UART_Echo_Data_Process(UartDevice_t *RECEIVE_DEVICE, UartDevice_t *SEND_DEVICE)
{
    if(RECEIVE_DEVICE->UCB.RxOutPtr != RECEIVE_DEVICE->UCB.RxInPtr)
    {
        UART_printf(SEND_DEVICE, "本次接收了 %d 字节数据\r\n", 
                    RECEIVE_DEVICE->UCB.RxOutPtr->end - RECEIVE_DEVICE->UCB.RxOutPtr->start + 1);
        for(int i = 0; i < RECEIVE_DEVICE->UCB.RxOutPtr->end - RECEIVE_DEVICE->UCB.RxOutPtr->start + 1; i++)
        {
            UART_printf(SEND_DEVICE, "%c", RECEIVE_DEVICE->UCB.RxOutPtr->start[i]);
        }
        UART_printf(SEND_DEVICE, "\r\n");
        RECEIVE_DEVICE->UCB.RxOutPtr++;
        if(RECEIVE_DEVICE->UCB.RxOutPtr > RECEIVE_DEVICE->UCB.RxEndPtr)
        {
            RECEIVE_DEVICE->UCB.RxOutPtr = &RECEIVE_DEVICE->UCB.RxLocation[0];
        }
    }
}

/*
*   UART_DMA_TX_Start: 启动串口DMA发送
*   参数：uartDevice - 指向要发送数据的串口设备结构体的指针
*   功能：1. 检查发送缓冲区是否有待发送的数据，并且DMA当前处于空闲状态
*    2. 如果满足条件，则启用DMA传输，将数据从发送缓冲区传输到串口外设
*/
static void UART_DMA_TX_Start(UartDevice_t *uartDevice)
{
  if((uartDevice->UCB.TxOutPtr != uartDevice->UCB.TxInPtr) && (uartDevice->UCB.TxDmaState == UART_STATE_IDLE))
  {
    for(uint16_t i = 0; i < uartDevice->UCB.TxOutPtr->end - uartDevice->UCB.TxOutPtr->start + 1; i++)
    {
      uartDevice->UCB.TxOutPtr->start[i] == 0x00 ? UART_printf(&Uart1, "%02x ",0) : UART_printf(&Uart1, "%02x ",uartDevice->UCB.TxOutPtr->start[i]);
    }
    UART_printf(&Uart1, "\r\n");
    uartDevice->UCB.TxDmaState = UART_STATE_BUSY;
    __HAL_DMA_ENABLE(uartDevice->hdma_tx);
    HAL_UART_Transmit_DMA(uartDevice->huart, uartDevice->UCB.TxOutPtr->start, uartDevice->UCB.TxOutPtr->end - uartDevice->UCB.TxOutPtr->start + 1);
    uartDevice->UCB.TxOutPtr++;
    if(uartDevice->UCB.TxOutPtr > uartDevice->UCB.TxEndPtr)
    {
      uartDevice->UCB.TxOutPtr = &uartDevice->UCB.TxLocation[0];
    }
  }
}

/*
*   UART_TX_Buf_DATA_Send: 将数据放入发送缓冲区准备发送
*   参数：uartDevice - 指向要发送数据的串口设备结构体的指针
*   pData - 指向要发送的数据的指针
*   len - 要发送的数据长度
*   功能：1. 检查发送缓冲区剩余空间是否足够存放要发送的数据，如果足够则直接放入当前输入指针位置，否则回绕到缓冲区起始位置放入数据
*    2. 更新发送缓冲区的输入指针和发送计数器，记录当前发送缓冲区中数据的总长度
*/
static void UART_TX_Buf_DATA_Send(UartDevice_t *uartDevice, u8 *pData, u16 len)
{
  if(uartDevice->TxBufSize - uartDevice->UCB.TxCounter >= len)
  {
    uartDevice->UCB.TxInPtr->start = &uartDevice->TxBuff[uartDevice->UCB.TxCounter];
  }else
  {
    uartDevice->UCB.TxInPtr->start = &uartDevice->TxBuff[0];
    uartDevice->UCB.TxCounter = 0;
  }
  memcpy(uartDevice->UCB.TxInPtr->start, pData, len);
  uartDevice->UCB.TxCounter += len;
  uartDevice->UCB.TxInPtr->end = &uartDevice->UCB.TxInPtr->start[len - 1];
  uartDevice->UCB.TxInPtr++;
  if(uartDevice->UCB.TxInPtr > uartDevice->UCB.TxEndPtr)
  {
    uartDevice->UCB.TxInPtr = &uartDevice->UCB.TxLocation[0];
  }
}

/*
*   StartUartRev: 启动串口DMA接收
*   参数：uartDevice - 指向要启动接收的串口设备结构体的指针
*   功能：1. 启动串口的DMA接收，使能串口接收功能
*   2. 确保以标准块大小启动DMA接收，以避免与接收完成时的长度计算不匹配问题
*/
static void StartUartRev(UartDevice_t *uartDevice)
{
    // Ensure we start with the standard chunk size to avoid calculation mismatches
    HAL_UARTEx_ReceiveToIdle_DMA(uartDevice->huart, uartDevice->UCB.RxInPtr->start, uartDevice->UCB.RxMaxNum);
    __HAL_DMA_DISABLE_IT(uartDevice->hdma_rx, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(uartDevice->hdma_rx, DMA_IT_TC);
}
/*********************************HAL库底层的中断函数处理*******************************************************/
/****************************************************************************************************/

/*
*   HAL_UART_TxCpltCallback: 串口发送完成回调函数
*   参数：huart - 指向发生发送完成事件的UART句柄的指针
*   功能：1. 在发送完成回调中，首先检查传入的UART句柄是否为NULL，如果是则直接返回，避免空指针访问导致的错误
*   2. 遍历UART_DEVICES数组，找到与传入的UART句柄匹配的串口设备结构体
*   3. 如果找到匹配的设备，检查其发送DMA是否仍然启用，如果是则禁用DMA以确保传输完全结束
*   4. 将该设备的发送DMA状态设置为IDLE，表示发送完成，可以进行下一次发送操作
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{   
    if(huart == NULL) return;

    for (int i = 0; UART_DEVICES[i] != NULL; i++)
    {
        if (UART_DEVICES[i]->huart == huart)
        {
            if(UART_DEVICES[i]->hdma_tx->Instance->CR & DMA_SxCR_EN) {
                __HAL_DMA_DISABLE(UART_DEVICES[i]->hdma_tx);
            }
            UART_DEVICES[i]->UCB.TxDmaState = UART_STATE_IDLE; 
            return;
        }
    }
}

/*
*   HAL_UART_RxCpltCallback: 串口接收完成回调函数
*   参数：huart - 指向发生接收完成事件的UART句柄的指针
*   功能：1. 在接收完成回调中，首先检查传入的UART句柄是否为NULL，如果是则直接返回，避免空指针访问导致的错误
*   2. 遍历UART_DEVICES数组，找到与传入的UART句柄匹配的串口设备结构体
*   3. 如果找到匹配的设备，首先将该设备的接收DMA状态设置为IDLE，表示接收完成
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == NULL) return;

    for (int i = 0; UART_DEVICES[i] != NULL; i++)
    {
        if (UART_DEVICES[i]->huart == huart)
        {
            UART_DEVICES[i]->UCB.RxDmaState = UART_STATE_IDLE;
            UART_RX_Handle_Common(UART_DEVICES[i]);
            HAL_UARTEx_ReceiveToIdle_DMA(UART_DEVICES[i]->huart, UART_DEVICES[i]->UCB.RxInPtr->start, UART_DEVICES[i]->UCB.RxMaxNum);
            __HAL_DMA_DISABLE_IT(UART_DEVICES[i]->hdma_rx, DMA_IT_HT);
            __HAL_DMA_DISABLE_IT(UART_DEVICES[i]->hdma_rx, DMA_IT_TC);
            return;
        }
    }
}

/*
*   HAL_UARTEx_RxEventCallback: 串口接收事件回调函数（接收完成或接收空闲）
*   参数：huart - 指向发生接收事件的UART句柄的指针
*   Size - 本次接收事件中接收到的数据长度（字节数）
*   功能：1. 在接收事件回调中，首先检查传入的UART句柄是否为NULL，如果是则直接返回，避免空指针访问导致的错误
*   2. 遍历UART_DEVICES数组，找到与传入的UART句柄匹配的串口设备结构体
*   3. 如果找到匹配的设备，首先将该设备的接收DMA状态设置为IDLE，表示接收完成
*   4. 调用UART_RX_Handle_Common函数进行接收完成后的通用处理，更新接收缓冲区指针和计数器
*   5. 重新启动DMA接收，准备接收下一批数据，并且禁用DMA的半传输和传输完成中断，以避免在处理接收完成事件时再次触发中断
*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart == NULL) return;

    for (int i = 0; UART_DEVICES[i] != NULL; i++)
    {
        if (UART_DEVICES[i]->huart == huart)
        {
            UART_DEVICES[i]->UCB.RxDmaState = UART_STATE_IDLE;
            UART_RX_Handle_Common(UART_DEVICES[i]);
            HAL_UARTEx_ReceiveToIdle_DMA(UART_DEVICES[i]->huart, UART_DEVICES[i]->UCB.RxInPtr->start, UART_DEVICES[i]->UCB.RxMaxNum);
            __HAL_DMA_DISABLE_IT(UART_DEVICES[i]->hdma_rx, DMA_IT_HT);
            __HAL_DMA_DISABLE_IT(UART_DEVICES[i]->hdma_rx, DMA_IT_TC);
            return;
        }
    }
}
/***************************************************************************************/

