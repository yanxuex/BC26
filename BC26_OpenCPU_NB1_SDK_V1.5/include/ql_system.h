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
 *   ql_system.h 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *  System related APIs
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
 

#ifndef __QL_SYSTEM_H__
#define __QL_SYSTEM_H__
#include "ql_common.h"
#include "ql_type.h"
#include "ql_memory.h"


/**************************************************************
 * Message Structure Definition
 **************************************************************/
typedef struct {
    u32  message;
    u32  param1;
    u32  param2;
    u32  srcTaskId;
} ST_MSG;

/**************************************************************
 * User Message ID Definition
 **************************************************************/
#define     MSG_ID_USER_START       0x1000
#define     MSG_ID_RIL_READY        (MSG_ID_USER_START + 1)
#define     MSG_ID_URC_INDICATION   (MSG_ID_USER_START + 2)
#define     MSG_ID_APP_TEST         (MSG_ID_USER_START + 3)


typedef enum {
	EVENT_FLAG0 = 0x00000001,
	EVENT_FLAG1 = 0x00000002,
	EVENT_FLAG2 = 0x00000004,
	EVENT_FLAG3 = 0x00000008,
	EVENT_FLAG4 = 0x00000010,
	EVENT_FLAG5 = 0x00000020,
	EVENT_FLAG6 = 0x00000040,
	EVENT_FLAG7 = 0x00000080,
	EVENT_FLAG8 = 0x00000100,
	EVENT_FLAG9 = 0x00000200,
	EVENT_FLAG_END
}Enum_EventFlag;

/**************************************************************
 * Error Code Definition
 **************************************************************/
typedef enum {
    OS_SUCCESS,
    OS_ERROR,
    OS_Q_FULL,
    OS_Q_EMPTY,
    OS_SEM_NOT_AVAILABLE,
    OS_WOULD_BLOCK,
    OS_MESSAGE_TOO_BIG,
    OS_INVALID_ID,
    OS_NOT_INITIALIZED,
    OS_INVALID_LENGHT,
    OS_NULL_ADDRESS,
    OS_NOT_RECEIVE,
    OS_NOT_SEND,
    OS_MEMORY_NOT_VALID,
    OS_NOT_PRESENT,
    OS_MEMORY_NOT_RELEASE
} Enum_OS_ErrCode;

typedef enum {
    SYS_STATE_START = 0,
    SYS_STATE_ATOK  = 1,
    SYS_STATE_PHBOK = 2,
    SYS_STATE_SMSOK = 3
}Enum_SysInitState;

typedef enum {
    CFUN_STATE_0 = 0,
    CFUN_STATE_1 = 1,
    CFUN_STATE_4 = 4
}Enum_CfunState;



/*****************************************************************
* Function:     Ql_Sleep
*
* Description:
*               This is a delay interface,suspends the execution of the current task
*               until the time-out interval elapses.
* Parameters:
*               msec:
*                   The time interval for which execution is to
*                   be suspended, in milliseconds.
* Return:
*               None
* Note:
*               if  users set the msec to 1, the task will also delay 10 ms, because one tick of the OS is 10 ms.
*****************************************************************/
void Ql_Sleep(u32 msec);

/*****************************************************************
* Function:     Ql_Delay_us
*
* Description:
*               This is a delay interface,blocks the execution of the current task
*               until the time-out interval elapses.
* Parameters:
*               usec:
*                   The time interval for which execution is to
*                   be blocked, in microseconds.
* Return:
*               None
* Note:
*               If users  need to delay too long, such as a few seconds, it's best to use Ql_Sleep,because 
* this API is block.
*****************************************************************/
void Ql_Delay_us(u32 usec);

/*****************************************************************
* Function:     Ql_Reset
*
* Description:
*               This function resets the system.
*
* Parameters:
*               resetType:
*                   must be 0.
* Return:
*               None
*****************************************************************/
void Ql_Reset(u8 resetType);


/*****************************************************************
* Function:     Ql_GetSDKVer
*
* Description:
*               Get the version ID of the SDK.
*
* Parameters:
*               ptrVer:
*                   [out] Point  to a unsigned char buffer, which is
*                           the the version ID of the SDK.
*               len:
*                   [in] A number will be compare with the length of version ID.
*
* Return:
*               The smaller between len and the length of version ID.
*****************************************************************/
s32  Ql_GetSDKVer(u8* ptrVer, u32 len);

/*****************************************************************
* Function:     Ql_OS_GetMessage 
* 
* Description:
*               This function retrieves a message from the current task's 
*               message queue. When there is no message in the task's
*               message queue, the task is in the waiting state..
* Parameters:
*               msg: 
*                   A pointer to ST_MSG.
* Return:        
*               Always return QL_RET_OK.
*****************************************************************/
s32 Ql_OS_GetMessage(ST_MSG* msg);

/*****************************************************************
* Function:     Ql_OS_SendMessage 
* 
* Description:
*               Send message between tasks.

* Parameters:
*               destTaskId: 
*                   One value of Enum_TaskId.
*               msgId: 
*                   User message id, must bigger than 0xFF.
*               param1:
*               param2:
*                   Parameters to send to another task.
* Return:        
*               OS_SUCCESS, sent data to the target task successfully.
*               OS_INVALID_ID, the task id is invalid.
*               OS_NOT_INITIALIZED, the target task is invalid.
*               OS_Q_FULL, the message queue of target task is full.
*****************************************************************/
s32 Ql_OS_SendMessage(s32 destTaskId, u32 msgId, u32 param1, u32 param2);

/*****************************************************************
* Function:     Ql_OS_SendMessageFromISR 
* 
* Description:
*               Send message between tasks,It Supports sending messages in ISR(except callback of psm_eint).

* Parameters:
*               destTaskId: 
*                   One value of Enum_TaskId.
*               msgId: 
*                   User message id, must bigger than 0xFF.
*               param1:
*               param2:
*                   Parameters to send to another task.
* Return:        
*               OS_SUCCESS
*               OS_INVALID_ID
*               OS_MEMORY_NOT_VALID
*               OS_Q_FULL
*****************************************************************/
s32 Ql_OS_SendMessageFromISR(s32 destTaskId, u32 msgId, u32 param1, u32 param2);

/*****************************************************************
* Function:     Ql_OS_CreateMutex 
* 
* Description:
*               Create a mutex with name.

* Parameters:
*               None.
*
* Return:        
*               Mutex Id
*****************************************************************/
u32 Ql_OS_CreateMutex(void);

/*****************************************************************
* Function:     Ql_OS_TakeMutex 
* 
* Description:
*               Obtain an instance of the specified MUTEX.

* Parameters:
*               mutexId: 
*                        Mutex Id
*               block_time:
*                        The time in ticks to wait for the mutex to become  available.unit:ms
* Return:        
*                TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
bool Ql_OS_TakeMutex(u32 mutexId,u32 block_time);

/*****************************************************************
* Function:     Ql_OS_GiveMutex 
* 
* Description:
*               Release the instance of the specified MUTEX.

* Parameters:
*               mutexId: Mutex Id
* Return:        
*               TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
bool Ql_OS_GiveMutex(u32 mutexId);

/*****************************************************************
* Function:     Ql_OS_CreateSemaphore 
* 
* Description:
*               Creates a counting semaphore.
* Parameters:
*               maxCount: 
*                   The maximum count value that can be reached.  When the semaphore reaches this value it can no longer be 'given'.
*               InitialCount: 
*                  The count value assigned to the semaphore when it is created.
* Return:        
*               Value of created semaphore
*****************************************************************/
u32 Ql_OS_CreateSemaphore(u32 maxCount,u32 InitialCount);

/*****************************************************************
* Function:     Ql_OS_TakeSemaphore 
* 
* Description:
*               Obtain an instance of the specified semaphore.

* Parameters:
*               semId:
*                        A handle to the semaphore being taken - obtained when the semaphore was created.
*               block_time: 
*                        wait for the semaphore to become available, unit:ms
* Return:        
*               TRUE: if the semaphore was obtained. 
*               FALSE: if xBlockTime expired without the semaphore becoming available..
*****************************************************************/
bool Ql_OS_TakeSemaphore(u32 semId, u32 block_time);

/*****************************************************************
* Function:     Ql_OS_GiveSemaphore 
* 
* Description:
*               Release the instance of the specified semaphore.

* Parameters:
*               semId: 
*                       A handle to the semaphore being taken - obtained when the semaphore was created.
* Return:        
*                TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
bool Ql_OS_GiveSemaphore(u32 semId);

/*****************************************************************
* Function:     Ql_OS_CreateEvent 
* 
* Description:
* 			This function creates an event-flag group with the specified name.
*			Each event-flag group contains 10 event flags. 
*
* Parameters:
*			None
*
* Return:        
*			An event Id that identify this event uniquely.
*****************************************************************/
u32 Ql_OS_CreateEvent(void);

/*****************************************************************
* Function:     Ql_OS_WaitEvent 
* 
* Description:
* 			This function waits until the specified type of event is in the signaled 
*                  state. Developers can specify different types of events for purposes.
*                  The event flags are defined in 'Enum_EventFlag'.
*
* Parameters:
*			evtId:
*				Event id that is returned by calling Ql_OS_CreateEvent().
*			evtFlag:
*				Event flag type. Please refer to Enum_EventFlag.
*               block_time:
*                         wait for the event to become available, unit:ms
*
* Return:        
*		    Returns the current event flag group value.
*****************************************************************/
s32 Ql_OS_WaitEvent(u32 evtId, u32 evtFlag,u32 block_time);

/*****************************************************************
* Function:     Ql_OS_SetEvent 
* 
* Description:
*                  This function sets the specified event flag. Any task waiting on the
*                  event, whose event flag request is satisfied, is resumed.
*
* Parameters:
*			evtId:
*				Event id that is returned by calling Ql_OS_CreateEvent().
*			evtFlag:
*				Event flag type. Please refer to Enum_EventFlag.
*
* Return:        
*			Returns the current event flag group value.
*****************************************************************/
s32 Ql_OS_SetEvent(u32 evtId, u32 evtFlag);

/*****************************************************************
* Function:     Ql_OS_SetEventFromISR
* 
* Description:
*                  This function sets the specified event flag, can be called from interrupt function. 
*			 Any task waiting on the event, whose event flag request is satisfied, is resumed.
*
* Parameters:
*			evtId:
*				Event id that is returned by calling Ql_OS_CreateEvent().
*			evtFlag:
*				Event flag type. Please refer to Enum_EventFlag.
*
* Return:        
*			Returns the current event flag group value.
*****************************************************************/
s32 Ql_OS_SetEventFromISR(u32 evtId, u32 evtFlag);
/*****************************************************************
* Function:     Ql_SetLastErrorCode 
* 
* Description:
*               Set error code

* Parameters:
*               errCode: Error code
* Return:        
*              TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
s32 Ql_SetLastErrorCode(s32 errCode);

/*****************************************************************
* Function:     Ql_GetLastErrorCode 
* 
* Description:
*               Retrieves the calling task's last-error code value.

* Parameters:
*               None
* Return:        
*               The return value is the calling task's last-error code.
*****************************************************************/
s32 Ql_GetLastErrorCode(void);

/*****************************************************************
* Function:     Ql_OS_GetCurrentTaskPriority 
* 
* Description:
*               Get the priority of the current task

* Parameters:
*               None
* Return:        
*               Task priority,The default value is 4.
*****************************************************************/
u32 Ql_OS_GetCurrentTaskPriority(void);

/*****************************************************************
* Function:     Ql_OS_GetCurrenTaskLeftStackSize 
* 
* Description:
*               Get the left number of bytes in the current task stack
* Parameters:
*               None
* Return:        
*               Number of bytes, ranges from 1024KB to 10*1024KB.
*****************************************************************/
u32 Ql_OS_GetCurrenTaskLeftStackSize(void);

/*****************************************************************
* Function:     Ql_OS_GetActiveTaskId 
* 
* Description:
*               This function returns the task ID of the current task.
* Parameters:
*               None.
* Return:        
*               The Id number of current task.
*****************************************************************/
s32 Ql_OS_GetActiveTaskId(void);

/*****************************************************************
* Function:     Ql_OS_GetTaskTickCount 
* 
* Description:
*               This function returns the task ticks since task start scheduler.
*               1 tick equals 10ms.
* Parameters:
*               None.
* Return:        
*               the count of ticks.
*****************************************************************/
u32 Ql_OS_GetTaskTickCount(void);

/*****************************************************************
* Function:     Ql_OS_GetTaskTickCountFromISR 
* 
* Description:
*               This function returns the task ticks since task start scheduler,it is safe to be called from an ISR.
*               1 tick equals 10ms.
*
* Parameters:
*               None.
* Return:        
*               the count of ticks.
*****************************************************************/
u32 Ql_OS_GetTaskTickCountFromISR(void);


/*****************************************************************
* Function:     Ql_SecureData_Store 
* 
* Description:
*              This function can be used to store some critical user data 
*              to prevent them from losing.
*
*              Note:
*              1.
*              OpenCPU has designed 2 blocks of system storage space to 
*              backup critical user data. Developer may specify the first
*              parameter index [1-2] to specify different storage block. 
*              each blocks can store 2048 bytes 
*              2.
*              User should not call this API function frequently, which is not
*              good for life cycle of flash.
*
* Parameters:
*              index:
*               [in]  the index of the secure data block. The range is: 1~2.
*              
*               pData: 
*                   [in] The data to be backed up. In 1~2 groups, every group can 
*                   save 2048 bytes at most. 
*
*               len:
*                   [in] The length of the user data. The maximum of this value is 2048B.
* Return:       
*               QL_RET_OK, this function succeeds.
*             -1:An unknown error occurred,may be parameter is error.
*             -2:The NVDM found a checksum error when reading the data item
*             -3:No space is available in the flash
*             -4: The data item wasn't found by the NVDM.
              -5:The user parameter is invalid.
*               ......
*****************************************************************/
s32 Ql_SecureData_Store(u8 index , u8* pData, u32 len);

/*****************************************************************
* Function:     Ql_SecureData_Read 
* 
* Description:
*              This functin reads secure data which is previously 
*              stored by Ql_SecureData_Store.
* Parameters:
*               index:
*                   [in] The index of the secure data block. The range is: 1~2.
*
*               len:
*                   [in] The length of the user data. The maximum of this value is 2048B.
* Return:       
*               QL_RET_OK, this function succeeds.
*             -1:An unknown error occurred,may be parameter is error.
*             -2:The NVDM found a checksum error when reading the data item
*             -3:No space is available in the flash
*             -4: The data item wasn't found by the NVDM.
              -5:The user parameter is invalid.

*****************************************************************/
s32 Ql_SecureData_Read(u8 index, u8* pBuffer, u32 len);



/********************************TASK DEFINITION*************************************************/
#ifdef TASK_ENTRYFUNC_DEF
#undef TASK_ENTRYFUNC_DEF
#endif
#ifdef TASK_DEFINITION
#undef TASK_DEFINITION
#endif
#define TASK_ID_DEF
#include "ql_common.h"
TASK_DEFINITION_BEGIN
#include "custom_task_cfg.h"
TASK_DEFINITION_END
#undef TASK_ID_DEF

#endif  // End-of __QL_SYSTEM_H__
