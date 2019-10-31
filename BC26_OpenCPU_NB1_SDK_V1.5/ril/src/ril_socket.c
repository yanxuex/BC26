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
 *   ril_network.c 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   The module implements network related APIs.
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
#include "ril_util.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_uart.h"
#include "ril_socket.h"

#ifdef __OCPU_RIL_SUPPORT__

#define RIL_SOCKET_DEBUG_ENABLE 0
#if RIL_SOCKET_DEBUG_ENABLE > 0
#define RIL_SOCKET_DEBUG_PORT  UART_PORT2
static char DBG_Buffer[1024];
#define RIL_SOCKET_DEBUG(BUF,...)  QL_TRACE_LOG(RIL_SOCKET_DEBUG_PORT,BUF,1024,__VA_ARGS__)
#else
#define RIL_SOCKET_DEBUG(BUF,...) 
#endif


static s32 ATResponse_Handler(char* line, u32 len, void* userData)
{
    
    if (Ql_RIL_FindLine(line, len, "OK"))
    {  
        return  RIL_ATRSP_SUCCESS;
    }
    else if (Ql_RIL_FindLine(line, len, "ERROR"))
    {  
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
    {
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
    {
        return  RIL_ATRSP_FAILED;
    }
    
    return RIL_ATRSP_CONTINUE; //continue wait
}


static s32 ATRsp_Soc_Qisend_Handler(char* line, u32 len, void* userData)
{
    
    if (Ql_RIL_FindLine(line, len, "SEND OK"))
    {  
        return  RIL_ATRSP_SUCCESS;
    }
    else if (Ql_RIL_FindLine(line, len, "ERROR"))
    {  
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
    {
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "SEND FAIL"))
    {
        return  RIL_ATRSP_FAILED;
    }
    
    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATRsp_Soc_Qiclose_Handler(char* line, u32 len, void* userData)
{
    
    if (Ql_RIL_FindLine(line, len, "CLOSE OK"))
    {  
        return  RIL_ATRSP_SUCCESS;
    }
    else if (Ql_RIL_FindLine(line, len, "ERROR"))
    {  
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
    {
        return  RIL_ATRSP_FAILED;
    }
    
    return RIL_ATRSP_CONTINUE; //continue wait
}

s32  RIL_SOC_QIOPEN(ST_Socket_Param_t* socket_param_t)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT, 0, sizeof(strAT));
    Ql_sprintf(strAT, "AT+QICFG=\"%s\",1\n","viewmode");
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),NULL,NULL,0);
    RIL_SOCKET_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    if (RIL_AT_SUCCESS != ret)
    {
        return ret;
    }
	
    Ql_memset(strAT, 0, sizeof(strAT));
    Ql_sprintf(strAT, "AT+QICFG=\"%s\",1\n","showlength");
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),NULL,NULL,0);
    RIL_SOCKET_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    if (RIL_AT_SUCCESS != ret)
    {
        return ret;
    }

    Ql_memset(strAT, 0, sizeof(strAT));
	if(socket_param_t->service_type == SOCKET_SERVICE_TYPE_TCP)
	{
	   Ql_sprintf(strAT,"AT+QIOPEN=%d,%d,\"%s\",\"%s\",%d,%d,%d,%d\n",socket_param_t->contextID,socket_param_t->connectID, \
	  	          "TCP",socket_param_t->address,socket_param_t->remote_port,socket_param_t->local_port, \
	  	          SOCKET_ACCESS_MODE_DIRECT,socket_param_t->protocol_type);
	}
	else if(socket_param_t->service_type == SOCKET_SERVICE_TYPE_UDP)
	{
    	Ql_sprintf(strAT,"AT+QIOPEN=%d,%d,\"%s\",\"%s\",%d,%d,%d,%d\n",socket_param_t->contextID,socket_param_t->connectID,\
    		       "UDP",socket_param_t->address,socket_param_t->remote_port,socket_param_t->local_port,\
    		       SOCKET_ACCESS_MODE_DIRECT,socket_param_t->protocol_type);
	}
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	RIL_SOCKET_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;

}

s32 RIL_SOC_QISEND(u8 connectID, u32 send_length,u8* send_buffer)
{
    s32 ret = RIL_AT_SUCCESS;
	u8* strAT  = NULL;
    if((send_length > SOCKET_SEND_BUFFER_LENGTH)||(NULL == send_buffer))
    {
		return RIL_AT_INVALID_PARAM;
	}
	
	strAT = (u8*)Ql_MEM_Alloc(sizeof(u8)*(SOCKET_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}
	
    Ql_memset(strAT, 0, sizeof(u8)*(SOCKET_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
    Ql_sprintf(strAT, "AT+QISEND=%d,%d,\"%s\"\n",connectID,send_length, send_buffer);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATRsp_Soc_Qisend_Handler,0,30*1000);
	RIL_SOCKET_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
	if(NULL != strAT)
	{
       Ql_MEM_Free(strAT);
	   strAT  = NULL;
	}
    return ret;
}

s32 RIL_SOC_QISENDEX(u8 connectID, u32 send_length,u8* send_hex_buffer)
{
    s32 ret = RIL_AT_SUCCESS;
    u8* strAT  = NULL;

    if((send_length > SOCKET_SEND_BUFFER_LENGTH)||(NULL == send_hex_buffer))
    {
		return RIL_AT_INVALID_PARAM;
	}

	strAT = (u8*)Ql_MEM_Alloc(sizeof(u8)*(SOCKET_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}
	
    Ql_memset(strAT, 0,sizeof(u8)*(SOCKET_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
    Ql_sprintf(strAT, "AT+QISENDEX=%d,%d,\"%s\"\n",connectID,send_length, send_hex_buffer);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATRsp_Soc_Qisend_Handler,0,30*1000);
	RIL_SOCKET_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
	if(NULL != strAT)
	{
       Ql_MEM_Free(strAT);
	   strAT  = NULL;
	}
    return ret;
}

s32 RIL_SOC_QICFG_FORMAT(bool send_format,bool recv_format)

{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];
	
    Ql_memset(strAT, 0, sizeof(strAT));
    Ql_sprintf(strAT, "AT+QICFG=\"dataformat\",%d,%d\n",send_format,recv_format);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,0,0);
	
	RIL_SOCKET_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
}

s32 RIL_SOC_QICLOSE(u8 connectID)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];
	
    Ql_memset(strAT, 0, sizeof(strAT));
    Ql_sprintf(strAT, "AT+QICLOSE=%d\n",connectID);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATRsp_Soc_Qiclose_Handler,0,0);
	
	RIL_SOCKET_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
}

#endif  //__OCPU_RIL_SUPPORT__

