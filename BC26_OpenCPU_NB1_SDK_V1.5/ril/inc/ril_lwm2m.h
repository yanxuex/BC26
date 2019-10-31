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
 *   ril_lwm2m.h 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   The file declares some API functions, which are related to lwm2m
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
#ifndef __RIL_LwM2M_H__
#define __RIL_LwM2M_H__
#include "ql_type.h"

#define LWM2M_SEND_BUFFER_LENGTH  (512)

typedef enum
{
    LWM2M_ACCESS_MODE_DIRECT = 0,   //Direct push mode   
    LWM2M_ACCESS_MODE_BUFFER        // Buffer access mode 
}Enum_Lwm2m_Access_Mode;

typedef enum
{
    LWM2M_SEND_MODE_NON = 0,   //Send NON message
    LWM2M_SEND_MODE_CON        //Send CON message
}Enum_Lwm2m_Send_Mode;


typedef enum
{
    LWM2M_DATA_FORMAT_TEXT = 0,   //Text mode
    LWM2M_DATA_FORMAT_HEX         //Hex mode
}Enum_Lwm2m_Data_Format;


typedef struct{
 u32 obj_id;   // Object ID.
 u32 ins_id;   //  Instance ID.
 s32 res_num;   //Resources num.
 u32 res_id;   //Resorce ID
 u32 send_length;   //Length of data sent.
 u8* buffer;     //The data format depends on AT+QLWCFG's configure.
 Enum_Lwm2m_Send_Mode lwm2m_send_mode;   //send data mode
}ST_Lwm2m_Send_Param_t;

/******************************************************************************
* Function:     RIL_QLwM2M_Serv
*  
* Description:
*               Configure the IoT Platform Address and Port
*
* Parameters:    
*               ip_addr:
*                   [in] 
*                   The IoT platform ip address
*               port:
*                   [in]
*                   The IoT platform port.
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Serv(u8* ip_addr,u32 port);


/******************************************************************************
* Function:     RIL_QLwM2M_Conf
*  
* Description:
*               Configure the IoT Platform Parameters
*
* Parameters:    
*               endpoint:
*                   [in] 
*                 The device's endpoint name.
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Conf(u8* endpoint);


/******************************************************************************
* Function:     RIL_QLwM2M_Addobj
*  
* Description:
*                   Add a LwM2M Object
*
* Parameters:    
*               obj_id:
*                   [in] 
*                   Object id. The max object id number is 65535.
*               ins_id:
*                   [in] 
*                   Instance id.
*               res_num:
*                   [in] 
*                   Resources id number.
*               res_id:
*                   [in] 
*                   Resources id.



* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Addobj(u32 obj_id,u8 ins_id,u8 res_num,u8* res_id);


/******************************************************************************
* Function:     RIL_QLwM2M_Open
*  
* Description:
*                   Send a Register Request
*
* Parameters:    
*                 * lwm2m_access_mode:
*                   [in] 
*                   lwm2m access mode,please refer to Enum_Lwm2m_Access_Mode.
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Open(Enum_Lwm2m_Access_Mode lwm2m_access_mode);


/******************************************************************************
* Function:     RIL_QLwM2M_Update
*  
* Description:
*                  Send an Update Request
*
* Parameters:    
*                void
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Update(void);


/******************************************************************************
* Function:     RIL_QLwM2M_Cfg
*  
* Description:
*               Configure Optional Parameters
*
* Parameters:    
*               send_format:
*                   [in] 
*                   send data format
*               recv_format:
*                   [in] 
*                   receive data format
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
s32 RIL_QLwM2M_Cfg(Enum_Lwm2m_Data_Format send_format,Enum_Lwm2m_Data_Format recv_format);



/******************************************************************************
* Function:     RIL_QLwM2M_Send
*  
* Description:
*                 Send Data with Mode
*
* Parameters:    
*                   lwm2m_send_param_t:
*                   [in] 
*                   lwm2m send param,please refer to ST_Socket_Param_t.
*
*                RIL_AT_SUCCESS,send AT successfully. 
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Send(ST_Lwm2m_Send_Param_t *lwm2m_send_param_t);


/******************************************************************************
* Function:     RIL_QLwM2M_Close
*  
* Description:
*               Send a Deregister Request
*
* Parameters:    
*               void
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
s32 RIL_QLwM2M_Close(void);


/******************************************************************************
* Function:     RIL_QLwM2M_Delete
*  
* Description:
*               Delete the LwM2M Context
*
* Parameters:    
*               void
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
s32 RIL_QLwM2M_Delete(void);


#endif // __RIL_NETWORK_H__
