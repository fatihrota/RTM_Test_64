/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RTM_MainApp.cpp
 * @brief       Implementation of the main class of the library. It also includes
 *				main() function which is the starting point of the application.
 *
 * @author      Mehmet Fatih Özay
 * @date        2022-02-25
 *
 * @ingroup     RTM_MainApp
 * @{
 *****************************************************************************/

 /*============================================================================*/
 /* Includes                                                                   */
 /*============================================================================*/

/*-LOGGING-------------------------------------------------------------------*/
#ifndef pEcLogParms
#define pEcLogParms G_pEcLogParms
#endif

#include <cstdint>
#include <cstring>
#include "RTM_MainApp.h"
/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

#define MSG_QUEUE_NRTM_TO_RTM		  	TEXT("MsgQueueFromNRTMtoRTM")
#define MSG_QUEUE_RTM_TO_NRTM         	TEXT("MsgQueueFromRTMtoNRTM")
#define MSG_NUM_WORDS       			200

/*============================================================================*/
/* Type definitions                                                           */
/*============================================================================*/

/*============================================================================*/
/* Global data                                                                */
/*============================================================================*/


RTM_MainApp* RTM_MainApp::instance = 0;

/*============================================================================*/
/* Module global data                                                         */
/*============================================================================*/

/*============================================================================*/
/* Implementation of functions                                                */
/*============================================================================*/

extern EC_T_VOID testRun(EC_T_VOID* testIdx);

RTM_MainApp::RTM_MainApp() {

}

RTM_MainApp::~RTM_MainApp() {

}

/******************************************************************************/
RTM_MainApp* RTM_MainApp::getInstance() {
	if (instance == 0) {
		instance = new RTM_MainApp();
	}

	return instance;
}

/******************************************************************************/
EC_T_VOID RTM_MainApp::cleanInstance() {
	if (instance)
		delete instance;
	instance = NULL;
}

/******************************************************************************/
EC_T_VOID mainRun(EC_T_VOID* pvAppContext)
{
	T_EC_DEMO_APP_CONTEXT *demoAppTmp = static_cast<T_EC_DEMO_APP_CONTEXT *>(pvAppContext);
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();

	RTM_TestRun testTmp;
	RTM_CompactController ccTmp;
	ccTmp.CC_ID = 0;

	RTM_CompactController ccTmp_2;
	ccTmp_2.CC_ID = 1;

	RTM_TestRun testTmp_2;
	RTM_CompactController ccTmp_3;
	ccTmp_3.CC_ID = 2;

	RTM_CompactController ccTmp_4;
	ccTmp_4.CC_ID = 3;

	RTM_CompactController ccTmp_5;
	ccTmp_5.CC_ID = 4;

	testTmp.ccVector.push_back(ccTmp);
	testTmp.ccVector.push_back(ccTmp_2);
	testTmp.testId = 0;

	testTmp_2.ccVector.push_back(ccTmp_3);
	testTmp_2.ccVector.push_back(ccTmp_4);
	testTmp_2.ccVector.push_back(ccTmp_5);
	testTmp_2.testId = 1;


	mainApp->testVector.push_back(testTmp);
	mainApp->testVector.push_back(testTmp_2);

	//Suppose There is a single Test
	printf("Test Vector Size : %d\n", mainApp->testVector.size());
	printf("Test Vector 0 CC Size : %d\n", mainApp->testVector[0].ccVector.size());
	printf("Test Vector 1 CC Size : %d\n", mainApp->testVector[1].ccVector.size());

	for (EC_T_UINT idx = 0; idx < mainApp->testVector.size(); idx++)
	{
		mainApp->testVector[idx].createProcess();
	}

	for (EC_T_UINT idx = 0; idx < mainApp->testVector.size(); idx++)
	{
		EC_T_VOID*             pvJobTaskHandle   = EC_NULL;
		char str[20];
		sprintf(str, "RTM_Test%d", idx);

		pvJobTaskHandle = OsCreateThread((EC_T_CHAR*)str,
				testRun,
				demoAppTmp->AppParms.CpuSet + 1,
				RTM_THREAD_PRIO,
				JOBS_THREAD_STACKSIZE,
				(EC_T_VOID*)idx);
	}

	while(1)
	{
		OsSleep(100);
	}
}


/******************************************************************************/
EC_T_VOID RTM_MainApp::createMsgQueueWithNRTM(EC_T_VOID)
{
	UINT32 dwRetVal;

	dwRetVal = RtosLibInit();
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR initializing RTOS Library\n"));
	}

	MsgQueueOptions_fromNRTM.dwSize                  = sizeof(MsgQueueOptions_fromNRTM);
	MsgQueueOptions_fromNRTM.dwFlags                 = RTOSMSGQUEUE_OPTIONS_FLAG_ALLOWBROKEN;
	MsgQueueOptions_fromNRTM.dwNumMessages           = 10;
	MsgQueueOptions_fromNRTM.dwMsgDataSizeInBytes    = MSG_NUM_WORDS;
	MsgQueueOptions_fromNRTM.bReadAccess             = TRUE;
	dwRetVal = RtosMsgQueueCreate(MSG_QUEUE_NRTM_TO_RTM, &MsgQueueOptions_fromNRTM, &hQueue_fromNRTM);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR Create MSQ %s\n", MSG_QUEUE_NRTM_TO_RTM));
	}

	MsgQueueOptions_fromRTM.dwSize                  = sizeof(MsgQueueOptions_fromRTM);
	MsgQueueOptions_fromRTM.dwFlags                 = RTOSMSGQUEUE_OPTIONS_FLAG_ALLOWBROKEN;
	MsgQueueOptions_fromRTM.dwNumMessages           = 10;
	MsgQueueOptions_fromRTM.dwMsgDataSizeInBytes    = MSG_NUM_WORDS;
	MsgQueueOptions_fromRTM.bReadAccess             = FALSE;
	dwRetVal = RtosMsgQueueCreate(MSG_QUEUE_RTM_TO_NRTM, &MsgQueueOptions_fromRTM, &hQueue_fromRTM);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR Create MSQ %s\n", MSG_QUEUE_RTM_TO_NRTM));
	}

	EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_ERROR, "MSQ are Created\n"));
}

/******************************************************************************/
EC_T_VOID RTM_MainApp::copyRcvdEthercatMsgToBuffer(EC_T_BYTE* ecatMsg)
{
	memcpy(&receivedEtherCatArray[0], (UINT8*)&ecatMsg[0], MAX_ETHERCAT_MSG_SIZE);
}

/******************************************************************************/
EC_T_VOID RTM_MainApp::takeDataFromMsgQueue(EC_T_VOID)
{
	UINT32                  dwNumData;
	memset((UINT8*)&rcvSignalFromNRTM[0], 0, MSG_NUM_WORDS);

	Info_fromNRTM.dwSize = sizeof(Info_fromNRTM);
	RtosMsgQueueInfoGet(hQueue_fromNRTM, &Info_fromNRTM);

	//printf("Pending : %d\n", Info_fromNRTM.dwNumPending);
	if (Info_fromNRTM.dwNumPending > 0)
	{
		UINT32 dwRetVal = RtosMsgQueueRead(hQueue_fromNRTM, (UINT8*)&rcvSignalFromNRTM[0], MSG_NUM_WORDS, &dwNumData, 1);
		if (dwRetVal != RTE_SUCCESS)
		{
			printf("RQ : %d\n", dwRetVal);

		}

		//printf("Take Data From Msq Queue : %d - %d - %d \n", rcvSignalFromNRTM[0], rcvSignalFromNRTM[1], rcvSignalFromNRTM[2]);
	}
}

/******************************************************************************/
EC_T_VOID RTM_MainApp::triggerTests(EC_T_VOID)
{
	for (EC_T_UINT idx=0; idx < testVector.size(); idx++)
	{
		OsSetEvent(testVector[idx].testEvent);
	}
}

/******************************************************************************/
EC_T_VOID RTM_MainApp::copySendBufferToEthercat(EC_T_BYTE* ecatMsg)
{
	memcpy((UINT8*)&ecatMsg[0], &sendEtherCatArray[0], MAX_ETHERCAT_MSG_SIZE);
}


/**@}*/
/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
