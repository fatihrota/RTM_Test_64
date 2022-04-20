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
#include "RtosComm.h"
/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

#define ECAT_MSG_SIZE_PER_CC			21

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

	EC_T_VOID createTestThreads(EC_T_VOID* pvAppContext);

	EC_T_VOID copyRcvdEthercatMsgToBuffer(EC_T_BYTE* ecatMsg);
	EC_T_VOID takeDataFromMsgQueue(EC_T_VOID);
	EC_T_VOID triggerTests(EC_T_VOID);
	EC_T_VOID copySendBufferToEthercat(EC_T_BYTE* ecatMsg);
	EC_T_BYTE sendEtherCatArray[MAX_ETHERCAT_MSG_SIZE];
	EC_T_BYTE receivedEtherCatArray[MAX_ETHERCAT_MSG_SIZE];

	EC_T_BYTE rcvSignalFromNRTM[MAX_ETHERCAT_MSG_SIZE];
	EC_T_BYTE sndSignalToNRTM[MAX_ETHERCAT_MSG_SIZE];

	std::vector<RTM_TestRun> testVector;	/* Holds seperate tests */
private:
	RTM_MainApp();
	static RTM_MainApp* instance;
};

/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
