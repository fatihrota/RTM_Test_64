#pragma once
/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RTM_MainApp.h
 * @brief       The main class of the library
 *
 * @author      Mehmet Fatih Ozay
 * @date        2022-02-25
 *
 * @ingroup     RTM_MainApp
 * @{
 *****************************************************************************/

 /*============================================================================*/
 /* Includes                                                                   */
 /*============================================================================*/
#include <iostream>
#include <stdint.h>
#include <vector>
#include "EcDemoApp.h"
#include <rtosLib.h>
#include "RTM_TestRun.h"
#include "RTM_Message.h"
/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

#define MAX_ETHERCAT_MSG_SIZE			200
#define ECAT_MSG_SIZE_PER_CC			21

#define MSG_SIGNATURE               0x08150816
#define SOCKET_MSG_SIZE               4000

#define SERVER_OSID                 0 /* OS 0 = Rtos */
#define SERVER_PORT                 6
/*============================================================================*/
/* Type definitions                                                           */
/*============================================================================*/

/*============================================================================*/
/* Global data                                                                */
/*============================================================================*/

/*============================================================================*/
/* Declarations                                                               */
/*============================================================================*/
#if !defined( TEXT )
#define TEXT( A )               A
#endif

class RTM_MainApp
{
public:
	virtual ~RTM_MainApp();

	static RTM_MainApp* getInstance();

	static void cleanInstance();

	RTOSMSGQUEUE_OPTIONS    MsgQueueOptions_Data_fromNRTM;
	RTOSLIB_HANDLE          hQueue_Data_fromNRTM  = NULL;
	RTOSMSGQUEUE_INFO       Info_Data_fromNRTM;

	RTOSMSGQUEUE_OPTIONS    MsgQueueOptions_Data_toNRTM;
	RTOSLIB_HANDLE          hQueue_Data_toNRTM  = NULL;
	RTOSMSGQUEUE_INFO       Info_Data_toNRTM;

	RTOSLIB_HANDLE  hSocket       = NULL;
	RTOSLIB_HANDLE  hSocketClient    = NULL;

	uint8_t nrtmConnected = 0;

	EC_T_VOID acceptNRTM(EC_T_VOID);
	EC_T_VOID createTestThreads(EC_T_VOID* pvAppContext);
	EC_T_VOID createSocketWithNRTM(EC_T_VOID);
	int takeMsgFromNRTM(EC_T_VOID);
	UINT32 DataRx(RTOSLIB_HANDLE hSocket,PRTOSSOCKET_ADDR pAddrFrom,BOOL bClient,TCHAR* szMsgSrc);
	UINT32 DataParse(BOOL bClient,TCHAR* szMsgSrc);
	EC_T_VOID connectNRTM(EC_T_VOID);

	EC_T_VOID createMsgQueueWithNRTM(EC_T_VOID);
	EC_T_VOID copyRcvdEthercatMsgToBuffer(EC_T_BYTE* ecatMsg);
	EC_T_VOID takeDataFromMsgQueue(EC_T_VOID);
	EC_T_VOID triggerTests(EC_T_VOID);
	EC_T_VOID copySendBufferToEthercat(EC_T_BYTE* ecatMsg);
	EC_T_BYTE sendEtherCatArray[MAX_ETHERCAT_MSG_SIZE];
	EC_T_BYTE receivedEtherCatArray[MAX_ETHERCAT_MSG_SIZE];

	EC_T_BYTE rcvSignalFromNRTM[MAX_ETHERCAT_MSG_SIZE];

	nrtm_msg    nrtmMsg   = {0};
	uint8_t rtosSendArray[NRTM_MESSAGE_TOTAL_SIZE];
	uint8_t rtosRcvArray[NRTM_MESSAGE_TOTAL_SIZE];

	std::vector<RTM_TestRun> testVector;	/* Holds seperate tests */
private:
	RTM_MainApp();
	static RTM_MainApp* instance;
};

/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
