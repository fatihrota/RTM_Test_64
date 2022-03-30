/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RTM_TestRun.cpp
 * @brief       Implementation of the TestRun class of the library.
 *
 * @author      Mehmet Fatih Özay
 * @date        2022-02-25
 *
 * @ingroup     RTM_TestRun
 * @{
 *****************************************************************************/

/*-LOGGING-------------------------------------------------------------------*/
#ifndef pEcLogParms
#define pEcLogParms G_pEcLogParms
#endif
 /*============================================================================*/
 /* Includes                                                                   */
 /*============================================================================*/
#include "RTM_TestRun.h"
#include "RTM_MainApp.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#define FORK 1

#ifdef FORK
extern "C" {
	#include <semaphore.h>
}
#include <fcntl.h>
#endif

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

/*============================================================================*/
/* Module global data                                                         */
/*============================================================================*/

/*============================================================================*/
/* Implementation of functions                                                */
/*============================================================================*/



RTM_TestRun::RTM_TestRun(){
	testEvent = OsCreateEvent();
	recvEvent = OsCreateEvent();
}

RTM_TestRun::~RTM_TestRun(){

}


/******************************************************************************/
EC_T_VOID RTM_TestRun::createProcess(EC_T_VOID)
{
#if FORK
	key = 1000 + testId;
	std::string cmd = "/tmp/RTM_ProcessAPP"; // secondary program you want to run

	if ((shmid = shmget(key,MAX_SHM_SIZE,IPC_CREAT | 0666)) < 0)
	{
		perror("shmget");
		exit(1);
	}

	/* Now we attach the segment to our data space.*/
	if ((shm = shmat(shmid, NULL, 0)) == (void*)-1) {

		perror("shmat");
		exit(1);
	}

	pid = fork(); // create child process

	printf("pid : %d\n",pid);

	switch (pid)
	{
	case -1: // error
	{
		perror("fork");
		exit(1);
	}
	case 0: // child process
	{
		std::string s = std::to_string(testId);
		char const *pchar = s.c_str();  //use char const* as target type
		printf("CMD : %s pchar : %s\n", cmd.c_str(), pchar);
		execl(cmd.c_str(), pchar, (char *)NULL); // run the command
		perror("execl"); // execl doesn't return unless there is a problem
		exit(1);
	}
	default:
		break;
	}

	sprintf(semName_w, "syncSem%dw", testId);
	sprintf(semName_r, "syncSem%dr", testId);

	printf("semName_w : %s semName_r : %s\n", semName_w, semName_r);

	sem_id_w = sem_open(semName_w, O_CREAT, 0600, 0);
	if (sem_id_w == SEM_FAILED){
		perror("Parent   : [sem_open] Failed\n"); return;
	}

	sem_id_r = sem_open(semName_r, O_CREAT, 0600, 0);
	if (sem_id_r == SEM_FAILED){
		perror("Parent   : [sem_open] Failed\n"); return;
	}
#endif

}

uint8_t counter = 0;
/******************************************************************************/
EC_T_VOID RTM_TestRun::trigerrProcess(EC_T_VOID)
{
	uint8_t *s = (uint8_t*)shm;
	*s = counter;
	*(s + 1) = counter + 1;
	//*s++ = (char)(counter+1);
	//*s++ = counter + 1 + '0';
	//printf("T : %d - %x - %x\n", testId, *s, *(s+1) );

	if (sem_post(sem_id_w) < 0)
		printf("Parent   : [sem_post] Failed \n");

	counter++;
}

/******************************************************************************/
EC_T_VOID RTM_TestRun::waitForProcess(EC_T_VOID)
{
	//printf("W : %d\n", testId);
	if (sem_wait(sem_id_r) < 0)
		printf("Parent  : [sem_wait] Failed\n");
}
/******************************************************************************/
EC_T_VOID RTM_TestRun::parseReceivedEthercatBuffer(EC_T_VOID)
{
	//printf("rcv ecat %d\n", testId);
}

/******************************************************************************/
EC_T_VOID RTM_TestRun::parseReceivedCommand(EC_T_VOID)
{
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();
	//printf("rcv cmd %d\n", mainApp->testVector[testId].ccVector.size());
	for(unsigned int idx=0; idx < mainApp->testVector[testId].ccVector.size(); idx++)
	{
		memcpy(&(mainApp->testVector[testId].ccVector[idx].cmdBuffer[0]), &(mainApp->rcvSignalFromNRTM[mainApp->testVector[testId].ccVector[idx].offsetInBuffer]), ECAT_MSG_SIZE_PER_CC);

	}
}

/******************************************************************************/
EC_T_VOID RTM_TestRun::commandPlayer(EC_T_VOID)
{
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();
	//printf("cmd %d\n", testId);
	for(unsigned int idx=0; idx < mainApp->testVector[testId].ccVector.size(); idx++)
	{
		memcpy(&(mainApp->sendEtherCatArray[mainApp->testVector[testId].ccVector[idx].offsetInBuffer]), &(mainApp->testVector[testId].ccVector[idx].cmdBuffer[0]), ECAT_MSG_SIZE_PER_CC);
	}

}

/******************************************************************************/
EC_T_VOID testRun(EC_T_VOID* testIdx)
{
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();

	//printf("Test Vector Size : %d - %d\n", mainApp->testVector.size(), (int)testIdx);
	EC_T_INT tIdx = (int)testIdx;

	while(1)
	{
		OsWaitForEvent(mainApp->testVector[tIdx].testEvent, EC_WAITINFINITE);
		mainApp->testVector[tIdx].parseReceivedEthercatBuffer();
		mainApp->testVector[tIdx].trigerrProcess();
		mainApp->testVector[tIdx].waitForProcess();
		mainApp->testVector[tIdx].parseReceivedCommand();
		mainApp->testVector[tIdx].commandPlayer();
		OsSetEvent(mainApp->testVector[tIdx].recvEvent);
	}

}


/**@}*/
/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
