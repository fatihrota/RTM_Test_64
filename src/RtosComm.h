#pragma once
/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RtosComm.h
 * @brief       The RtosComm class that hold required information and functions
 *				to operate connection and messaging with Real Time Operating System.
 *              It also contains event handler, notification mechanism etc.
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
#include <RtosLib.h>
#include <stdint.h>
#include "EcDemoApp.h"
/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

#if !defined( TEXT )
#define TEXT( A )               A
#endif

#define MSGQUEUE_DATA_NAME_SEND			TEXT("MsgQueue_Data_FromRTMtoNRTM")
#define MSGQUEUE_DATA_NAME_RCV			TEXT("MsgQueue_Data_FromNRTMtoRTM")

#define DATA_SEND_MSQ_SIZE			10
#define DATA_RCV_MSQ_SIZE			8000

#define SERVER_OSID                 0 /* OS 0 = Rtos */
#define SERVER_PORT                 7
#define RTOS_MSG_SIZE				8000

#define MAX_ETHERCAT_MSG_SIZE			200

/*============================================================================*/
/* Type definitions                                                           */
/*============================================================================*/

/*============================================================================*/
/* Global data                                                                */
/*============================================================================*/

/*============================================================================*/
/* Declarations                                                               */
/*============================================================================*/

class RtosComm
{
public:

	virtual ~RtosComm();

	static RtosComm* getInstance();

	static void cleanInstance();

	/**
	 * Initialize RTOS Library.
	 *
	 * @return
	 */
	INT32 libInit(void);
	/**
	 * Create Send Message Qeueu to send data to RTM. Sent data will be directed
	 * to Compact Controller Device by RTM.
	 *
	 * @return
	 */
	INT32 create_Data_SendMessageQueue();

	/**
	 * Create Receive Message Queue to receive data from RTM. Received Data is sent by
	 * Compact Controller device
	 *
	 * @return
	 */
	INT32 create_Data_ReceiveMessageQueue();

	RTOSLIB_HANDLE      hSocket = NULL;
	RTOSLIB_HANDLE      hSocketClient = NULL;

	UINT8 connectedRTOS = 0;
	INT32 createSocket(void);
	INT32 bindSocket(void);
	INT32 listenSocket(void);
	INT32 acceptClient(void);
	INT32 receiveMessage(void);
	INT32 sendMessage(int senDataLen);

	UINT8 rtosSendArray[RTOS_MSG_SIZE];
	UINT8 rtosRcvArray[RTOS_MSG_SIZE];

	RTOSLIB_HANDLE          hQueue_data_snd = NULL;	/* RTOS Message Queue handler for outgoing messages */
	RTOSLIB_HANDLE          hQueue_data_rcv = NULL; /* RTOS Message Queue handler for incoming messages */

	RTOSMSGQUEUE_INFO       InfoMsq_data_snd;	/* RTOS Message Queue information for outgoing messages */
	RTOSMSGQUEUE_INFO       InfoMsq_data_rcv; /* RTOS Message Queue information for incoming messages */

private:
	RtosComm();
	static RtosComm* instance;
};

/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/

