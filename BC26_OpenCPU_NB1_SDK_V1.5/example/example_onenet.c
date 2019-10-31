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
 *   example_onenet.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   This example demonstrates how to use onenet function with APIs in OpenCPU.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_ONENET__" in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 * note:
 *     This example shows how to receive hex data using direct push mode.
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
#ifdef __EXAMPLE_ONENET__
#include "custom_feature_def.h"
#include "ql_stdlib.h"
#include "ql_common.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_uart.h"
#include "ql_timer.h"
#include "ril_network.h"
#include "ril_onenet.h"
#include "ril.h"
#include "ril_util.h"
#include "ril_system.h"
#include "ql_urc_register.h"


#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT0
#define DBG_BUF_LEN   1024
static char DBG_BUFFER[DBG_BUF_LEN];
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif


/*****************************************************************
* define process state
******************************************************************/
typedef enum{
    STATE_NW_QUERY_STATE,
    STATE_ONENET_CREATE,
    STATE_ONENET_ADDOBJ,
    STATE_ONENET_OPEN,
    STATE_ONENET_OBSERVER_RSP,
    STATE_ONENET_DISCOVER_RSP,
    STATE_ONENET_CFG,
    STATE_ONENET_NOTIFY,
    STATE_ONENET_UPDATE,
    STATE_ONENET_WRITE_RSP,
    STATE_ONENET_CLOSE,
    STATE_TOTAL_NUM
}Enum_ONENETSTATE;
static u8 m_onenet_state = STATE_NW_QUERY_STATE;


typedef struct{
 u32 ref;             // Instance ID of OneNET communication suite.
 u32 observe_msg_id;  //Message ID for business processing.
 u32 discover_msg_id; //Only reply to URC of  ��+MIPLDISCOVER��
 u32 write_msg_id;
 u32 execute_msg_id;
 u32 write_Attributes_msg_id;
 u32 recv_length;
 u32 obj_id;   
 u32 ins_id;  
 u32  res_id;   
}Onenet_Param_t;


/*****************************************************************
* ONENET  timer param
******************************************************************/
#define ONENET_TIMER_ID         TIMER_ID_USER_START
#define ONENET_TIMER_PERIOD     500


/*****************************************************************
* buffer Param
******************************************************************/
static Enum_SerialPort m_myUartPort  = UART_PORT0;

#define SRVADDR_BUFFER_LEN  100
#define SEND_BUFFER_LEN     1024
#define ONENET_TEMP_BUFFER_LENGTH 100

static u8 m_send_buf[SEND_BUFFER_LEN]={0};

/*****************************************************************
*  onenet Param
******************************************************************/
ST_ONENET_Obj_Param_t onenet_obj_param_t = {0,0,0,0,NULL,0,0};//add object relate parameters.

Onenet_Urc_Param_t* onenet_urc_param_ptr = NULL;  //Save the received URC information in this variable.

Onenet_Param_t onenet_param_t= {0,0,0,0,0,0,0,0,0,0,0}; //deal with business  parameters.

volatile bool observe_flag = FALSE; //Indicates whether or not to observe.
/*****************************************************************
* uart callback function
******************************************************************/
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);

/*****************************************************************
* timer callback function
******************************************************************/
static void Callback_Timer(u32 timerId, void* param);

/*****************************************************************
* onenet recv callback function
******************************************************************/
static void callback_onenet_req(u8* buffer,u32 length);

/*****************************************************************
* other subroutines
******************************************************************/
static s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/u8* pBuffer, /*[in]*/u32 bufLen);
static void proc_handle(u8 *pData,s32 len);
static s32 ret;

void proc_main_task(s32 taskId)
{
    ST_MSG msg;

    // Register & open UART port
    Ql_UART_Register(m_myUartPort, CallBack_UART_Hdlr, NULL);
    Ql_UART_Open(m_myUartPort, 115200, FC_NONE);

    APP_DEBUG("<--OpenCPU: onenet Client.-->\r\n");

    //register & start timer 
    Ql_Timer_Register(ONENET_TIMER_ID, Callback_Timer, NULL);

	//register  recv callback
    ret = Ql_Onenet_Req_Register(callback_onenet_req);
	APP_DEBUG("<--register recv callback successful(%d)-->\r\n",ret);

    while(TRUE)
    {
        Ql_OS_GetMessage(&msg);
        switch(msg.message)
        {
#ifdef __OCPU_RIL_SUPPORT__
        case MSG_ID_RIL_READY:
            APP_DEBUG("<-- RIL is ready -->\r\n");
            Ql_RIL_Initialize();
            break;
#endif
		case MSG_ID_URC_INDICATION:
		{     
			switch (msg.param1)
            {
    		    case URC_SIM_CARD_STATE_IND:
    			APP_DEBUG("<-- SIM Card Status:%d -->\r\n", msg.param2);
				if(SIM_STAT_READY == msg.param2)
				{
                    Ql_Timer_Start(ONENET_TIMER_ID, ONENET_TIMER_PERIOD, TRUE);//start timer
				}
    			break;	
				case URC_ONENET_EVENT:// Indication for  Notify the TE of Event.
				{
					onenet_urc_param_ptr = msg.param2;
					if(onenet_urc_param_ptr->evtid == EVENT_UPDATE_SUCCESS)
					{
  
                       APP_DEBUG("<- event update success-->\r\n",onenet_urc_param_ptr->ref,onenet_urc_param_ptr->evtid);
					}

     			    APP_DEBUG("<- recv event, ref(%d),evtid(%d)-->\r\n",onenet_urc_param_ptr->ref,onenet_urc_param_ptr->evtid);
				}
				break;
				case URC_ONENET_OBSERVE:// Indication for  Notify the TE an Observe Request.
				{
					onenet_urc_param_ptr = msg.param2;
					if( onenet_obj_param_t.ref  == onenet_urc_param_ptr->ref &&observe_flag ==FALSE)
					{
     					APP_DEBUG("<--urc_onenet_observer,ref(%d),msg_id(%d),obj_id(%d),ins_id(%d),res_id(%d)-->\r\n",\
						onenet_urc_param_ptr->ref,onenet_urc_param_ptr->msgid,onenet_urc_param_ptr->objid,\
						onenet_urc_param_ptr->insid,onenet_urc_param_ptr->resid);

						onenet_param_t.ref = onenet_urc_param_ptr->ref;//Instance ID of OneNET communication suite.
						onenet_param_t.observe_msg_id = onenet_urc_param_ptr->msgid;//The message id of packet.
						onenet_param_t.obj_id  = onenet_urc_param_ptr->objid;//The object identifier,eg 3203
						onenet_param_t.ins_id = onenet_urc_param_ptr->insid; //Instance identifier,eg 0

						observe_flag = TRUE;
     					m_onenet_state = STATE_ONENET_OBSERVER_RSP;
					}
				}
				break;
    		    case URC_ONENET_DISCOVER:// Indication for Notify the TE to Respond the Discover Request
				{
					onenet_urc_param_ptr = msg.param2;
			     	if( onenet_obj_param_t.ref  == onenet_urc_param_ptr->ref)
					{
     					APP_DEBUG("<--urc_onenet_discover,ref(%d),msg_id(%d),obj_id(%d)-->\r\n", \
						onenet_urc_param_ptr->ref,onenet_urc_param_ptr->msgid,onenet_urc_param_ptr->objid);

						onenet_param_t.ref = onenet_urc_param_ptr->ref;
						onenet_param_t.discover_msg_id = onenet_urc_param_ptr->msgid;

     					m_onenet_state = STATE_ONENET_DISCOVER_RSP;
					}
    		    }
    			break;
		        default:
    		    //APP_DEBUG("<-- Other URC: type=%d\r\n", msg.param1);
    	        break;
			}
		}
		break;
	default:
         break;
        }
    }
}


static void callback_onenet_req(u8* buffer,u32 length)
{
	u8 strTmp[50]; 
	char* p = NULL;
	Ql_UART_Write(DEBUG_PORT,buffer,length);
	//APP_DEBUG("<--buffer(%s),length(%d)-->\r\n",buffer,length);
	p = Ql_strstr(buffer,"\r\n+MIPLWRITE:");
	if(p)
	{
		Ql_UART_Write(m_myUartPort,buffer,length);
	    Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	    QSDK_Get_Str(buffer,strTmp,0);
	    onenet_param_t.ref= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	    QSDK_Get_Str(buffer,strTmp,1);
	    onenet_param_t.write_msg_id= Ql_atoi(strTmp);
		
		APP_DEBUG("<--ref = %d, msgid = %d-->\r\n",onenet_param_t.ref, onenet_param_t.write_msg_id);
	}
	p = Ql_strstr(buffer,"\r\n+MIPLEXECUTE:");
	if(p)
	{
		Ql_UART_Write(m_myUartPort,buffer,length);
	    Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	    QSDK_Get_Str(buffer,strTmp,0);
	    onenet_param_t.ref= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	    QSDK_Get_Str(buffer,strTmp,1);
	    onenet_param_t.execute_msg_id= Ql_atoi(strTmp);
		
		APP_DEBUG("<--ref = %d, msgid = %d-->\r\n",onenet_param_t.ref, onenet_param_t.execute_msg_id);
	}
	p = Ql_strstr(buffer,"\r\n+MIPLPARAMETER:");
	if(p)
	{
		Ql_UART_Write(m_myUartPort,buffer,length);
	    Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	    QSDK_Get_Str(buffer,strTmp,0);
	    onenet_param_t.ref= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	    QSDK_Get_Str(buffer,strTmp,1);
	    onenet_param_t.write_Attributes_msg_id= Ql_atoi(strTmp);
		
		APP_DEBUG("<--ref = %d, msgid = %d-->\r\n",onenet_param_t.ref, onenet_param_t.write_Attributes_msg_id);
	}

}

static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
    switch (msg)
    {
    case EVENT_UART_READY_TO_READ:
        {
           break;
        }
    case EVENT_UART_READY_TO_WRITE:
        break;
    default:
        break;
    }
}

static void Callback_Timer(u32 timerId, void* param)
{
    if (ONENET_TIMER_ID == timerId)
    {
        switch (m_onenet_state)
        {        
            case STATE_NW_QUERY_STATE:
            {
                s32 cgreg = 0;
                ret = RIL_NW_GetEGPRSState(&cgreg);
                APP_DEBUG("<--Network State:cgreg=%d-->\r\n",cgreg);
                if((cgreg == NW_STAT_REGISTERED)||(cgreg == NW_STAT_REGISTERED_ROAMING))
                {
                    m_onenet_state = STATE_ONENET_CREATE;
                }
                break;
            }

            case STATE_ONENET_CREATE:
            {
                ret = RIL_QONENET_Create();
                if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<-Onenet Create successfully-->\r\n");
                    m_onenet_state = STATE_ONENET_ADDOBJ;
                }else
                {
                    APP_DEBUG("<--Onenet Create failure, error=%d.-->\r\n",ret);
                }
                break;
            }

			case STATE_ONENET_ADDOBJ:
            {
	
                onenet_obj_param_t.ref = 0;
			    onenet_obj_param_t.obj_id = 3203;
			    onenet_obj_param_t.ins_count = 1;//instance count
			    
			    //Attribute count, which indicates the count of readable and/or writeable resources.
			    onenet_obj_param_t.attrcount = 4;

				//Action count, which indicates the count of executable resources.
			    onenet_obj_param_t.actcount = 1;

				onenet_obj_param_t.insbitmap=(u8*) Ql_MEM_Alloc(sizeof(u8)*ONENET_TEMP_BUFFER_LENGTH);
				Ql_memset(onenet_obj_param_t.insbitmap,0,ONENET_TEMP_BUFFER_LENGTH);
				Ql_strncpy(onenet_obj_param_t.insbitmap,"1\0",Ql_strlen("1\0"));//the instance ID 0 will be registered,
				
                ret = RIL_QONENET_Addobj(&onenet_obj_param_t);
                if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<-- Onenet Add object successfully\r\n");
                    m_onenet_state = STATE_ONENET_OPEN;
                }
				else
                {
                    APP_DEBUG("<--Onenet Add object failure,error=%d.-->\r\n",ret);
                }

				Ql_MEM_Free(onenet_obj_param_t.insbitmap);
				onenet_obj_param_t.insbitmap = NULL;
                break;
            }
            case STATE_ONENET_OPEN:
            {
                u32 lifetime = 129600;    //Lifetime, unit is second.
                ret = RIL_QONENET_Open(onenet_obj_param_t.ref,lifetime);
			    if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<-- Onenet Open successfully\r\n");
                    m_onenet_state = STATE_TOTAL_NUM;
					
                }else
                {
                    APP_DEBUG("<--Onenet Open failure,error=%d.-->\r\n",ret);
                }
                break;
            }
            case STATE_ONENET_OBSERVER_RSP:
            {
				ST_ONENET_Observe_Param_t onenet_observe_param_t;
				
				onenet_observe_param_t.ref = onenet_param_t.ref;
				onenet_observe_param_t.msgid = onenet_param_t.observe_msg_id;
                onenet_observe_param_t.obderve_result= ONENET_OBSERVE_RESULT_1;//2.04 Content, indicate the correct result.
                ret = RIL_QONENET_Observer_Rsp(&onenet_observe_param_t);
			    if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<--Onenet Observer respond successfully\r\n");
					observe_flag = FALSE;
                    m_onenet_state = STATE_TOTAL_NUM;
					
                }else
                {
                    APP_DEBUG("<--Onenet Observer respond failure,error=%d.-->\r\n",ret);
                }
                break;
            }
             case STATE_ONENET_DISCOVER_RSP:
            {
				ST_ONENET_Discover_Rsp_Param_t onenet_discover_rsp_param_t;
				
				onenet_discover_rsp_param_t.ref = onenet_param_t.ref;
				onenet_discover_rsp_param_t.msgid = onenet_param_t.discover_msg_id;
                onenet_discover_rsp_param_t.result= ONENET_OBSERVE_RESULT_1;//2.05 Content, indicate the correct result.
				onenet_discover_rsp_param_t.length = Ql_strlen("5750\0");//resource id  

				onenet_discover_rsp_param_t.value_string=(u8*) Ql_MEM_Alloc(sizeof(u8)*ONENET_TEMP_BUFFER_LENGTH);
				Ql_memset(onenet_discover_rsp_param_t.value_string,0,ONENET_TEMP_BUFFER_LENGTH);
				
				Ql_strncpy(onenet_discover_rsp_param_t.value_string,"5750\0",Ql_strlen("5750\0"));

				onenet_param_t.res_id = Ql_atoi(onenet_discover_rsp_param_t.value_string);

				ret = RIL_QONENET_Discover_Rsp(&onenet_discover_rsp_param_t);
			    if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<--Onenet discover respond successfully\r\n");
                    m_onenet_state = STATE_ONENET_CFG;
					
                }else
                {
                    APP_DEBUG("<--Onenet discover failure,error=%d.-->\r\n",ret);
                }
                Ql_MEM_Free(onenet_discover_rsp_param_t.value_string);
				onenet_discover_rsp_param_t.value_string = NULL;

                break;
            }
			case STATE_ONENET_CFG:
            {
				 ST_ONENET_Config_Param_t onenet_config_param;
				 onenet_config_param.onenet_access_mode =ONENET_ACCESS_MODE_DIRECT;//direct push mode
				 onenet_config_param.onenet_recv_mode = ONENET_RECV_MODE_HEX;  //hex data
				ret = RIL_QONENET_Config(&onenet_config_param,FALSE);
                if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<--Configure Optional Parameters successfully\r\n");
                    m_onenet_state = STATE_ONENET_NOTIFY;
                }else
                {
                    APP_DEBUG("<--Configure Optional Parameters failure,error=%d.-->\r\n",ret);
                }
                break;
            }
            case STATE_ONENET_NOTIFY:
            {
				ST_ONENET_Notify_Param_t onenet_notify_param_t;
				onenet_notify_param_t.ref = onenet_param_t.ref;
				onenet_notify_param_t.msgid = onenet_param_t.observe_msg_id;
			    onenet_notify_param_t.objid =  onenet_param_t.obj_id; //eg  3203
			    onenet_notify_param_t.insid =  onenet_param_t.ins_id; //eg  0
			    onenet_notify_param_t.resid = onenet_param_t.res_id;  //eg 5750
			    onenet_notify_param_t.value_type = ONENET_VALUE_TYPE_STRING; //send data mode.
			    onenet_notify_param_t.len = Ql_strlen("n1\0");//data

				onenet_notify_param_t.value=(u8*) Ql_MEM_Alloc(sizeof(u8)*ONENET_TEMP_BUFFER_LENGTH);
				Ql_memset(onenet_notify_param_t.value,0,ONENET_TEMP_BUFFER_LENGTH);
				
				Ql_strncpy(onenet_notify_param_t.value,"n1\0",Ql_strlen("n1\0")); 
			    onenet_notify_param_t.index = 0;
			    onenet_notify_param_t.flag =0;
			    //onenet_notify_param_t.ackid = 0
                ret = RIL_QONENET_Notify(&onenet_notify_param_t,FALSE);
                if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<-- Onenet notify Request successfully\r\n");
					 m_onenet_state = STATE_ONENET_UPDATE;
                }else
                {
                    APP_DEBUG("<--Onenet notify Request failure,error=%d.-->\r\n",ret);
                }
			    Ql_MEM_Free(onenet_notify_param_t.value);
				onenet_notify_param_t.value = NULL;
                break;
            }
			
			case STATE_ONENET_UPDATE:
            {
				u32 lifttime = 129600;
                ret = RIL_QONENET_Update(onenet_param_t.ref,lifttime,0) ;
                if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<-- send data successfully\r\n");
                    m_onenet_state = STATE_TOTAL_NUM;
					
                }else
                {
                    APP_DEBUG("<--send data failure,error=%d.-->\r\n",ret);
					m_onenet_state = STATE_ONENET_CLOSE;
                }
                break;
            }
			case STATE_ONENET_WRITE_RSP:// Indication for Notify the TE to Respond the Write Request 
			{
				ST_ONENET_Write_Rsp_Param_t onenet_write_rsp_param_t;
				
				onenet_write_rsp_param_t.ref = onenet_param_t.ref;
				onenet_write_rsp_param_t.msgid = onenet_param_t.write_msg_id;
				onenet_write_rsp_param_t.result = ONENET_OBSERVE_RESULT_2;
				
				ret = RIL_QONENET_Write_Rsp(&onenet_write_rsp_param_t) ;
                if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<-- send write respond successfully\r\n");					
					m_onenet_state = STATE_TOTAL_NUM;					
                }
				else
                {
                    APP_DEBUG("<--send write respond failure,error=%d.-->\r\n",ret);
                }
				break;
		    }
			case STATE_ONENET_CLOSE:
            {
                ret = RIL_QONENET_CLOSE(onenet_param_t.ref);
                if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("<-- close onenet successfully\r\n");
					Ql_Timer_Stop(ONENET_TIMER_ID);
                    m_onenet_state = STATE_TOTAL_NUM;
                }else
                {
                    APP_DEBUG("<--close onenet failure,error=%d.-->\r\n",ret);
                }
                break;
            }
			case STATE_TOTAL_NUM:
            {
              //APP_DEBUG("<--onenet test case-->\r\n",ret);
			  m_onenet_state = STATE_TOTAL_NUM;
            }
            default:
                break;
        }    
    }
}

#endif // __EXAMPLE_ONENET__

