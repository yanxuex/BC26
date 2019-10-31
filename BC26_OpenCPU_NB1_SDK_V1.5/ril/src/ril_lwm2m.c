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
 *   ril_lwm2m.c 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   The module implements lwm2m related APIs.
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
#include "ril_network.h"
#include "ril.h"
#include "ril_util.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_system.h"
#include "ql_common.h"
#include "ril_LwM2M.h"
#include "ql_uart.h"

#ifdef __OCPU_RIL_SUPPORT__

#define RIL_LwM2M_DEBUG_ENABLE 0
#if RIL_LwM2M_DEBUG_ENABLE > 0
#define RIL_LwM2M_DEBUG_PORT  UART_PORT2
static char DBG_Buffer[1024];
#define RIL_LwM2M_DEBUG(BUF,...)  QL_TRACE_LOG(RIL_LwM2M_DEBUG_PORT,BUF,1024,__VA_ARGS__)
#else
#define RIL_LwM2M_DEBUG(BUF,...) 
#endif


static s32 ATResponse_Handler(char* line, u32 len, void* userData)
{
    RIL_LwM2M_DEBUG(DBG_Buffer,"[ATResponse_Handler] %s\r\n", (u8*)line);
    
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

static s32 ATResponse_Open_Handler(char* line, u32 len, void* userData)
{
	RIL_LwM2M_DEBUG(DBG_Buffer,"[ATResponse_Handler] %s\r\n", (u8*)line);
	
	if (Ql_RIL_FindString(line, len, "CONNECT OK"))
	{  
		return	RIL_ATRSP_SUCCESS;
	}
	else if (Ql_RIL_FindLine(line, len, "ERROR"))
	{  
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
	{
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
	{
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "CONNECT FAIL"))
	{
		return	RIL_ATRSP_FAILED;
	}
	
	return RIL_ATRSP_CONTINUE; //continue wait
}


static s32 ATResponse_Update_Handler(char* line, u32 len, void* userData)
{
	RIL_LwM2M_DEBUG(DBG_Buffer,"[ATResponse_Handler] %s\r\n", (u8*)line);
	
	if (Ql_RIL_FindString(line, len, "UPDATE OK"))
	{  
		return	RIL_ATRSP_SUCCESS;
	}
	else if (Ql_RIL_FindLine(line, len, "ERROR"))
	{  
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
	{
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
	{
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "UPDATE FAIL"))
	{
		return	RIL_ATRSP_FAILED;
	}
	
	return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATResponse_Send_Handler(char* line, u32 len, void* userData)
{
	RIL_LwM2M_DEBUG(DBG_Buffer,"[ATResponse_Handler] %s\r\n", (u8*)line);
	
	if (Ql_RIL_FindString(line, len, "SEND OK"))
	{  
		return	RIL_ATRSP_SUCCESS;
	}
	else if (Ql_RIL_FindLine(line, len, "ERROR"))
	{  
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
	{
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
	{
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "SEND FAIL"))
	{
		return	RIL_ATRSP_FAILED;
	}
	
	return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATResponse_Close_Handler(char* line, u32 len, void* userData)
{
	RIL_LwM2M_DEBUG(DBG_Buffer,"[ATResponse_Handler] %s\r\n", (u8*)line);
	
	if (Ql_RIL_FindString(line, len, "CLOSE OK"))
	{  
		return	RIL_ATRSP_SUCCESS;
	}
	else if (Ql_RIL_FindLine(line, len, "ERROR"))
	{  
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
	{
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
	{
		return	RIL_ATRSP_FAILED;
	}
	else if (Ql_RIL_FindString(line, len, "CLOSE FAIL"))
	{
		return	RIL_ATRSP_FAILED;
	}
	
	return RIL_ATRSP_CONTINUE; //continue wait
}

/********************************************RIL API********************************************/
s32  RIL_QLwM2M_Serv(u8* ip_addr,u32 port)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200] ;
	
    Ql_memset(strAT,0, sizeof(strAT));
    Ql_sprintf(strAT,"AT+QLWSERV=\"%s\",%d\n",ip_addr,port);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT,ret);
    return ret;
}



s32 RIL_QLwM2M_Conf(u8* endpoint) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+QLWCONF=\"%s\"\n",endpoint);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;   
}

s32 RIL_QLwM2M_Addobj(u32 obj_id,u8 ins_id,u8 res_num,u8* res_id) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];
	
    Ql_memset(strAT,0, sizeof(strAT));
	if(NULL == res_id)
	{
		 Ql_sprintf(strAT, "AT+QLWADDOBJ=%d,%d\n",obj_id,ins_id);
	}
	else 
	{
		Ql_sprintf(strAT, "AT+QLWADDOBJ=%d,%d,%d,\"%s\"\n",obj_id,ins_id, res_num,res_id);
	}
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
    
}

s32 RIL_QLwM2M_Delobj(u8 obj_id) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];
	
    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT, "AT+QLWDELOBJ=%d\n",obj_id);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
    
}


s32 RIL_QLwM2M_Open(Enum_Lwm2m_Access_Mode lwm2m_access_mode)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+QLWOPEN=%d\n",lwm2m_access_mode);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
    
}

s32 RIL_QLwM2M_Update(void) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+QLWUPDATE");
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Update_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
    
}

s32 RIL_QLwM2M_Cfg(Enum_Lwm2m_Data_Format send_format,Enum_Lwm2m_Data_Format recv_format) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT, 0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+QLWCFG=\"dataformat\",%d,%d\n",send_format,recv_format);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;    
}

s32 RIL_QLwM2M_Send(ST_Lwm2m_Send_Param_t *lwm2m_send_param_t)
{
    s32 ret = RIL_AT_SUCCESS;
    u8* strAT  = NULL;

    if((NULL ==lwm2m_send_param_t->buffer)||(lwm2m_send_param_t->send_length>LWM2M_SEND_BUFFER_LENGTH))
    {
      return RIL_AT_INVALID_PARAM;
	}
	strAT = (u8*)Ql_MEM_Alloc((sizeof(u8)*LWM2M_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}

	Ql_memset(strAT, 0,(sizeof(u8)*LWM2M_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
	if(NULL == lwm2m_send_param_t->buffer)
	{
		Ql_MEM_Free(strAT);
	    strAT = NULL;
		return RIL_AT_INVALID_PARAM;
	}

	if(LWM2M_SEND_MODE_NON == lwm2m_send_param_t->lwm2m_send_mode)
	{
	  Ql_sprintf(strAT, "AT+QLWDATASEND=%d,%d,%d,%d,\"%s\",\"%s\"\n",lwm2m_send_param_t->obj_id,\
	  lwm2m_send_param_t->ins_id,lwm2m_send_param_t->res_id,lwm2m_send_param_t->send_length,lwm2m_send_param_t->buffer,"0x0000");
      ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	}
	else if(LWM2M_SEND_MODE_CON == lwm2m_send_param_t->lwm2m_send_mode)
	{
	  Ql_sprintf(strAT, "AT+QLWDATASEND=%d,%d,%d,%d,\"%s\",\"%s\"\n",lwm2m_send_param_t->obj_id,\
	  lwm2m_send_param_t->ins_id,lwm2m_send_param_t->res_id,lwm2m_send_param_t->send_length,lwm2m_send_param_t->buffer,"0x0100");
      ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Send_Handler,NULL,0);
	}
	
    RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret); 
	if(NULL != strAT)
	{
       Ql_MEM_Free(strAT);
	   strAT = NULL;
	}
    return ret;    
}

s32 RIL_QLwM2M_Close(void) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+QLWCLOSE\n");
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Close_Handler,NULL,0);
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;    
}

s32 RIL_QLwM2M_Delete(void) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+QLWDEL\n");
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;    
}

#endif  //__OCPU_RIL_SUPPORT__

