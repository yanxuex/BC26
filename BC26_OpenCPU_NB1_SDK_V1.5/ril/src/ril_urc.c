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
 *   ril_urc.c 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   The module handles URC in RIL.
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
#include "custom_feature_def.h"
#include "ql_stdlib.h"
#include "ril.h "
#include "ril_util.h"
#include "ql_system.h"
#include "ql_common.h"
#include "ql_uart.h"
#include "ql_type.h"
#include "ril_onenet.h"
#include "ril_lwm2m.h"
#include "ril_socket.h"
#include "ril_system.h"
#include "ril_dfota.h"
#include "ril_network.h"

#ifdef __OCPU_RIL_SUPPORT__


#define RIL_URC_DEBUG_ENABLE 0
#if RIL_URC_DEBUG_ENABLE > 0
#define RIL_URC_DEBUG_PORT  UART_PORT0
static char DBG_Buffer[1024];
#define RIL_URC_DEBUG(BUF,...)  QL_TRACE_LOG(RIL_URC_DEBUG_PORT,BUF,1024,__VA_ARGS__)
#else
#define RIL_URC_DEBUG(BUF,...) 
#endif

/************************************************************************/
/* Definition for URC receive task id.                                  */
/************************************************************************/
#define URC_RCV_TASK_ID  main_task_id

/************************************************************************/
/* Definition for URC param.                                  */
/************************************************************************/
Socket_Urc_Param_t socket_urc_param= {0,0};  //tcp or udp

Lwm2m_Urc_Param_t  lwm2m_urc_param = {0,0,0,0};//lwm2m

Onenet_Urc_Param_t  onenet_urc_param = {0,  //ref
										0,  //evtid
										0,  //exten
										0,  //ackid
										0,  //msgid
										0,  //objid
										0,  //insid
										0,  //resid
										0,  //observe_flag
										0,  //len
										0,  //flag
										0,  //index
										0,  //value_type
										{0},//buffer
										0,  //remain_lifetime
										0}; //access mode								
MQTT_Urc_Param_t   mqtt_urc_param =  {0,0,255,255,255,255,{255}}; //mqtt

/************************************************************************/
/* Declarations for URC handler.                                        */
/************************************************************************/
static void OnURCHandler_Network(const char* strURC, void* reserved);
static void OnURCHandler_SIM(const char* strURC, void* reserved);
static void OnURCHandler_CFUN(const char* strURC, void* reserved);
static void OnURCHandler_InitStat(const char* strURC, void* reserved);

/********************PSM***********************************************/
static void OnURCHandler_NbiotEvent(const char* strURC, void* reserved);

/*************** ***DFOTA********************************************/
static void OnURCHandler_DFOTA_Hander  (const char* strURC, void* reserved);

/*************** ***TCP &&UDP********************************************/
static void OnURCHandler_QIURC_OPEN(const char* strURC, void* reserved);
static void OnURCHandler_SOCKET_QIURC(const char* strURC, void* reserved);

/*************** ***LWM2M********************************************/
static void OnURCHandler_LwM2M_OBSERVE(const char* strURC, void* reserved);

/*************** ***ONENET********************************************/
static void OnURCHandler_ONENET_EVENT(const char* strURC, void* reserved);
static void OnURCHandler_ONENET_OBSERVER(const char* strURC, void* reserved);
static void OnURCHandler_ONENET_DISCOVER(const char* strURC, void* reserved);
static void OnURCHandler_ONENET_READ(const char* strURC, void* reserved);

/*************** ***MQTT********************************************/
static void OnURCHandler_MQTT_OPEN(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_CONN(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_SUB(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_PUB(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_TUNS(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_STATE(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_CLOSE(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_DISC(const char* strURC, void* reserved);


/************************************************************************/
/* Customer ATC URC callback                                          */
/************************************************************************/

/******************************************************************************
* Definitions for URCs and the handler.
* -------------------------------------
* -------------------------------------
* In OpenCPU RIL, URC contains two types: system URC and AT URC.
*   - System URCs indicate the various status of module.
*   - AT URC serves some specific AT command. 
*     For example, some AT command responses as below:
*         AT+QABC     (send at command)
*
*         OK          (response1)
*         +QABC:xxx   (response2) --> this is the final result which is reported by URC.
*     When calling Ql_RIL_SendATCmd() to send such AT command, the return value of 
*     Ql_RIL_SendATCmd indicates the response1, and the response2 may be reported
*     via the callback function. 
******************************************************************************/
/****************************************************/
/* Definitions for system URCs and the handler      */
/****************************************************/
const static ST_URC_HDLENTRY m_SysURCHdlEntry[] = {


    //Network status unsolicited response
    {"\r\n+CEREG:",                               OnURCHandler_Network},

    //SIM card unsolicited response
    {"\r\n+CPIN:",                                OnURCHandler_SIM},                       

    //CFUN unsolicited response
    {"\r\n+CFUN:",                                OnURCHandler_CFUN},
	//PSM EVENT unsolicited response
    {"\r\n+QNBIOTEVENT:",                         OnURCHandler_NbiotEvent},

};

/****************************************************/
/* Definitions for AT URCs and the handler          */
/****************************************************/
const static ST_URC_HDLENTRY m_AtURCHdlEntry[] = {
	{"\r\n+QIURC:",                               OnURCHandler_SOCKET_QIURC},
	{"\r\n+QIOPEN:",                              OnURCHandler_QIURC_OPEN},
	{"\r\n+QLWOBSERVE:",						  OnURCHandler_LwM2M_OBSERVE},
	{"\r\n+MIPLEVENT:",						      OnURCHandler_ONENET_EVENT},
	{"\r\n+MIPLOBSERVE:",						  OnURCHandler_ONENET_OBSERVER},
	{"\r\n+MIPLDISCOVER:",						  OnURCHandler_ONENET_DISCOVER},
	{"\r\n+MIPLREAD:",                            OnURCHandler_ONENET_READ},
	{"\r\n+QIND: \"FOTA\"",                       OnURCHandler_DFOTA_Hander},	
	{"\r\n+QMTOPEN:",                             OnURCHandler_MQTT_OPEN},
	{"\r\n+QMTCONN:",                             OnURCHandler_MQTT_CONN},
	{"\r\n+QMTSUB:",                              OnURCHandler_MQTT_SUB},
    {"\r\n+QMTPUB:",                              OnURCHandler_MQTT_PUB},
	{"\r\n+QMTUNS:",                              OnURCHandler_MQTT_TUNS},
	{"\r\n+QMTSTAT:",                             OnURCHandler_MQTT_STATE},
	{"\r\n+QMTCLOSE:",                            OnURCHandler_MQTT_CLOSE},
	{"\r\n+QMTDISC:",                             OnURCHandler_MQTT_DISC},
};



/**********************************************URC handler*****************************************************/
static void OnURCHandler_QIURC_OPEN(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QIOPEN:");
	p1 += Ql_strlen("+QIOPEN:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		socket_urc_param.connectID= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		socket_urc_param.error_no= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SOCKET_OPEN, &socket_urc_param);	
	}
}

static void OnURCHandler_SOCKET_QIURC(const char* strURC, void* reserved)
{
    u8* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
    s32 ret;
	
    p1 = Ql_strstr(strURC, "+QIURC:");
	p1 += Ql_strlen("+QIURC:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';
	
    if (p1)
    {		
		Ql_memset(strTmp, 0x0,  sizeof(strTmp));
	    ret = QSDK_Get_Str(p1,strTmp,0);
        if(Ql_memcmp(strTmp,"\"closed\"",Ql_strlen("\"closed\"")) == 0)
		{
			Ql_memset(strTmp, 0x0,  sizeof(strTmp));
		    QSDK_Get_Str(p1,strTmp,1);
			socket_urc_param.connectID = Ql_atoi(strTmp);
	
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SOCKET_CLOSE,socket_urc_param.connectID);
		}
    }
}

static void OnURCHandler_LwM2M_OBSERVE(const char* strURC, void* reserved)
{
    char* p1 = NULL;
	char* p2 = NULL;
    s32 res_id;
	char strTmp[10];
    p1 = Ql_strstr(strURC, "\r\n+QLWOBSERVE:");
	p1 += Ql_strlen("\r\n+QLWOBSERVE:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';
    if (p1)
    {
		  Ql_memset(strTmp, 0x0,  sizeof(strTmp));
		  QSDK_Get_Str(p1,strTmp,0);
		  lwm2m_urc_param.observe_flag= Ql_atoi(strTmp);

		  Ql_memset(strTmp, 0x0,  sizeof(strTmp));
		  QSDK_Get_Str(p1,strTmp,1);
		  lwm2m_urc_param.obj_id= Ql_atoi(strTmp);

		  Ql_memset(strTmp, 0x0,  sizeof(strTmp));
		  QSDK_Get_Str(p1,strTmp,2);
		  lwm2m_urc_param.ins_id= Ql_atoi(strTmp);

		  Ql_memset(strTmp, 0x0,  sizeof(strTmp));
		  QSDK_Get_Str(p1,strTmp,3);
		  lwm2m_urc_param.res_num= Ql_atoi(strTmp);
          Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LwM2M_OBSERVE, &lwm2m_urc_param);
    }

}


OnURCHandler_ONENET_EVENT(const char* strURC, void* reserved)
{
	char* p1 = NULL;
    char* p2 = NULL;
    s32 ret;
    char strTmp[10];
    p1 = Ql_strstr(strURC, "+MIPLEVENT:");
	p1 += Ql_strlen("+MIPLEVENT:");
    p1++;
	
    if (p1)
    {
  		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
  		QSDK_Get_Str(p1,strTmp,0);
  		onenet_urc_param.ref = Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
  		QSDK_Get_Str(p1,strTmp,1);
  		onenet_urc_param.evtid = Ql_atoi(strTmp);

		if ( onenet_urc_param.evtid == EVENT_RESPONSE_FAILED || onenet_urc_param.evtid == EVENT_NOTIFY_FAILED )
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
			QSDK_Get_Str(p1, strTmp, 2);
			onenet_urc_param.msgid = Ql_atoi(strTmp);
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_ONENET_EVENT, &onenet_urc_param);
		}
		else if ( onenet_urc_param.evtid == EVENT_UPDATE_NEED )
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
			QSDK_Get_Str(p1, strTmp, 2);
			onenet_urc_param.remain_lifetime = Ql_atoi(strTmp);
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_ONENET_EVENT, &onenet_urc_param);
		}
		else if ( onenet_urc_param.evtid == EVENT_NOTIFY_SUCCESS )
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
			ret = QSDK_Get_Str(p1, strTmp, 2);
			if ( ret == TRUE )
			{
				onenet_urc_param.ackid = Ql_atoi(strTmp);
				Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_ONENET_EVENT, &onenet_urc_param);
			}
			else 
			{
				Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_ONENET_EVENT, &onenet_urc_param);
			}
		}
		else 
		{
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_ONENET_EVENT, &onenet_urc_param);
		}
    }
}


OnURCHandler_ONENET_OBSERVER(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	p1 = Ql_strstr(strURC, "+MIPLOBSERVE:");
	p1 += Ql_strlen("+MIPLOBSERVE:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		onenet_urc_param.ref = Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		onenet_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		onenet_urc_param.observe_flag= Ql_atoi(strTmp);
		
	    Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,3);
		onenet_urc_param.objid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,4);
		onenet_urc_param.insid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,5);
		onenet_urc_param.resid= Ql_atoi(strTmp);

		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_ONENET_OBSERVE, &onenet_urc_param);
	}
}

static void OnURCHandler_ONENET_DISCOVER(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	p1 = Ql_strstr(strURC, "+MIPLDISCOVER:");
	p1 += Ql_strlen("+MIPLDISCOVER:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		onenet_urc_param.ref = Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		onenet_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		onenet_urc_param.objid= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_ONENET_DISCOVER, &onenet_urc_param);
	}
}

static void OnURCHandler_ONENET_READ(const char* strURC, void* reserved)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	s32 ret;
	p1 = Ql_strstr(strURC, "+MIPLREAD:");
	p1 += Ql_strlen("+MIPLTREAD:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';
	
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
	//+MIPLREAD: <ref>,<msgId>,<objId>,<insId>,<resId>
	if ( p1 )
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		onenet_urc_param.ref = Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		onenet_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		onenet_urc_param.objid= Ql_atoi(strTmp);

		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,3);
		onenet_urc_param.insid= Ql_atoi(strTmp);

		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,4);
		onenet_urc_param.resid= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_ONENET_READ, &onenet_urc_param);
	}
}

static void OnURCHandler_SIM(const char* strURC, void* reserved)
{
    char* p1 = NULL;
    char* p2 = NULL;
    char strTmp[20];
    s32 len;
	
    Ql_memset(strTmp, 0x0, sizeof(strTmp));
    len = Ql_sprintf(strTmp, "\r\n+CPIN: ");
    if (Ql_StrPrefixMatch(strURC, strTmp))
    {
        p1 = Ql_strstr(strURC, "\r\n+CPIN: ");
        p1 += len;
        p2 = Ql_strstr(p1, "\r\n");
        if (p1 && p2)
        {
            Enum_SIMState cpinStat;
            Ql_memset(strTmp, 0x0, sizeof(strTmp));
            Ql_memcpy(strTmp, p1, p2 - p1);
            cpinStat = RIL_SIM_GetSimStateByName(strTmp, p2 - p1);
            Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SIM_CARD_STATE_IND, cpinStat);
        }
    }
}

static void OnURCHandler_Network(const char* strURC, void* reserved)
{
    char* p1 = NULL;
    char* p2 = NULL;
    char strTmp[10];
    
    if (Ql_StrPrefixMatch(strURC, "\r\n+CEREG: "))
    {
        u32 nwStat;
        p1 = Ql_strstr(strURC, "\r\n+CEREG: ");
        p1 += Ql_strlen("\r\n+CEREG: ");
		if(*(p1+1) == 0x2C)          //Active query network status without reporting URCS
		{
		   return;
		}
        p2 = Ql_strstr(p1, "\r\n");
        if (p1 && p2)
        {
            Ql_memset(strTmp, 0x0, sizeof(strTmp));
            Ql_memcpy(strTmp, p1, p2 - p1);
            nwStat = Ql_atoi(strTmp);
            Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_EGPRS_NW_STATE_IND, nwStat);
        }
    }
}

static void OnURCHandler_InitStat(const char* strURC, void* reserved)
{
    u32 sysInitStat = SYS_STATE_START;
    
    if (Ql_strstr(strURC, "\r\nCall Ready\r\n") != NULL)
    {
        sysInitStat = SYS_STATE_PHBOK;
    }
    else if(Ql_strstr(strURC, "\r\nSMS Ready\r\n") != NULL)
    {
        sysInitStat = SYS_STATE_SMSOK;
    }
    Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SYS_INIT_STATE_IND, sysInitStat);
}

static void OnURCHandler_CFUN(const char* strURC, void* reserved)
{
    char* p1 = NULL;
    char* p2 = NULL;
    char strTmp[10];
    s32 len;
    u32 cfun;

    len = Ql_strlen("\r\n+CFUN: ");
    p1 = Ql_strstr(strURC, "\r\n+CFUN: ");
    p1 += len;
    p2 = Ql_strstr(p1, "\r\n");
    if (p1 && p2)
    {
        Ql_memset(strTmp, 0x0, sizeof(strTmp));
        Ql_memcpy(strTmp, p1, 1);
        cfun = Ql_atoi(strTmp);
        Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_CFUN_STATE_IND, cfun);
    }
}


static void OnURCHandler_NbiotEvent(const char* strURC, void* reserved)
{
    Enum_PSM_State psm_state = END_PSM;

	RIL_URC_DEBUG(DBG_Buffer,"OnURCHandler_NbiotEvent(%s)\r\n", strURC);
	
	if (Ql_strstr(strURC, "\r\n+QNBIOTEVENT: \"ENTER PSM\"") != NULL)
	{
		psm_state = ENTER_PSM;
	}
	else if (Ql_strstr(strURC, "\r\n+QNBIOTEVENT: \"EXIT PSM\"") != NULL)
	{
		psm_state= EXIT_PSM;
	}
	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_PSM_EVENT, psm_state);
}

static void OnURCHandler_DFOTA_Hander  (const char* strURC, void* reserved)
{
    Dfota_Upgrade_State upgrade_state = DFOTA_STATE_END;
	s32 dfota_errno; 

	//RIL_URC_DEBUG(DBG_Buffer,"OnURCHandler_DFOTA_Hander(%s)\r\n", strURC);
	if (Ql_strstr(strURC, "\r\n+QIND: \"FOTA\"") != NULL)
	{
		DFOTA_Analysis(strURC,&upgrade_state,&dfota_errno);
		if(dfota_errno == 0)//normal
		{
            Dfota_Upgrade_States(upgrade_state,0);
		}
		else//failed
		{
			Dfota_Upgrade_States(upgrade_state,dfota_errno);
		}
	}
}

static void OnURCHandler_MQTT_OPEN(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTOPEN:");
	p1 += Ql_strlen("+QMTOPEN:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_OPEN, &mqtt_urc_param);	
	}

}

static void OnURCHandler_MQTT_CONN(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTCONN:");
	p1 += Ql_strlen("+QMTCONN:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		if(TRUE == QSDK_Get_Str(p1,strTmp,2))
		{
		  mqtt_urc_param.connect_code= Ql_atoi(strTmp);
		}
		else
		{
			mqtt_urc_param.connect_code= 255;
		}
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_CONN, &mqtt_urc_param);
	}
}

static void OnURCHandler_MQTT_SUB(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	char i;
	
	p1 = Ql_strstr(strURC, "+QMTSUB:");
	p1 += Ql_strlen("+QMTSUB:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);

		for(i=0;i<MQTT_MAX_TOPIC;i++)
        {
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
     		if(TRUE == QSDK_Get_Str(p1,strTmp,(3+i)))
     		{
     		  mqtt_urc_param.sub_value[i]= Ql_atoi(strTmp);
     		}
     		else
     		{
     			mqtt_urc_param.sub_value[i]= 255;
				break;
     		}
		}

		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_SUB, &mqtt_urc_param);	
	}

}

static void OnURCHandler_MQTT_PUB(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];

	p1 = Ql_strstr(strURC, "+QMTPUB:");
	p1 += Ql_strlen("+QMTPUB:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';
	

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);

  		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		if(TRUE == QSDK_Get_Str(p1,strTmp,3))
		{
		   mqtt_urc_param.pub_value= Ql_atoi(strTmp);
		}
		else
		{
			mqtt_urc_param.pub_value= 255;
		}
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_PUB, &mqtt_urc_param);	
	}
}

static void OnURCHandler_MQTT_TUNS(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTUNS:");
	p1 += Ql_strlen("+QMTUNS:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_TUNS, &mqtt_urc_param);	
	}
}

static void OnURCHandler_MQTT_STATE(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTSTAT:");
	p1 += Ql_strlen("+QMTSTAT:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.mqtt_state= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_STATE, &mqtt_urc_param);	
	}
}

static void OnURCHandler_MQTT_CLOSE(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	p1 = Ql_strstr(strURC, "+QMTCLOSE:");
	p1 += Ql_strlen("+QMTCLOSE:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_CLOSE, &mqtt_urc_param);	
	}
}

static void OnURCHandler_MQTT_DISC(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTDISC:");
	p1 += Ql_strlen("+QMTDISC:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_DISC, &mqtt_urc_param);	
	}
}




static void OnURCHandler_Undefined(const char* strURC, void* reserved)
{
    Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_END, 0);
}

/*****************************************************************
* Function:     OnURCHandler 
* 
* Description:
*               This function is the entrance for Unsolicited Result Code (URC) Handler.
*
* Parameters:
*               strURC:      
*                   [IN] a URC string terminated by '\0'.
*
*               reserved:       
*                   reserved, can be NULL.
* Return:        
*               The function returns "ptrUrc".
*****************************************************************/
void OnURCHandler(const char* strURC, void* reserved)
{
    s32 i;
    
    if (NULL == strURC)
    {
        return;
    }
    // For system URCs
    for (i = 0; i < NUM_ELEMS(m_SysURCHdlEntry); i++)
    {
        if (Ql_strstr(strURC, m_SysURCHdlEntry[i].keyword))
        {
            m_SysURCHdlEntry[i].handler(strURC, reserved);
            return;
        }
    }

    // For AT URCs
    for (i = 0; i < NUM_ELEMS(m_AtURCHdlEntry); i++)
    {
        if (Ql_strstr(strURC, m_AtURCHdlEntry[i].keyword))
        {
            m_AtURCHdlEntry[i].handler(strURC, reserved);
            return;
        }
    }

    // For undefined URCs
    OnURCHandler_Undefined(strURC, reserved);
}

/******************************************************************************
* Function:     Ql_RIL_IsURCStr
*  
* Description:
*               This function is used to check whether a string is URC information
*               you defined.
.
* Parameters:    
*               strRsp: 
*                     [in]a string for the response of the AT command.
* Return:  
*               0 : not URC information
*               1 : URC information
******************************************************************************/
s32 Ql_RIL_IsURCStr(const char* strRsp)
{
    s32 i;
    for (i = 0; i < NUM_ELEMS(m_SysURCHdlEntry); i++) 
    {
        if (Ql_strstr(strRsp, m_SysURCHdlEntry[i].keyword)) 
        {
            return 1;
        }
    }
    for (i = 0; i < NUM_ELEMS(m_AtURCHdlEntry); i++) 
    {
        if (Ql_strstr(strRsp, m_AtURCHdlEntry[i].keyword)) 
        {
            return 1;
        }
    }
    return 0;
}

#endif  // __OCPU_RIL_SUPPORT__
