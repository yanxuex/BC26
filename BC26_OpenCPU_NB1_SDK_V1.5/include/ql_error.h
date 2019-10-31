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
 *   ql_error.h 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   error code  defines.
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


#ifndef __QL_ERROR_H__
#define __QL_ERROR_H__

/****************************************************************************
 * Error Code Definition
 ***************************************************************************/
enum {
    QL_RET_OK           = 0,
    QL_RET_ERR_PARAM = -1,
    QL_RET_ERR_PORT_NOT_OPEN = -2,
    QL_RET_ERR_TIMER_FULL =  -3,
    QL_RET_ERR_INVALID_TIMER = -4,
    QL_RET_ERR_FATAL = -5,
    QL_RET_ERR_INVALID_OP = -6,
    QL_RET_ERR_UART_BUSY = -7,    
    QL_RET_ERR_INVALID_PORT = -8,    
    QL_RET_ERR_NOMATCHVERSION        = -9,
    QL_RET_ERR_NOSUPPORTPIN      = -10,
    QL_RET_ERR_NOSUPPORTMODE      = -11,
    QL_RET_ERR_NOSUPPORTEINT      = -12,
    QL_RET_ERR_NOSUPPORTSET      = -13,
    QL_RET_ERR_NOSUPPORTGET      = -14,
    QL_RET_ERR_NOSUPPORTCONTROL      = -15,
    QL_RET_ERR_PINALREADYSUBCRIBE      = -16,
    QL_RET_ERR_BUSSUBBUSY      = -17,
    QL_RET_ERR_NOGPIOMODE      = -18,
    QL_RET_ERR_NORIGHTOPERATE      = -19,
    QL_RET_ERR_ALREADYUNSUBCRIBE      = -20,
    QL_RET_ERR_FULLI2CBUS      = -21,
    QL_RET_ERR_NOTSUPPORTBYHANDLE      = -22,
    QL_RET_ERR_INVALIDBUSHANDLE      = -23,
    QL_RET_ERR_NOEXISTOBJEXT      = -24,
    QL_RET_ERR_OPERATEOBJEXTFAILED      = -25,
    QL_RET_ERR_OPENOBJEXTFAILED      = -26,
    QL_RET_ERR_WRITEOBJEXTFAILED      = -27,
    QL_RET_ERR_READOBJEXTFAILED      = -28,
    QL_RET_ERR_FLASHFULLOVER     = -29,   
    QL_RET_ERR_FLASHSPACE              = -30,
    QL_RET_ERR_DRIVE                        = -31,
    QL_RET_ERR_DRIVEFULLOVER        = -32,
    QL_RET_ERR_INVALIDFLASHID      = -33,
    QL_RET_ERR_I2CHWFAILED      = -34,
    QL_RET_ERR_FILEFAILED      = -35,
    QL_RET_ERR_FILEOPENFAILED      = -36,
    QL_RET_ERR_FILENAMETOOLENGTH = -37,
    QL_RET_ERR_FILEREADFAILED = -38,
    QL_RET_ERR_FILEWRITEFAILED  = -39,
    QL_RET_ERR_FILESEEKFAILED  = -40,
    QL_RET_ERR_FILENOTFOUND  = -41,
    QL_RET_ERR_FILENOMORE  = -42,
    QL_RET_ERR_FILEDISKFULL = -43,
    QL_RET_ERR_INVALID_BAUDRATE = -44,
    QL_RET_ERR_API_NO_RESPONSE = -45,
    QL_RET_ERR_API_INVALID_RESPONSE = -46,
    QL_RET_ERR_SMS_EXCEED_LENGTH =-47,
    QL_RET_ERR_SMS_NOT_INIT = -48,
    QL_RET_ERR_INVALID_TASK_ID = -49,   
    QL_RET_ERR_NOT_IN_BASIC_MODE = -50,
    QL_RET_ERR_INVALID_PARAMETER = -51,
    QL_RET_ERR_PATHNOTFOUND = -52,
    QL_RET_ERR_GET_MEM = -53,
    QL_RET_ERR_GENERAL_FAILURE = -54,
    QL_RET_ERR_FILE_EXISTS = -55,
    QL_RET_ERR_SMS_INVALID_FORMAT = -56,    
    QL_RET_ERR_SMS_GET_FORMAT = -57,        
    QL_RET_ERR_SMS_INVALID_STORAGE = -58,        
    QL_RET_ERR_SMS_SET_STORAGE = -59,            
    QL_RET_ERR_SMS_SEND_AT_CMD = -60,
    QL_RET_ERR_API_CMD_BUSY = -61,  


    Ql_RET_ERR_EINT_USED                        =-300, 
    Ql_RET_ERR_EINT_ALREADY_REGISTERED          = -301, 
    QL_RET_ERR_ADC_ALREADY_REGISTERED           = -302,
    QL_RET_ERR_ADC_NOT_REGISTERED               = -303,
    QL_RET_ERR_ADC_SAMPLING_ALREADY             = -304,
    QL_RET_ERR_CHANNEL_OUTRANGE                 = -305,
    QL_RET_ERR_CHANNEL_USED                     = -306,
    QL_RET_ERR_CHANNEL_NOT_FOUND                = -307,
    QL_RET_ERR_IIC_SLAVE_NOT_FOUND              = -308,
    QL_RET_ERR_IIC_SAME_SLAVE_ADDRESS           = -309,
    QL_RET_ERR_IIC_SLAVE_TOO_MANY               = -310,
    QL_RET_ERR_IIC_NOT_IIC_CONTROLER            = -311,
    QL_RET_ERR_FULLSPIBUS                       = -312,
    

    Ql_RET_ERR_SIM_NOT_INSERTED                =-400,
    Ql_RET_ERR_SIM_TYPE_ERROR                  =-401, 

    /* SMS -500 ~ -550*/  
    QL_RET_ERR_SMS_ERROR = -500,
    QL_RET_ERR_SMS_NOT_INITIAL = -501,
    Ql_RET_ERR_SMS_NOT_READY = -502,
    QL_RET_ERR_SMS_INVALID_PARAM = -503,
    QL_RET_ERR_SMS_OUT_OF_MEMORY = -504,
    QL_RET_ERR_SMS_INCORRECT_DATA_LENGTH = -505,
    QL_RET_ERR_SMS_PDU_SYNTEX_ERROR = -506,
    QL_RET_ERR_SMS_INVALID_MEM_INDEX = -507,
    QL_RET_ERR_SMS_CMD_CONFLICT = -508,
    QL_RET_ERR_SMS_MSG_EMPTY = -509,
    QL_RET_ERR_SMS_INVALID_NUMBER_STRING = -510,
    QL_RET_ERR_SMS_INVALID_TEXT_CONTENT = -511,
    QL_RET_ERR_SMS_NOT_SUPPORTED = -512,
    QL_RET_ERR_SMS_INCORRECT_FORMAT = -513,
    QL_RET_ERR_SMS_STORAGE_FULL = -514,

 
     /* Flash OPT End */

    Ql_RET_ERR_SYS_NOT_READY = -9998,
    Ql_RET_ERR_UNKOWN = -9999,
    Ql_RET_NOT_SUPPORT = -10000,
};

#endif // End-of QL_ERROR_H 

