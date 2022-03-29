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
#include "json.hpp"

/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

#define MSG_QUEUE_DATA_NRTM_TO_RTM		  	TEXT("MsgQueue_Data_FromNRTMtoRTM")
#define MSG_QUEUE_DATA_RTM_TO_NRTM         	TEXT("MsgQueue_Data_FromRTMtoNRTM")

#define MSG_NUM_WORDS       			200

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


	printf("Message data size : %d header size : %d footere size : %d", NRTM_MSG_SIZE, sizeof(msg_header), sizeof(msg_footer));

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

	mainApp->createMsgQueueWithNRTM();
	mainApp->createSocketWithNRTM();

	while(1)
	{
		if (!mainApp->nrtmConnected)
		{
			mainApp->acceptNRTM();
		}
		else
		{
			mainApp->takeMsgFromNRTM();
		}
		OsSleep(1000);
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

	MsgQueueOptions_Data_fromNRTM.dwSize                  = sizeof(MsgQueueOptions_Data_fromNRTM);
	MsgQueueOptions_Data_fromNRTM.dwFlags                 = RTOSMSGQUEUE_OPTIONS_FLAG_ALLOWBROKEN;
	MsgQueueOptions_Data_fromNRTM.dwNumMessages           = 10000;
	MsgQueueOptions_Data_fromNRTM.dwMsgDataSizeInBytes    = MSG_NUM_WORDS;
	MsgQueueOptions_Data_fromNRTM.bReadAccess             = TRUE;
	dwRetVal = RtosMsgQueueCreate(MSG_QUEUE_DATA_NRTM_TO_RTM, &MsgQueueOptions_Data_fromNRTM, &hQueue_Data_fromNRTM);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR Create MSQ %s - %d\n", MSG_QUEUE_DATA_NRTM_TO_RTM, dwRetVal));
	}

	MsgQueueOptions_Data_toNRTM.dwSize                  = sizeof(MsgQueueOptions_Data_toNRTM);
	MsgQueueOptions_Data_toNRTM.dwFlags                 = RTOSMSGQUEUE_OPTIONS_FLAG_ALLOWBROKEN;
	MsgQueueOptions_Data_toNRTM.dwNumMessages           = 10;
	MsgQueueOptions_Data_toNRTM.dwMsgDataSizeInBytes    = MSG_NUM_WORDS;
	MsgQueueOptions_Data_toNRTM.bReadAccess             = FALSE;
	dwRetVal = RtosMsgQueueCreate(MSG_QUEUE_DATA_RTM_TO_NRTM, &MsgQueueOptions_Data_toNRTM, &hQueue_Data_toNRTM);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR Create MSQ %s - %d\n", MSG_QUEUE_DATA_RTM_TO_NRTM, dwRetVal));
	}

	printf( "MSQs are Created\n");
}

#if 0
/******************************************************************************/
EC_T_VOID RTM_MainApp::createSocketWithNRTM(EC_T_VOID)
{
	UINT32          dwRetVal = 0;

	dwRetVal = RtosSocketCreate( RTOSSOCKET_FAMILY_RTE, RTOSSOCKET_TYPE_STREAM, RTOSSOCKET_PROTOCOL_TCP, &hSocket );
	if( RTE_SUCCESS != dwRetVal )
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, " %-28s : Error RtosSocketCreate (0x%X)\n", "Server Tcp", dwRetVal));
	}
}
#endif

/******************************************************************************/
EC_T_VOID RTM_MainApp::createSocketWithNRTM(EC_T_VOID)
{
	UINT32          dwRetVal = 0;

	dwRetVal = RtosSocketCreate( RTOSSOCKET_FAMILY_RTE, RTOSSOCKET_TYPE_STREAM, RTOSSOCKET_PROTOCOL_TCP, &hSocket );
	if( RTE_SUCCESS != dwRetVal )
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, " %-28s : Error RtosSocketCreate (0x%X)\n", "Server Tcp", dwRetVal));
	}

	dwRetVal = RtosSocketBind(hSocket, SERVER_PORT);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, " %-28s : Error RtosSocketBind (0x%X)\n", "Server Tcp", dwRetVal));

	}

	dwRetVal = RtosSocketListen(hSocket);
	if (RTE_SUCCESS != dwRetVal)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, " %-28s : Error RtosSocketListen (0x%X)\n", "Server Tcp", dwRetVal));
	}
}

#if 0
/******************************************************************************/
EC_T_VOID RTM_MainApp::connectNRTM(EC_T_VOID)
{
	UINT32          dwRetVal = 0;
	RTOSSOCKET_ADDR     Addr;

	Addr.bySize         = sizeof(RTOSSOCKET_ADDR);
	Addr.byFamily       = RTOSSOCKET_FAMILY_RTE;
	Addr.wPort          = SERVER_PORT;
	Addr.u.Rte.dwOsId   = SERVER_OSID;
	dwRetVal = RtosSocketConnect( hSocket, &Addr );
	if( RTE_SUCCESS != dwRetVal )
	{
		//EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, " %-28s : Error RtosSocketConnect (0x%X)\n", "Server Tcp", dwRetVal));
		printf("not connected\n");
		nrtmConnected = 0;
	}
	else
	{
		printf("connected\n");
		nrtmConnected = 1;
	}

}
#endif
/******************************************************************************/
EC_T_VOID RTM_MainApp::acceptNRTM(EC_T_VOID)
{
	UINT32          dwRetVal;
	RTOSSOCKET_ADDR AddrNew;
	AddrNew.bySize = sizeof(RTOSSOCKET_ADDR);
	dwRetVal = RtosSocketAccept(hSocket, &hSocketClient, &AddrNew, 1000);
	switch (RTE_ERROR_GET_ERROR(dwRetVal))
	{
	case RTE_SUCCESS:
		printf("Client accepted\n");
		nrtmConnected = 1;
		break;
	case RTE_ERROR_TIMEOUT:
		nrtmConnected = 0;
		break;
	default:
		nrtmConnected = 0;
		break;
	}

}

/******************************************************************************/
uint8_t tmpRcvd = 0;
int counterX = 0;
EC_T_VOID RTM_MainApp::copyRcvdEthercatMsgToBuffer(EC_T_BYTE* ecatMsg)
{
	memcpy(&receivedEtherCatArray[0], (UINT8*)&ecatMsg[0], MAX_ETHERCAT_MSG_SIZE);
	//printf("[0] : %d\n", receivedEtherCatArray[0]);

	if(tmpRcvd == 255)
	{
		if (receivedEtherCatArray[0] != 0)
		{
			printf("X : %d - %d\n", tmpRcvd,receivedEtherCatArray[0] );
		}
	}
	else if(tmpRcvd != (receivedEtherCatArray[0]-1))
	{
		printf("E : %d - %d\n",tmpRcvd,receivedEtherCatArray[0]);
	}
	tmpRcvd = receivedEtherCatArray[0];

	/*if (counterX % 64 == 0)
	{
		printf("[0] : %d\n", receivedEtherCatArray[0]);
	}*/

	counterX++;


}

/******************************************************************************/
EC_T_VOID RTM_MainApp::takeDataFromMsgQueue(EC_T_VOID)
{
	UINT32                  dwNumData;
	memset((UINT8*)&rcvSignalFromNRTM[0], 0, MSG_NUM_WORDS);

	Info_Data_fromNRTM.dwSize = sizeof(Info_Data_fromNRTM);
	RtosMsgQueueInfoGet(hQueue_Data_fromNRTM, &Info_Data_fromNRTM);

	//printf("Pending : %d\n", Info_fromNRTM.dwNumPending);
	if (Info_Data_fromNRTM.dwNumPending > 0)
	{
		UINT32 dwRetVal = RtosMsgQueueRead(hQueue_Data_fromNRTM, (UINT8*)&rcvSignalFromNRTM[0], MSG_NUM_WORDS, &dwNumData, 1);
		if (dwRetVal != RTE_SUCCESS)
		{
			printf("RQ : %d\n", dwRetVal);

		}

		//printf("Take Data From Msq Queue : %d - %d - %d \n", rcvSignalFromNRTM[0], rcvSignalFromNRTM[1], rcvSignalFromNRTM[2]);
	}
}

#define NONE	0
/******************************************************************************/
int RTM_MainApp::takeMsgFromNRTM(EC_T_VOID)
{
	UINT32  dwRetVal;
	UINT32  dwRes;
	UINT32  dwBytesToRead   = NRTM_MESSAGE_TOTAL_SIZE;
	UINT32  dwBytesRead     = 0;
	UINT32  dwBytesReadNew;

	dwRes = RtosSocketRecv( hSocketClient, &rtosRcvArray[dwBytesRead], (dwBytesToRead-dwBytesRead), &dwBytesReadNew );

	if (!dwBytesReadNew)
	{
		return 0;
	}

	memcpy(&((UINT8*)&nrtmMsg)[0], &rtosRcvArray[0], sizeof(nrtmMsg.header));
	printf("Rcvd datalen : %d - %d\n" ,nrtmMsg.header.dataLen, dwBytesReadNew);

	if (nrtmMsg.header.isFragment || (nrtmMsg.header.dataLen + sizeof(msg_header)) != dwBytesReadNew)
	{
		printf("Msg is fragmented\n");
	}

	nrtmMsg.msg = (uint8_t *)malloc(nrtmMsg.header.dataLen * sizeof(uint8_t));

	memcpy(&nrtmMsg.msg[0], &rtosRcvArray[sizeof(msg_header)], nrtmMsg.header.dataLen);

	char str[13 + 1];
	memcpy(str, &nrtmMsg.msg[0], nrtmMsg.header.dataLen);
	str[nrtmMsg.header.dataLen] = 0;
	printf("%s\n", str);


	nrtm_msg helloAck;
	helloAck.header.ccID = NONE;
	helloAck.header.cmd = 2;
	helloAck.header.fragmentNo = NONE;
	helloAck.header.isACK = 0;
	helloAck.header.isFragment = NONE;
	helloAck.header.msgID = 99;
	helloAck.header.nrtmID = 1;
	helloAck.header.rtmID = 1;
	helloAck.header.startByte = 0x03;
	helloAck.header.testID = NONE;
	helloAck.header.timestamp = 1;

	json helloJson;
	helloJson["msg"] = "HelloACKFromRTM";
	std::string helloMsg = helloJson.dump();

	helloAck.msg = (uint8_t *)malloc(helloMsg.length() * sizeof(uint8_t));

	memcpy(&helloAck.msg[0], helloMsg.data(), helloMsg.length());
	helloAck.header.dataLen = helloMsg.length();

	UINT32 dwWritten = 0;
	int datalen = helloAck.header.dataLen + sizeof(msg_header);
	memcpy(rtosSendArray, (uint8_t *)&helloAck, sizeof(msg_header));
	memcpy(&rtosSendArray[sizeof(msg_header)], (uint8_t *)&helloAck.msg[0], helloAck.header.dataLen);


	dwRetVal = RtosSocketSend(hSocketClient, (UINT8*)&rtosSendArray[0], datalen, &dwWritten);



#if 0
	/* Read data */
	do
	{
		dwBytesReadNew = 0;
		dwRes = RtosSocketRecv( hSocket, &((UINT8*)&nrtmMsg)[dwBytesRead], (dwBytesToRead-dwBytesRead), &dwBytesReadNew );

		switch( RTE_ERROR_GET_ERROR( dwRes ) )
		{
		case RTE_SUCCESS:
			break;
		case RTE_ERROR_TIMEOUT:
			printf(("  Rx: Timeout\n") );
			break;
		default:
			printf((" Rx: Error (0x%X)\n"), dwRes );
			break;
		}
		if( RTE_SUCCESS != dwRes )
		{
			printf("Error");
			return RTE_ERROR;
		}
		dwBytesRead += dwBytesReadNew;
	} while( dwBytesRead < dwBytesToRead );
#endif


	return RTE_SUCCESS;

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

	if ((t2.tv_sec-t.tv_sec) > 0 || (t2.tv_nsec-t.tv_nsec) > 100000)
		printf("%ld\n", (t2.tv_nsec-t.tv_nsec));

}

/******************************************************************************/
EC_T_VOID RTM_MainApp::copySendBufferToEthercat(EC_T_BYTE* ecatMsg)
{
	memcpy((UINT8*)&ecatMsg[0], &sendEtherCatArray[0], MAX_ETHERCAT_MSG_SIZE);
}

extern int ECMaster_main(int nArgc, char* ppArgv[]);
extern volatile EC_T_BOOL  bRun;
/******************************************************************************/
int main(int nArgc, char* ppArgv[])
{
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();
	UINT32 dwRetVal;

	printf("My app Init\n");
	EC_T_VOID*             pvJobTaskHandle   = EC_NULL;

	pvJobTaskHandle = OsCreateThread((EC_T_CHAR*)"RTM_MainApp",
					mainRun,
					0,
					RTM_THREAD_PRIO,
					JOBS_THREAD_STACKSIZE,
					NULL);


	ECMaster_main(nArgc, ppArgv);
	while(bRun)
	{
		printf("Main loop\n");
		OsSleep(1000);
	}

	UINT32          dwRes;
	if (mainApp->hSocketClient != NULL)
	{
		dwRes = RtosSocketClose( mainApp->hSocketClient );
		mainApp->hSocketClient = NULL;
	}

	if (mainApp->hSocket != NULL)
	{
		dwRes = RtosSocketClose( mainApp->hSocket );
		mainApp->hSocket = NULL;
	}
}

/**@}*/
/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
