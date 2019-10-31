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
 *   ril_socket.h 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   The file declares some API functions, which are related to socket
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
#ifndef __RIL_SOCKET_H__
#define __RIL_SOCKET_H__
#include "ql_type.h"
#include "ril.h"

#define CONNECTID_MAX       (5)
#define SOCKET_SEND_BUFFER_LENGTH  (512)

typedef enum
{
    SOCKET_SERVICE_TYPE_TCP = 0,   //TCP
    SOCKET_SERVICE_TYPE_UDP        //UDP  
}Enum_Socket_Service_type;

typedef enum
{
    SOCKET_ACCESS_MODE_BUFFER = 0,   //Buffer access mode
    SOCKET_ACCESS_MODE_DIRECT        //Direct push mode     
}Enum_Socket_Access_Mode;

typedef enum
{
    SOCKET_PROTOCOL_TYPE_IPV4 = 0,   //ipv4
    SOCKET_PROTOCOL_TYPE_IPV6         //ipv6  
}Enum_Socket_Protocol_Type;

typedef struct{
 u8 contextID;   // context ID, range is 1-3
 u8 connectID;  //  socket service index, range is 0-4
 Enum_Socket_Service_type service_type;//0: Start a TCP connection as a client,1: Start a UDP connection as a client                
 u8* address;  //IP address or domain name.
 u32 remote_port;  // The port of the remote server, range is 1-65535
 u32 local_port;   //The local port, range is 1-65535,if <local_port> is 0, then the local port will be assigned automatically,  else the local port is assigned as specified
 //Enum_Socket_Access_Mode access_mode;//0: Buffer access mode, 1: Direct push mode
 Enum_Socket_Protocol_Type protocol_type;//     0: IPv4,1: IPv6
}ST_Socket_Param_t;


/******************************************************************************
* Function:     RIL_SOC_QIOPEN
*  
* Description:
*              Start a socket service by AT+QIOPEN. The service type can be specified by the parameter <service_type>.
*The data access mode (buffer access mode, direct push access mode) can be specified by parameter <accessmode>. 
*The URC " +QIOPEN" indicates whether the socket service is started successfully.
*It is suggested to wait 40 seconds for the URC response as " +QIOPEN: <connectID>,<err>��. If connected fail, 
* host must execute AT+QICLOSE=<connectID> to close the socket and change the <socket_state> to initial.
*
* Parameters:    
*                   ST_Socket_Param_t:
*                   [in] 
*                     socket param,please refer to ST_Socket_Param_t.
*                   
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32  RIL_SOC_QIOPEN(ST_Socket_Param_t* socket_param_t);


/******************************************************************************
* Function:     RIL_SOC_QISEND
*  
* Description:
*                 The function sends data to a connected TCP/UDP socket.
*
* Parameters:    
*            connectID:
*                   [in] 
*                   socket service index, range is 0-4.
*        send_length:
*                   [in]
*                   the length of data to be sent, which cannot exceed 700
*        send_buffer:
*                    [in]
*                    send data
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_SOC_QISEND(u8 connectID, u32 send_length,u8* send_buffer); 

/******************************************************************************
* Function:     RIL_SOC_QISENDEX
*  
* Description:
*                 The function sends hex string to a connected TCP/UDP socket.
*
* Parameters:    
*            connectID:
*                   [in] 
*                   socket service index, range is 0-4.
*        send_length:
*                   [in]
*                   the length of data to be sent, which cannot exceed 700
*   send_hex_buffer:
*                    [in]
*                    send hex string
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_SOC_QISENDEX(u8 connectID, u32 send_length,u8* send_hex_buffer);


/******************************************************************************
* Function:     RIL_SOC_QICFG_FORMAT
*  
* Description:
*                 AT+QICFG can set the data format for sending and receiving.
*
* Parameters:    
*            send_format:
*                   [in] 
*                   ingore
*        recv_format:
*                   [in]
*                   0: text mode[default]
*                   1: hex mode
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_SOC_QICFG_FORMAT(bool send_format,bool recv_format);


/******************************************************************************
* Function:     RIL_SOC_QICLOSE
*  
* Description:
*                 The function close Socket Service.
*
* Parameters:    
*               connectID:
*                   [in] 
*                   socket service index, range is 0-4.
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_SOC_QICLOSE(u8 connectID);


#endif // __RIL_NETWORK_H__
