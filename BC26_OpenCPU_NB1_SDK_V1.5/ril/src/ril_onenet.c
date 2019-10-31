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
 *   ril_onenet.c 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   The module implements onenet related APIs.
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
#include "ril_onenet.h"
#include "ql_uart.h"

#ifdef __OCPU_RIL_SUPPORT__

#define RIL_ONENET_DEBUG_ENABLE 0
#if RIL_ONENET_DEBUG_ENABLE > 0
#define RIL_ONENET_DEBUG_PORT  UART_PORT2
static char DBG_Buffer[1024];
#define RIL_ONENET_DEBUG(BUF,...)  QL_TRACE_LOG(RIL_ONENET_DEBUG_PORT,BUF,1024,__VA_ARGS__)
#else
#define RIL_ONENET_DEBUG(BUF,...) 
#endif


static s32 ATResponse_Handler(char* line, u32 len, void* userData)
{
    RIL_ONENET_DEBUG(DBG_Buffer,"[ATResponse_Handler] %s\r\n", (u8*)line);
    
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
	else if (Ql_RIL_FindString(line, len, "+CIS ERROR:"))
	{
		return RIL_ATRSP_FAILED;
	}
    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATResponse_Create_Handler(char* line, u32 len, void* userData)
{
	ST_RIL_Onenet_Param *onenet_param = (ST_RIL_Onenet_Param *)userData;
    char *head = Ql_RIL_FindString(line, len, onenet_param->prefix); //continue wait
    
    if(head)
    {
            char* p1 = NULL;
            char* p2 = NULL;

            p1 = Ql_strstr(head, ":");
			p1 +=1;
            p2 = Ql_strstr(p1, "\r\n");
			*p2 = '\0';
            if (p1)
            {
                onenet_param->errno= Ql_atoi(p1);
				RIL_ONENET_DEBUG(DBG_Buffer,"[socket_param->length] %d\r\n",onenet_param->errno);
            }
        return RIL_ATRSP_CONTINUE;
    }
    head = Ql_RIL_FindLine(line, len, "OK");
    if(head)
    {  
        return  RIL_ATRSP_SUCCESS;  
    }
    head = Ql_RIL_FindLine(line, len, "ERROR");
    if(head)
    {  
        return  RIL_ATRSP_FAILED;
    }
    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
    head = Ql_RIL_FindString(line, len, "+CMS ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
	head = Ql_RIL_FindString(line, len, "+CIS ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}


/********************************************RIL API********************************************/
s32  RIL_QONENET_Create(void)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200] ;
	ST_RIL_Onenet_Param  onenet_param;
	onenet_param.prefix ="+MIPLCREATE:";

    Ql_memset(strAT,0, sizeof(strAT));
    Ql_sprintf(strAT,"AT+MIPLCREATE\n");
	
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Create_Handler,&onenet_param,0);
	RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,errno=(%d),ret=(%d) -->\r\n",strAT,onenet_param.errno,ret);
	if(RIL_AT_SUCCESS != ret)
    {
        return ret;
    }
    else if(0 != onenet_param.errno) 
    {
        return onenet_param.errno;// error number
    }
	
    return ret;
}



s32 RIL_QONENET_Addobj(ST_ONENET_Obj_Param_t *onenet_obj_param_t)
{
	 s32 ret = RIL_AT_SUCCESS;
	 char strAT[200] ;
	
	 Ql_memset(strAT,0, sizeof(strAT));
	 Ql_sprintf(strAT,"AT+MIPLADDOBJ=%d,%d,%d,\"%s\",%d,%d\n",onenet_obj_param_t->ref,onenet_obj_param_t->obj_id,\
	 onenet_obj_param_t->ins_count,onenet_obj_param_t->insbitmap,onenet_obj_param_t->attrcount,onenet_obj_param_t->actcount);

	 ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	 RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}

s32 RIL_QONENET_Delobj(u32 ref,u32 obj_id)
{
	 s32 ret = RIL_AT_SUCCESS;
	 char strAT[200] ;
	
	 Ql_memset(strAT,0, sizeof(strAT));
	 Ql_sprintf(strAT,"AT+MIPLDELOBJ=%d,%d\n",ref,obj_id);

	 ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	 RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}


s32 RIL_QONENET_Open(u32 ref,u32 lifetime )
{
	 s32 ret = RIL_AT_SUCCESS;
	 char strAT[200] ;
	
	 Ql_memset(strAT,0, sizeof(strAT));
	 Ql_sprintf(strAT,"AT+MIPLOPEN=%d,%d\n",ref,lifetime);

	 ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	 RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}


s32 RIL_QONENET_Observer_Rsp(ST_ONENET_Observe_Param_t* onenet_observe_param_t)
{
	s32 ret = RIL_AT_SUCCESS;
	char strAT[200] ;
	
	Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+MIPLOBSERVERSP=%d,%d,%d\n",onenet_observe_param_t->ref,onenet_observe_param_t->msgid,onenet_observe_param_t->obderve_result);
	
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	if(RIL_AT_SUCCESS != ret)
	{
		return ret;
	}
	return ret;
}


s32 RIL_QONENET_Discover_Rsp(ST_ONENET_Discover_Rsp_Param_t *onenet_discover_rsp_param_t)
{
	s32 ret = RIL_AT_SUCCESS;
	char strAT[200] ;
	
	Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+MIPLDISCOVERRSP=%d,%d,%d,%d,\"%s\"\n",onenet_discover_rsp_param_t->ref,onenet_discover_rsp_param_t->msgid,\
	onenet_discover_rsp_param_t->result,onenet_discover_rsp_param_t->length,onenet_discover_rsp_param_t->value_string);
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	if(RIL_AT_SUCCESS != ret)
	{
		return ret;
	}
	return ret;
}


s32 RIL_QONENET_Config(ST_ONENET_Config_Param_t* onenet_config_param_t,bool server_config_flag)
{
   	 s32 ret = RIL_AT_SUCCESS;
	 char strAT[200] ;

	
	 Ql_memset(strAT,0, sizeof(strAT));
	 if(FALSE == server_config_flag)
	 {
	   Ql_sprintf(strAT,"AT+MIPLCONFIG=%d,%d\n",onenet_config_param_t->onenet_access_mode,onenet_config_param_t->onenet_recv_mode);
	 }
	 else 
	 {
		Ql_sprintf(strAT,"AT+MIPLCONFIG=%d,%d,%d,\"%s\",%d\n",onenet_config_param_t->onenet_access_mode,onenet_config_param_t->onenet_recv_mode,
		onenet_config_param_t->onenet_bs_mode,onenet_config_param_t->ip,onenet_config_param_t->port);
	 }
	 ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	 RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}


s32 RIL_QONENET_Notify(ST_ONENET_Notify_Param_t* onenet_notify_param_t,bool ack_flag)
{
	s32 ret = RIL_AT_SUCCESS;
	u8* strAT  = NULL;
	
    if((NULL ==onenet_notify_param_t->value)||(onenet_notify_param_t->len > ONENET_SEND_BUFFER_LENGTH))
    {
      return RIL_AT_INVALID_PARAM;
	}
	
	strAT = (u8*)Ql_MEM_Alloc((sizeof(u8)*ONENET_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}
	
	Ql_memset(strAT,0,(sizeof(u8)*ONENET_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
	
	if(ack_flag == FALSE)
	{
	Ql_sprintf(strAT,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",%d,%d\n",onenet_notify_param_t->ref,\
		onenet_notify_param_t->msgid,onenet_notify_param_t->objid,onenet_notify_param_t->insid,     \
		onenet_notify_param_t->resid,onenet_notify_param_t->value_type,onenet_notify_param_t->len,  \
		onenet_notify_param_t->value, onenet_notify_param_t->index,onenet_notify_param_t->flag);
	}
    else if(ack_flag == TRUE)
	{
	Ql_sprintf(strAT,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",%d,%d,%d\n",onenet_notify_param_t->ref,\
		onenet_notify_param_t->msgid,onenet_notify_param_t->objid,onenet_notify_param_t->insid,     \
		onenet_notify_param_t->resid,onenet_notify_param_t->value_type,onenet_notify_param_t->len,  \
		onenet_notify_param_t->value,onenet_notify_param_t->index,onenet_notify_param_t->flag,\
		onenet_notify_param_t->ackid);
	}
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	if(NULL != strAT)
	{
       Ql_MEM_Free(strAT);
	   strAT  = NULL;
	}
	
	if(RIL_AT_SUCCESS != ret)
	{
		return ret;
	}
	return ret;
}


s32 RIL_QONENET_Update(u32 ref,u32 lifetime,Enum_ONENET_Obj_Flag obj_flag)
{

     s32 ret = RIL_AT_SUCCESS;
	 char strAT[200] ;
	
	 Ql_memset(strAT,0, sizeof(strAT));
	 Ql_sprintf(strAT,"AT+MIPLUPDATE=%d,%d,%d\n",ref,lifetime,obj_flag);

	 ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	 RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}

s32 RIL_QONENET_Write_Rsp(ST_ONENET_Write_Rsp_Param_t* onenet_write_rsp_param_t)
{

     s32 ret = RIL_AT_SUCCESS;
	 char strAT[200] ;

     Ql_memset(strAT,0, sizeof(strAT));
     Ql_sprintf(strAT,"AT+MIPLWRITERSP=%d,%d,%d\n",onenet_write_rsp_param_t->ref,onenet_write_rsp_param_t->msgid,onenet_write_rsp_param_t->result);
    
     ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
     RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}

s32 RIL_QONENET_Read_Rsp(ST_ONENET_Notify_Param_t* onenet_read_param_t)
{
	s32 ret = RIL_AT_SUCCESS;
	u8* strAT  = NULL;

	strAT = (u8*)Ql_MEM_Alloc((sizeof(u8)*ONENET_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}

	Ql_memset(strAT,0,(sizeof(u8)*ONENET_SEND_BUFFER_LENGTH*2)+RIL_MAX_AT_HEAD_LEN);
	
	Ql_sprintf(strAT,"AT+MIPLREADRSP=%d,%d,%d,%d,%d,%d,%d,%d,\"%s\",%d,%d\n",onenet_read_param_t->ref,\
			onenet_read_param_t->msgid,onenet_read_param_t->result, onenet_read_param_t->objid,onenet_read_param_t->insid, 	\
			onenet_read_param_t->resid,onenet_read_param_t->value_type,onenet_read_param_t->len,	\
			onenet_read_param_t->value, onenet_read_param_t->index,onenet_read_param_t->flag);

	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	if(NULL != strAT)
	{
       Ql_MEM_Free(strAT);
	   strAT  = NULL;
	}
	
	if(RIL_AT_SUCCESS != ret)
	{
		return ret;
	}
	return ret;
}

s32 RIL_QONENET_Execute_Rsp(u32 ref, u32 msgid, u32 result)
{
     s32 ret = RIL_AT_SUCCESS;
	 char strAT[200] ;
	
	 Ql_memset(strAT,0, sizeof(strAT));
	 Ql_sprintf(strAT,"AT+MIPLEXECUTERSP=%d,%d,%d\n",ref,msgid,result);

	 ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	 RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}

s32 RIL_QONENET_CLOSE(u32 ref)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+MIPLCLOSE=%d\n",ref);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;    
}


s32 RIL_QONENET_DELETE(u32 ref)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+MIPLDELETE=%d\n",ref);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	RIL_ONENET_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;    
}
 
#endif  //__OCPU_RIL_SUPPORT__

