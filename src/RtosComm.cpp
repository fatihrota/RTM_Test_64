/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RtosComm.cpp
 * @brief       Implementation of the RtosComm class of the library.
 *
 * @author      Mehmet Fatih Özay
 * @date        2022-03-30
 *
 * @ingroup     RtosComm
 * @{
 *****************************************************************************/

 /*============================================================================*/
 /* Includes                                                                   */
 /*============================================================================*/

#ifndef pEcLogParms
#define pEcLogParms G_pEcLogParms
#endif

#include "RtosComm.h"
#include <iostream>
#include <stdint.h>
#include "RtosMessage.h"


/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

/*============================================================================*/
/* Type definitions                                                           */
/*============================================================================*/

/*============================================================================*/
/* Global data                                                                */
/*============================================================================*/
RtosComm* RtosComm::instance = 0;

/*============================================================================*/
/* Module global data                                                         */
/*============================================================================*/

/*============================================================================*/
/* Implementation of functions                                                */
/*============================================================================*/

RtosComm::RtosComm() {

}

RtosComm::~RtosComm() {

}

/******************************************************************************/
RtosComm* RtosComm::getInstance() {
	if (instance == 0) {
		instance = new RtosComm();
	}

	return instance;
}

/******************************************************************************/
void RtosComm::cleanInstance() {
	if (instance)
		delete instance;
	instance = NULL;
}


/******************************************************************************/
INT32 RtosComm::libInit(void)
{
	UINT32 dwRetVal;
	/* RTOS lib Init */
	dwRetVal = RtosLibInit();
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR initializing RTOS Library (0x%X)\n", dwRetVal));
		return -1;
	}

	return 0;
}

/******************************************************************************/
INT32 RtosComm::create_Data_SendMessageQueue(void)
{
	UINT32 dwRetVal = 0;

	RTOSMSGQUEUE_OPTIONS    MsgQueueOptions;
	/* Create message queue from NRTM to RTM to send EtherCAT msg*/
	MsgQueueOptions.dwSize = sizeof(MsgQueueOptions);
	MsgQueueOptions.dwFlags = RTOSMSGQUEUE_OPTIONS_FLAG_ALLOWBROKEN;
	MsgQueueOptions.dwNumMessages = DATA_SEND_MSQ_SIZE;
	MsgQueueOptions.dwMsgDataSizeInBytes = MAX_ETHERCAT_MSG_SIZE;
	MsgQueueOptions.bReadAccess = FALSE;
	dwRetVal = RtosMsgQueueCreate(MSGQUEUE_DATA_NAME_SEND, &MsgQueueOptions, &hQueue_data_snd);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Failed to create message queue %s (0x%X)\n", MSGQUEUE_DATA_NAME_SEND, dwRetVal));
		return -1;
	}

	InfoMsq_data_snd.dwSize = sizeof(InfoMsq_data_snd);

	printf("Snd MSQ Queue Size : %d\n", dwTotalPdSizeIn);
	return 0;
}

/******************************************************************************/
INT32 RtosComm::create_Data_ReceiveMessageQueue(void)
{
	UINT32 dwRetVal;

	RTOSMSGQUEUE_OPTIONS    MsgQueueOptions;
	/* Create message queue from RTM to NRTM to receive EtherCAT msg*/
	MsgQueueOptions.dwSize = sizeof(MsgQueueOptions);
	MsgQueueOptions.dwFlags = RTOSMSGQUEUE_OPTIONS_FLAG_ALLOWBROKEN;
	MsgQueueOptions.dwNumMessages = DATA_RCV_MSQ_SIZE;
	MsgQueueOptions.dwMsgDataSizeInBytes = MAX_ETHERCAT_MSG_SIZE;
	MsgQueueOptions.bReadAccess = TRUE;
	dwRetVal = RtosMsgQueueCreate(MSGQUEUE_DATA_NAME_RCV, &MsgQueueOptions, &hQueue_data_rcv);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Failed to create message queue %s (0x%X)\n", MSGQUEUE_DATA_NAME_RCV, dwRetVal));
		return -1;
	}

	InfoMsq_data_rcv.dwSize = sizeof(InfoMsq_data_rcv);

	printf("Rcv MSQ Queue Size : %d\n", dwTotalPdSizeOut);
	return 0;
}

/******************************************************************************/
INT32 RtosComm::sendMessage(INT32 sendDataLen)
{
	UINT32          dwRetVal = 0;
	UINT32          dwWritten = 0;

	dwRetVal = RtosSocketSend(hSocketClient, (UINT8*)&rtosSendArray[0], sendDataLen, &dwWritten);

	switch (RTE_ERROR_GET_ERROR(dwRetVal))
	{
	case RTE_SUCCESS:
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "Send message success : %d\n", dwWritten));
		break;
	case RTE_ERROR_TIMEOUT:
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Send message failed = timeout : %d\n", dwWritten));
		break;
	default:
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Send message failed = default : %d\n", dwWritten));
		break;
	}

	return dwWritten;
}
/******************************************************************************/
INT32 RtosComm::receiveMessage(void)
{
	UINT32  dwRetVal;
	UINT32  dwRes;
	UINT32  dwBytesToRead = NRTM_MESSAGE_TOTAL_SIZE;
	UINT32  dwBytesRead = 0;
	UINT32  dwBytesReadNew;

	memset(&rtosRcvArray[0], 0, NRTM_MESSAGE_TOTAL_SIZE);
	dwRes = RtosSocketRecv( hSocketClient, &rtosRcvArray[dwBytesRead], (dwBytesToRead-dwBytesRead), &dwBytesReadNew );

	if (!dwBytesReadNew)
	{
		return 0;
	}

	RtosMessage rtmMsg;

	rtmMsg.parseReceivedMsg(&rtosRcvArray[0]);

	return 0;
}

/******************************************************************************/
INT32 RtosComm::acceptClient(void)
{
	UINT32          dwRetVal;
	RTOSSOCKET_ADDR AddrNew;
	AddrNew.bySize = sizeof(RTOSSOCKET_ADDR);

	dwRetVal = RtosSocketAccept(hSocket, &hSocketClient, &AddrNew, 1000);
	switch (RTE_ERROR_GET_ERROR(dwRetVal))
	{
	case RTE_SUCCESS:
		printf("Connected\n");
		connectedRTOS = 1;
		break;
	case RTE_ERROR_TIMEOUT:
		connectedRTOS = 0;
		break;
	default:
		connectedRTOS = 0;
		break;
	}

	return 0;
}

/******************************************************************************/
INT32 RtosComm::listenSocket(void)
{
	UINT32  dwRetVal;
	dwRetVal = RtosSocketListen(hSocket);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, " %-28s : Error RtosSocketListen (0x%X)\n", "Server Tcp", dwRetVal));
	}

	return 0;
}

/******************************************************************************/
INT32 RtosComm::bindSocket(void)
{
	UINT32  dwRetVal;
	dwRetVal = RtosSocketBind(hSocket, SERVER_PORT);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, " Error RtosSocketBind (0x%X)\n", dwRetVal));

	}

	return 0;
}

/******************************************************************************/
INT32 RtosComm::createSocket(void)
{
	UINT32 dwRetVal;

	dwRetVal = RtosSocketCreate(RTOSSOCKET_FAMILY_RTE, RTOSSOCKET_TYPE_STREAM, RTOSSOCKET_PROTOCOL_TCP, &hSocket);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Failed to create socket (0x%X)\n", MSGQUEUE_DATA_NAME_RCV, dwRetVal));
		return -1;
	}

	return 0;
}

/**@}*/
/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
