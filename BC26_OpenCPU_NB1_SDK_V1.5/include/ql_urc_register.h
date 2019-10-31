
#ifndef __QL_URC_REGISTER_H__
#define __QL_URC_REGISTER_H__

typedef void(*Callback_Urc_Handle)(u8* buffer,u32 length);

/******************************************************************************
* Function:     Ql_Socket_Recv_Register
*
* Description:
*               Register callback for receiving data.When the module receives the urc of "+QIURC: \"recv\"", call particular callback.
* uses need to register the callback at the starting up and wake up from deep sleep.
*
* Parameters:
*              callback_socket_recv:
*                       [out] Register a callback to process downlink data.

* Return:
*                QL_RET_OK,successfully.
*                QL_RET_ERR_PARAM, Parameter error, maybe callback is NULL.
******************************************************************************/
s32 Ql_Socket_Recv_Register(Callback_Urc_Handle callback_socket_recv);


/******************************************************************************
* Function:     Ql_Mqtt_Recv_Register
*
* Description:
*               Register callback for receiving data.When the module receives  the urc of "+QMTRECV:", call particular callback.
* uses need to register the callback at the starting up and wake up from deep sleep.
*
* Parameters:
*              Callback_Urc_Handle:
*                       [out] Register a callback to process downlink data.

* Return:
*                QL_RET_OK,successfully.
*                QL_RET_ERR_PARAM, Parameter error, maybe callback is NULL.
******************************************************************************/
s32 Ql_Mqtt_Recv_Register(Callback_Urc_Handle callback_mqtt_recv);


/******************************************************************************
* Function:     Ql_Lwm2m_Recv_Register
*
* Description:
*               Register callback for receiving data.When the module receives the urc of "+QLWDATARECV:", call particular callback.
* uses need to register the callback at the starting up and wake up from deep sleep.
*
* Parameters:
*              callback_lwm2m_recv:
*                       [out] Register a callback to process downlink data.

* Return:
*                QL_RET_OK,successfully.
*                QL_RET_ERR_PARAM, Parameter error, maybe callback is NULL.
******************************************************************************/
s32 Ql_Lwm2m_Recv_Register(Callback_Urc_Handle callback_lwm2m_recv);


/******************************************************************************
* Function:     Ql_Onenet_Req_Register
*
* Description:
*               Register callback for receiving data.When the module receives the urc of "+MIPLWRITE", call particular callback.
* uses need to register the callback at the starting up and wake up from deep sleep.
*
* Parameters:
*              callback_onenet_recv:
*                       [out] Register a callback to process downlink data.

* Return:
*                QL_RET_OK,successfully.
*                QL_RET_ERR_PARAM, Parameter error, maybe callback is NULL.
******************************************************************************/
s32 Ql_Onenet_Req_Register(Callback_Urc_Handle callback_onenet_req);


#endif 
