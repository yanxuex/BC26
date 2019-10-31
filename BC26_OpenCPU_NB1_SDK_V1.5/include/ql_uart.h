/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2019
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ql_uart.h 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *  Uart APIs defines.
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
 

#ifndef __QL_UART_H__
#define __QL_UART_H__
#include "ql_common.h"
#include "ql_system.h"
#include "ql_gpio.h"

typedef enum {
    UART_PORT0 = 0,
    UART_PORT1,
    UART_PORT2,
    UART_PORT_END
}Enum_SerialPort;

typedef enum {
    DB_5BIT = 0,
    DB_6BIT,
    DB_7BIT,
    DB_8BIT
} Enum_DataBits;

typedef enum {
    SB_ONE = 1,
    SB_TWO
} Enum_StopBits;

typedef enum {
    PB_NONE = 0,
    PB_ODD,
    PB_EVEN
} Enum_ParityBits;

typedef enum {
    FC_NONE=1,  // None Flow Control
    FC_HW,      // Hardware Flow Control 
    FC_SW       // Software Flow Control
} Enum_FlowCtrl;

typedef struct {
    u32                 baudrate; 
    Enum_DataBits       dataBits;
    Enum_StopBits       stopBits;
    Enum_ParityBits     parity;
    Enum_FlowCtrl       flowCtrl;
}ST_UARTDCB;

typedef enum {
    EVENT_UART_TRANSACTION_ERROR = 0,
    EVENT_UART_READY_TO_READ,
    EVENT_UART_READY_TO_WRITE,
    EVENT_UART_FE_IND,
    EVENT_UART_RI_IND,
    EVENT_UART_DCD_IND,
    EVENT_UART_DTR_IND
} Enum_UARTEventType;

typedef enum {
    UART_RX_BUF_RECEIVE_BYTES,  // This function queries receive bytes  in the RX buffer.
    UART_TX_BUF_SEND_SPACES,     // This function queries available space in the  TX buffer.
    UART_TX_BUF_SEND_OUT        // This function queries data has all been sent out  in the TX buffer . 485 protocol may be need.
}Enum_UARTOption;


/*****************************************************************
* Function:     Ql_UART_Register 
* 
* Description:
*               This function registers the callback function for the 
*               specified serial port. 
*               UART callback function used for receiving the UART notification from CORE.
*
* Parameters:
*               [in]port:   
*                       Port name                            
*               [in]callback_uart:     
*                       The pointer of the UART call back function.
*               [in]event:
*                       indication the event type of UART call back, please reference Enum_UARTEventType.
*               [in]pinLevel:
*                       if the event type is EVENT_UART_RI_IND or EVENT_UART_DCD_IND or EVENT_UART_DTR_IND
*                       the level indication the relate pin's current level otherwise this parameter has no meaning, just ignore it.
*               [in]customizePara: 
*                       Customize parameter, if not use just set to NULL.    
*
* Return:        
*               QL_RET_OK indicates success; otherwise failure.
*
*****************************************************************/
typedef void (*CallBack_UART_Notify)( Enum_SerialPort port, Enum_UARTEventType event, bool pinLevel,void *customizePara);
s32 Ql_UART_Register(Enum_SerialPort port, CallBack_UART_Notify callback_uart,void * customizePara);


/*****************************************************************
* Function:     Ql_UART_Open 
* 
* Description:
*               This function opens a specified UART port with the specified 
*               flow control mode. 
*               Which task call this function, which task will own the specified UART port.
*
* Parameters:
*               port:       
*                   [in]Port name
*               baudrate:      
*                   [in]The baud rate of the UART to be opened
*                       for the physical the baud rate support 75,150,300,600,1200,2400,4800,7200,
*                       9600,14400,19200,28800,38400,57600,115200,230400,460800
*                       for the UART_PORT1 support auto-baud rate, when the baud set to 0.
*               flowCtrl:   
*                   [in] one value of Enum_FlowCtrl, used for the physical UART's flow control.
*                        the hardware flow control(FC_HW) only supported on the UART_PORT1,
*					     other ports not support hardware flow control.
*
* Return:        
*               QL_RET_OK indicates success; otherwise failure.       
*
*****************************************************************/
s32 Ql_UART_Open(Enum_SerialPort port,u32 baudrate, Enum_FlowCtrl flowCtrl);


/*****************************************************************
* Function:     Ql_UART_OpenEx 
* 
* Description:
*               This function opens a specified UART port with the 
*               specified DCB parameters.
*               Which task call this function, which task will own the specified UART port.
*
* Parameters:
*               [in]port:   
*                       Port name
*               [in]dcb:   
*                       The point to the UART dcb struct. 
*                       Include baud rate,data bits,stop bits,parity,and flow control
*                       only physical UART1(UART_PORT1) have the hardware flow control
*
* Return:        
*           QL_RET_OK, this function executed successfully.
*		    QL_RET_ERR_PARAM, the serial port does not register the corresponding callback function.
*		    QL_RET_ERR_UART_BUSY, the serial port has being opend already.
*		    QL_RET_ERR_INVALID_PARAMETER, parameter error.
*		    QL_RET_ERR_INVALID_PORT, the serial port is invaild, not one value of Enum_SerialPort.
*****************************************************************/
s32 Ql_UART_OpenEx(Enum_SerialPort port, ST_UARTDCB *dcb);



/*****************************************************************
* Function:     Ql_UART_Write 
* 
* Description:
*               This function is used to send data to the specified UART port. 
*
*               When the number of bytes actually write data is less than that to send,
*               Application should stop sending data, and application will receive an event 
*               EVENT_UART_READY_TO_WRITE later. 
*               After receiving this Event Application can continue to send data, 
*               and previously unsent data should be resend.
*  
* Parameters:
*               [in] port:
*                       Port name
*               [in]data:
*                       Pointer to data to be send
*               [in]writeLen:
*                       The length of the data to be written
*
* Return:        
*               If >= 0, indicates success, and the return value 
*                        is the length of actual write data length
*               If <  0, indicates failure to read      
*
*****************************************************************/
s32 Ql_UART_Write(Enum_SerialPort port, u8* data, u32 writeLen );


/*****************************************************************
* Function:     Ql_UART_Read 
* 
* Description:
*               This function read data from the specified UART port.
*               When the UART callback is called, and the message type is 
*               EVENT_UART_READ_TO_READ,
*               user need read all of the data in the data buffer 
*               which means the real read len is less than the to be read len,
*               otherwise later the UART data is coming, there will be no such event to notify user get data.
*
* Parameters:
*               [in]port:
*                        Port name
*               [out]data:
*                       Point to the read data
*               [in]readLen:
*			    then length of data want to read
*
* Return:        
*               If >= 0, indicates success, and the return value 
*                        is the length of actual read data length
*               If <  0, indicates failure to read
*
*****************************************************************/
s32 Ql_UART_Read(Enum_SerialPort port, u8* data, u32 readLen);


/*****************************************************************
* Function:     Ql_UART_Close 
* 
* Description:
*               This function close a specified UART port.
* Parameters:
*               [in] port:
*                      Port name
* Return:        
*             NONE  
*
*****************************************************************/
void Ql_UART_Close(Enum_SerialPort port);


/*****************************************************************
* Function:     Ql_UART_GetOption 
* 
* Description:
*               This function used for getting the UART option. such as send or receive bytes in the buffer, 
*  Judging that TX data has been sent out(If you use 485 protocol.Judge when to switch pin level by this option).
* Parameters:
*              [in]port:
*                       Port name
*              [in]opt:
*                       one value of Enum_UARTOption
*
* Return:  opt as follows,      
*        UART_RX_BUF_RECEIVE_BYTES: return a value of receve bytes in the RX buffer.
*        UART_TX_BUF_SEND_SPACES : return a value of available space in the TX buffer
*        UART_TX_BUF_SEND_OUT: TRUE,Data has been sent out.  FALSE,Data is still being sent.
*
*****************************************************************/
s32 Ql_UART_GetOption(Enum_SerialPort port,Enum_UARTOption opt);

/*****************************************************************
* Function:  Ql_RS485_Open 
* 
* Description:
*              Open the 485 Control pin,  when a specific uart port start data and send out all the data, the control pin will automatically change the level status.
* Parameters:
*              [in]port:
*                       Port name
*              [in]pinname:
*                       one value of Enum_PinName
*              [in]level:
*                       one value of Enum_PinLevel.default gpio level.
*               
* Return:        
*               QL_RET_OK indicates success; otherwise failure.   
*
*Note:
*               Before start write data, the control pins will be set high level automatically. When all the data is sent out, 
*         it will be switched to the low level automatically.
*
*****************************************************************/
s32 Ql_RS485_Open(Enum_SerialPort port,Enum_PinName pinname,Enum_PinLevel level);

/*****************************************************************
* Function:     Ql_RS485_Close 
* 
* Description:
*             Release 485 control pin which a specific uart port.
* Parameters:
*              [in]port:
*                       Port name
*              [in]pinname:
*                       one value of Enum_PinName
*
* Return:        
*               QL_RET_OK indicates success; otherwise failure.   
*
*****************************************************************/
s32 Ql_RS485_Close(Enum_SerialPort port,Enum_PinName pinname);

#endif  // End-of __QL_UART_H__

