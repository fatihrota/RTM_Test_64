/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RtosMessage.cpp
 * @brief       Implementation of the RotsMessage class of the library.
 *
 * @author      Mehmet Fatih Özay
 * @date        2022-03-30
 *
 * @ingroup     RtosMessage
 * @{
 *****************************************************************************/

 /*============================================================================*/
 /* Includes                                                                   */
 /*============================================================================*/
#include "RtosMessage.h"
#include "json.hpp"
#include <chrono>
#include "RtosComm.h"
#include "RTM_MainApp.h"
#include "RTM_TestRun.h"
#include "RTM_CompactController.h"
#include <cstdio>
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
extern EC_T_VOID testRun(EC_T_VOID* testIdx);
/*============================================================================*/
/* Module global data                                                         */
/*============================================================================*/
using namespace nlohmann;
using namespace std::chrono;
/*============================================================================*/
/* Implementation of functions                                                */
/*============================================================================*/


INT32 RtosMessage::createHeader(msg_header *hdr, UINT8 nrtmID, UINT8 rtmID, UINT8 testID, UINT8 ccID,
							   UINT8 isACK, UINT8 isFragment, UINT8 fragmentNo, cmdType cmd)
{
	hdr->startByte = 0x03;
	hdr->nrtmID = nrtmID;
	hdr->rtmID = rtmID;
	hdr->testID = testID;
	hdr->ccID = ccID;
	hdr->isACK = isACK;
	hdr->isFragment = isFragment;
	hdr->fragmentNo = fragmentNo;
	hdr->cmd = cmd;
	hdr->timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	return 0;
}

/******************************************************************************/
INT32 RtosMessage::createHelloAckMsg(UINT8 *dataArray, INT32 *datalen)
{
	rtosMsg msgHelloAck = {0};

	createHeader(&msgHelloAck.header, 1, 1, NONE, NONE, 1, NONE, NONE, HELLOACK);

	json helloJson;
	helloJson["msg"] = "HelloACKFromRTM";
	std::string helloMsg = helloJson.dump();
	msgHelloAck.msg = (uint8_t *)malloc(helloMsg.length() * sizeof(uint8_t));

	memcpy(&msgHelloAck.msg[0], helloMsg.data(), helloMsg.length());
	msgHelloAck.header.dataLen = helloMsg.length();

	*datalen = msgHelloAck.header.dataLen + sizeof(msg_header);
	memcpy(dataArray, (uint8_t *)&msgHelloAck, sizeof(msg_header));
	memcpy(&dataArray[sizeof(msg_header)], (uint8_t *)&msgHelloAck.msg[0], msgHelloAck.header.dataLen);

	return 0;
}

INT32 RtosMessage::parseHello(void *msgData, INT32 datalen)
{
	UINT8 *data = ((UINT8 *)msgData + sizeof(msg_header));

	std::string s((char *)data, datalen);

	json hello = json::parse(s.c_str());
	std::string msgTest = hello["msg"];
	printf("hello : %s\n", msgTest.c_str());

	RtosComm *rtmComm = RtosComm::getInstance();

	int dataSendLen = 0;
	memset(&rtmComm->rtosSendArray, 0, NRTM_MESSAGE_TOTAL_SIZE);
	createHelloAckMsg(rtmComm->rtosSendArray, &dataSendLen);
	rtmComm->sendMessage(dataSendLen);
	return 0;
}

uint8_t ethercatPause = 0;
INT32 RtosMessage::parseTestInfo(void *msgData, INT32 datalen)
{
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();
	UINT8 *data = ((UINT8 *)msgData + sizeof(msg_header));

	std::string s((char *)data, datalen);

	//std::cout << s;

	json testJson = json::parse(s.c_str());
	int numberOfTest = testJson["testCnt"];

	ethercatPause = 1;

	for (int idx = 0; idx < numberOfTest; idx++)	/* Loop in Test information */
	{
		RTM_TestRun testTmp;
		int numberOfDev = testJson["test"][idx]["deviceCnt"];

		testTmp.testId = testJson["test"][idx]["testId"];
		for (int devIdx = 0; devIdx < numberOfDev; devIdx++)	/* Loop in devices in test*/
		{
			RTM_CompactController ccTmp;
			ccTmp.CC_ID = testJson["test"][idx]["cc"][devIdx]["ccID"];

			testTmp.ccVector.push_back(ccTmp);

		}
		mainApp->testVector.push_back(testTmp);
	}

	printf("Test Info : test cnt : %d \n", mainApp->testVector.size());

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
				1,
				RTM_THREAD_PRIO,
				JOBS_THREAD_STACKSIZE,
				(EC_T_VOID*)idx);
	}

	ethercatPause = 0;
	return 0;
}

INT32 RtosMessage::parseReceivedMsg(void *msgData)
{
	msg_header *testHdr = static_cast<msg_header *>(msgData);

	printf("Test Hdr len : %d - cmd : %d\n", testHdr->dataLen, testHdr->cmd);

	switch (testHdr->cmd)
	{
	case HELLO:
		parseHello(msgData, testHdr->dataLen);
		break;
	case HELLOACK:
		break;
	case TESTINFO:
		parseTestInfo(msgData, testHdr->dataLen);
		break;
	default:
		break;
	}

	return 0;
}

INT32 RtosMessage::createTestInfoMsg(void)
{

	return 0;
}
/**@}*/
/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
