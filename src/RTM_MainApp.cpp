/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RTM_MainApp.cpp
 * @brief       Implementation of the main class of the library. It also includes
 *				main() function which is the starting point of the application.
 *
 * @author      Mehmet Fatih �zay
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
#include "json.hpp"
#include "RtosComm.h"
#include "EcSlaveInfo.h"

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


RTM_MainApp* RTM_MainApp::instance = 0;
using namespace nlohmann;
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
EC_T_VOID RTM_MainApp::createTestThreads(EC_T_VOID* pvAppContext)
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

	RTM_TestRun testTmp_3;
	RTM_CompactController ccTmp_6;
	ccTmp_6.CC_ID = 5;
	testTmp_3.ccVector.push_back(ccTmp_6);
	testTmp_3.testId = 2;

	RTM_TestRun testTmp_4;
	RTM_CompactController ccTmp_7;
	ccTmp_7.CC_ID = 6;
	testTmp_4.ccVector.push_back(ccTmp_7);
	testTmp_4.testId = 3;

	RTM_TestRun testTmp_5;
	RTM_CompactController ccTmp_8;
	ccTmp_8.CC_ID = 7;
	testTmp_5.ccVector.push_back(ccTmp_8);
	testTmp_5.testId = 4;


	mainApp->testVector.push_back(testTmp);
	mainApp->testVector.push_back(testTmp_2);
	mainApp->testVector.push_back(testTmp_3);
	mainApp->testVector.push_back(testTmp_4);
	mainApp->testVector.push_back(testTmp_5);

	//Suppose There is a single Test
	printf("Test Vector Size : %d\n", mainApp->testVector.size());
	printf("Test Vector 0 CC Size : %d\n", mainApp->testVector[0].ccVector.size());
	printf("Test Vector 1 CC Size : %d\n", mainApp->testVector[1].ccVector.size());
	printf("Test Vector 2 CC Size : %d\n", mainApp->testVector[2].ccVector.size());
	printf("Test Vector 3 CC Size : %d\n", mainApp->testVector[3].ccVector.size());
	printf("Test Vector 4 CC Size : %d\n", mainApp->testVector[4].ccVector.size());


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
}

/******************************************************************************/

EC_T_VOID mainRun(EC_T_VOID* pvAppContext)
{
	T_EC_DEMO_APP_CONTEXT *demoAppTmp = static_cast<T_EC_DEMO_APP_CONTEXT *>(pvAppContext);
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();
	RtosComm *rtmComm = RtosComm::getInstance();

	rtmComm->libInit();

	rtmComm->createLinkedList();

	//RtosTimeSyncStart();
	//rtmComm->create_Data_ReceiveMessageQueue();
	//rtmComm->create_Data_SendMessageQueue();
	/*rtmComm->createSocket();
	rtmComm->bindSocket();
	rtmComm->listenSocket();*/

	while(1)
	{
		/*if (!rtmComm->connectedRTOS)
		{
			rtmComm->acceptClient();
		}
		else
		{
			rtmComm->receiveMessage();
		}*/
		OsSleep(1000);
	}
}


/******************************************************************************/
uint8_t tmpRcvd = 0;
EC_T_VOID RTM_MainApp::copyRcvdEthercatMsgToBuffer(EC_T_BYTE* ecatMsg)
{
	RtosComm *rtmComm = RtosComm::getInstance();
	//memcpy(&receivedEtherCatArray[0], (UINT8*)&ecatMsg[0], rtmComm->dwTotalPdSizeIn);
	//printf("[0] : %d\n", receivedEtherCatArray[0]);
	int val = (int)(*(int *)ecatMsg);


	if(tmpRcvd == 255)
	{
		if (val != 0)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "X : %d - %d - %d\n", tmpRcvd,receivedEtherCatArray[0]));
		}
	}
	else if(tmpRcvd != (val-1))
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "E : %d - %d - %d\n",tmpRcvd,receivedEtherCatArray[0]));
	}
	tmpRcvd = val;
}

/******************************************************************************/

#define  OWNER_FREE         0
#define  OWNER_RTOS         1
#define  OWNER_WINDOWS      2
#define  OWNER_INVALID      9

uint8_t started  = 0;
uint8_t tmpTake = 0;
EC_T_VOID RTM_MainApp::takeDataFromMsgQueue(EC_T_VOID)
{
	RtosComm *rtmComm = RtosComm::getInstance();
	UINT32                  dwNumData;
	UINT32 dwRetVal = RTE_SUCCESS;
	UINT32  dwOwner;
	UINT32  dwInitVal;
	//memset((UINT8*)&rcvSignalFromNRTM[0], 0, rtmComm->dwTotalPdSizeOut);

	struct timespec t;
	OsMemset(&t, 0, sizeof(struct timespec));
	/* get current time */
	clock_gettime(CLOCK_MONOTONIC, &t);


	/* Try to get ownership */
	while(1)
	{
		UINT32 dwRetVal = RtosInterlockedCompareExchange( &rtmComm->ll.defaultList.dwOwner, OWNER_RTOS, OWNER_FREE, &dwInitVal );
		if( RTE_SUCCESS != dwRetVal )
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR,"Interlocked function failed\n"));
		}
		if( OWNER_FREE == dwInitVal )
		{
			/* We got the ownership */
			break;
		}
	}

	/*if (rtmComm->ll.defaultList.pFreeListStruct->count > 10)
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR,"L: %d F: %d\n", rtmComm->ll.defaultList.pList->count, rtmComm->ll.defaultList.pFreeListStruct->count));*/
	if (rtmComm->ll.defaultList.pList->count > 0 && rtmComm->ll.defaultList.pFreeListStruct->count >= DATA_RCV_MSQ_SIZE)
	{
		ecatMsg * temp= NULL ;
		temp=(ecatMsg *)rtmComm->ll.getNodeFromShm(&rtmComm->ll.defaultList,FROM_LIST | FROM_FRONT);
		UINT8 val = 0;
		if(NULL != temp)
		{
			val = temp->data[0];
			rtmComm->ll.putNodeToShm(&rtmComm->ll.defaultList, (RtosShmLinkedList::nodeShm *)temp, FROM_FREE_LIST | FROM_BACK );
		}
		else
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR,"Temp is NULL!\n"));
		}
		started = 1;

		if(tmpTake == 255)
		{
			if (val != 0)
			{
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "DX : %d - %d\n", tmpTake,val));
			}
		}
		else if(tmpTake != (val-1))
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "DE : %d - %d\n",tmpTake,val));
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR,"L: %d F: %d\n", rtmComm->ll.defaultList.pList->count, rtmComm->ll.defaultList.pFreeListStruct->count));
		}
		tmpTake = val;
	}
	else
	{
		if (started)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "STOP\n"));
			started = 0;
		}
	}

	VMF_MEMBARRIER( &rtmComm->ll.defaultList.dwOwner );
	rtmComm->ll.defaultList.dwOwner = OWNER_FREE;

	struct timespec t2;
	OsMemset(&t2, 0, sizeof(struct timespec));
	/* get current time */
	clock_gettime(CLOCK_MONOTONIC, &t2);

	long entryNsec = ((t2.tv_sec-t.tv_sec) * 1000000000) + (t2.tv_nsec-t.tv_nsec);

	if (entryNsec > 10000)
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Q: %ld\n", entryNsec));

}

/******************************************************************************/
EC_T_VOID RTM_MainApp::triggerTests(EC_T_VOID)
{
	struct timespec t;
    OsMemset(&t, 0, sizeof(struct timespec));
	/* get current time */
	clock_gettime(CLOCK_MONOTONIC, &t);


	for (EC_T_UINT idx=0; idx < testVector.size(); idx++)
	{
		OsSetEvent(testVector[idx].testEvent);
	}

	for (EC_T_UINT idx=0; idx < testVector.size(); idx++)
	{
		OsWaitForEvent(testVector[idx].recvEvent, 10);

	}

	struct timespec t2;
	OsMemset(&t2, 0, sizeof(struct timespec));
	/* get current time */
	clock_gettime(CLOCK_MONOTONIC, &t2);

	long entryNsec = ((t2.tv_sec-t.tv_sec) * 1000000000) + (t2.tv_nsec-t.tv_nsec);
	if (entryNsec > 100000)
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "T: %ld\n", entryNsec));

}

/******************************************************************************/
EC_T_VOID RTM_MainApp::copySendBufferToEthercat(EC_T_BYTE* ecatMsg)
{
	RtosComm *rtmComm = RtosComm::getInstance();
	memcpy((UINT8*)&ecatMsg[0], &sendEtherCatArray[0], rtmComm->dwTotalPdSizeOut);
}

/******************************************************************************/
EC_T_VOID shutdownApp(EC_T_VOID)
{
	RtosComm *rtmComm = RtosComm::getInstance();

	UINT32 dwRes = 0;
	if (rtmComm->hSocketClient != NULL)
	{
		dwRes = RtosSocketClose( rtmComm->hSocketClient );
		rtmComm->hSocketClient = NULL;
	}

	if (rtmComm->hSocket != NULL)
	{
		dwRes = RtosSocketClose( rtmComm->hSocket );
		rtmComm->hSocket = NULL;
	}

}

void takeDataFromMsq(EC_T_VOID* pvAppContext)
{
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();

	mainApp->readMsqEvent = OsCreateEvent();
	mainApp->okMsqEvent = OsCreateEvent();

	mainApp->takeDataFromMsgQueue();
	OsSetEvent(mainApp->okMsqEvent);

	while(1)
	{
		OsWaitForEvent(mainApp->readMsqEvent, EC_WAITINFINITE);
		mainApp->takeDataFromMsgQueue();
		OsSetEvent(mainApp->okMsqEvent);
	}

}

extern int ECMaster_main(int nArgc, char* ppArgv[]);
extern volatile EC_T_BOOL  bRun;
/******************************************************************************/
int main(int nArgc, char* ppArgv[])
{
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();
	EC_T_VOID* pvJobTaskHandle   = EC_NULL;
	EC_T_VOID* pvJobTaskHandleMsq   = EC_NULL;

	pvJobTaskHandle = OsCreateThread((EC_T_CHAR*)"RTM_MainApp",
					mainRun,
					0,
					MAIN_THREAD_PRIO,
					JOBS_THREAD_STACKSIZE,
					NULL);

#if 0
	pvJobTaskHandleMsq = OsCreateThread((EC_T_CHAR*)"RTM_TakeData",
						takeDataFromMsq,
						0,
						RTM_THREAD_PRIO,
						JOBS_THREAD_STACKSIZE,
						NULL);
#endif

	ECMaster_main(nArgc, ppArgv);
	while(bRun)
	{
		printf("Main loop\n");
		OsSleep(1000);
	}

	shutdownApp();
}

/**@}*/
/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
