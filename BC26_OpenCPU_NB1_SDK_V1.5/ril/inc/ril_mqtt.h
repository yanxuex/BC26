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
 *   ril_mqtt.h
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   The module implements MQTT related APIs.
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
#ifndef __RIL_MQTT_H__
#define __RIL_MQTT_H__

#include "ql_type.h"
#include "ril.h"

#define RIL_MQTT_LENGTH_MAX   (1400) //Maximum length of mqtt AT command.
#define RIL_MQTT_TOPIC_MAX    (256+2)//Maximum length of single topic information.
#define RIL_MQTT_INFO_MAX     (1024) //Maximum length of  total topic information.


//The QoS level at which the client wants to publish the messages.
typedef enum{
    QOS0_AT_MOST_ONECE,//At most once
    QOS1_AT_LEASET_ONCE,// At least once
    QOS2_AT_EXACTLY_ONECE// Exactly once
}Enum_Qos;

//MQTT socket identifier. The range is 0-5.
typedef enum{
    ConnectID_0,
    ConnectID_1,
    ConnectID_2,
    ConnectID_3,
    ConnectID_4,
    ConnectID_5
}Enum_ConnectID;

typedef struct{
    u8* topic[MQTT_MAX_TOPIC];
    Enum_Qos qos[MQTT_MAX_TOPIC];
    u8 count;
}ST_MQTT_topic_info_t;

/******************************************************************************
* Function:     RIL_MQTT_QMTCFG_Ali
*
* Description:
*               Configure  device information for  AliCloud
*
* Parameters:
*               connectID:
*                   [in]
*                    MQTT socket identifier. The range is 0-5.
*               product_key:
*                   [in]
*                    Product key issued by AliCloud.
*               device_name:
*                   [in]
*                    Device name issued by AliCloud.
*               device_secret:
*                   [in]
*                   Device secret key issued by AliCloud.

* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_MQTT_QMTCFG_Ali( Enum_ConnectID connectID,u8* product_key,u8* device_name,u8* device_secret);

/******************************************************************************
* Function:     RIL_MQTT_QMTOPEN
*
* Description:
*                The function is used to open a network for MQTT client.
*
* Parameters:
*               connectID:
*                   [in]
*                   MQTT socket identifier. The range is 0-5.
*               hostName:
*                   [in]
*                   The address of the server. It could be an IP address or a domain name. The maximum size is 100 bytes
*               port:
*                   [in]
*                    The port of the server. The range is 1-65535.

* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_MQTT_QMTOPEN(Enum_ConnectID connectID, u8* hostName, u32 port);

/******************************************************************************
* Function:     RIL_MQTT_QMTCONN
*
* Description:
*               The function is used requests a connection to MQTT server.
*
* Parameters:
*               connectID:
*                   [in]
*                   MQTT socket identifier. The range is 0-5.
*               clientID:
*                   [in]
*                   The client identifier string.
*               username:
*                   [in]
*                   User name of the client. It can be used for authentication.
*               password:
*                   [in]
*                    Password corresponding to the user name of the client.
*               It can be used for authentication.
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_MQTT_QMTCONN(Enum_ConnectID connectID, u8* clientID, u8* username, u8* password);

/******************************************************************************
* Function:     RIL_MQTT_QMTSUB
*
* Description:
*               The function is used to subscribe to one or more topics
*
* Parameters:
*               connectID:
*                   [in]
*                   MQTT socket identifier. The range is 0-5.
*               msgId:
*                   [in]
*                   Message identifier of packet. The range is 1-65535.
*               mqtt_topic_info_t:
*                   [in]
*                   mqtt topic information, please refer to ST_MQTT_topic_info_t.
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_MQTT_QMTSUB(Enum_ConnectID connectID, u32 msgId, ST_MQTT_topic_info_t* mqtt_topic_info_t);

/******************************************************************************
* Function:     RIL_MQTT_QMTPUB
*
* Description:
*               The function is used to publish messages to a server.
*
* Parameters:
*               connectID:
*                   [in]
*                   MQTT socket identifier. The range is 0-5.
*               msgId:
*                   [in]
*                   Message identifier of packet. The range is 1-65535.
*               qos:
*                   [in]
*                   The QoS level at which the client wants to publish the messages.
*               retain:
*                   [in]
*                   Whether or not the server will retain the message after it has been delivered to the current subscribers
*               topic:
*                   [in]
*                   Topic that needs to be published. Maximum length is 255 bytes.
*               message:
*                   [in]
*                   Message that needs to be published. Maximum length is 700 bytes.
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_MQTT_QMTPUB(Enum_ConnectID connectID, u32 msgId, Enum_Qos qos, u8 retain, u8* topic, u8* message);

/******************************************************************************
* Function:     RIL_MQTT_QMTUNS
*
* Description:
*               The function is used to unsubscribe from one or more topics
*
* Parameters:
*               connectID:
*                   [in]
*                   MQTT socket identifier. The range is 0-5.
*               msgId:
*                   [in]
*                   Message identifier of packet. The range is 1-65535.
*              mqtt_topic_info_t:
*                   [in]
*                   mqtt topic information, please refer to ST_MQTT_topic_info_t.
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_MQTT_QMTUNS(Enum_ConnectID connectID, u32 msgId,ST_MQTT_topic_info_t* mqtt_topic_info_t);

/******************************************************************************
* Function:     RIL_MQTT_QMTCLOSE
*
* Description:
*               The function is used to close a network for MQTT client.
*
* Parameters:
*               connectID:
*                   [in]
*                   MQTT socket identifier. The range is 0-5.
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_MQTT_QMTCLOSE(Enum_ConnectID connectID);

/******************************************************************************
* Function:     RIL_MQTT_QMTDISC
*
* Description:
*               The function is used when a client requests a disconnection from MQTT server.
*
* Parameters:
*               connectID:
*                   [in]
*                   MQTT socket identifier. The range is 0-5.
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_MQTT_QMTDISC(Enum_ConnectID connectID);

#endif //__RIL_MQTT_H__

